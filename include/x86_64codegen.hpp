/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

/** @file x86_64codegen.hpp
    @brief X86_64CodeGenerator: lowers an IRModule (ir.hpp) to real,
    assemblable, runnable x86-64 System V assembly text — the one
    concrete backend this pass of the project's larger code-generation
    initiative takes all the way through (see ir.hpp's file comment for
    the full phasing story: IR, one optimization pass, memory layout,
    one real backend, with AArch64/RISC-V backends and a real register
    allocator explicitly deferred, not attempted here).

    Why x86-64 System V specifically, and why Intel syntax: System V's
    integer calling convention (rdi/rsi/rdx/rcx/r8/r9 for the first six
    integer/pointer arguments, rax for an integer/pointer return, rbx/
    r12-r15/rbp callee-saved, 16-byte stack alignment at `call` sites)
    is the best-documented, most-tested-against ABI available in this
    environment (Linux/macOS on x86-64) — see target.hpp's own comment
    for why the *host* architecture, not some other one, is what a
    default build targets, and why this is nonetheless an explicit,
    swappable choice (`generate_module(module, some_other_target_info)`),
    not a hardcoded one. Intel syntax (`.intel_syntax noprefix`, a
    directive GNU `as`/`gcc` both understand directly — this project
    checked, `as --version`/`gcc --version` are both present in this
    environment) was picked over the GNU assembler's AT&T-syntax default
    because `dest, src` operand order and no `%register`/`$immediate`
    sigils read closer to the pseudocode a caller already has in an
    IRInstruction, easier to eyeball-verify against by hand; there is
    nothing about this design that depends on that choice, a future
    contributor preferring AT&T syntax could add it as a second
    `generate_module()`-shaped entry point without touching the lowering
    logic's actual instruction selection at all.

    Register allocation here is deliberately the simplest strategy that
    is still *correct*: every virtual register and every named local
    lives in its own stack slot for the whole function (see
    build_slot_map() below, which extends memorylayout.hpp's
    compute_frame_layout() — that function only knows about named
    locals/parameters, not the virtual registers an IRBuilder-driven
    caller introduces, so this file's own build_slot_map() adds one more
    stack slot per virtual register on top of it), and every instruction
    reloads whichever operands it needs from memory into one of two
    fixed scratch registers (`r10`/`r11`, per target.hpp's
    `TargetInfo::scratch_registers`) immediately before using them,
    writing its result straight back to memory immediately after. This
    is deliberately not a real register allocator (no linear-scan
    liveness analysis, no graph coloring, nothing tracking which values
    are still live where) — every single value, live or not, gets a
    memory round-trip on every use, which is obviously not the code a
    production compiler would want to ship. It is, however, simple
    enough to *get right* and to verify is right (this is the whole
    point of this pass: proving the pipeline end-to-end, not producing
    tight code yet), and it has one pleasant, non-obvious side benefit
    documented in generate_call() below: because no value is ever kept
    live in a register across an instruction boundary, a `call`
    instruction never needs to save/restore any caller-saved register at
    all — nothing is ever sitting in one that the callee could clobber.
    A real linear-scan or graph-coloring allocator is the documented,
    deferred next step once this pass's correctness is established.

    Scope of instruction selection: integer (`IRType::INT`) arithmetic,
    comparisons, MOVE, ADDR_OF/LOAD/STORE, CALL/RET, and JMP/CJMP are all
    implemented below. `IRType::REAL` (floating point, backed by
    `real_t`/`double` — see ir.hpp/types.hpp) is deliberately *not*
    lowered by this pass: real arithmetic on x86-64 needs the SSE
    register file (`xmm0`-`xmm15`) and an entirely different set of
    mnemonics (`movsd`/`addsd`/`subsd`/`mulsd`/`divsd`/`ucomisd`) plus its
    own argument-passing convention (System V passes floating-point
    arguments in `xmm0`-`xmm7`, not the integer argument registers this
    file's `TargetInfo` already describes) — real, additional work this
    pass's scope did not include, honestly reported here rather than
    silently mis-lowered. lower_instruction() below throws
    `std::logic_error` immediately if it is ever asked to lower an
    `IRType::REAL` instruction, rather than emitting integer instructions
    against operands that are not integers at all.
    @ingroup Compilers
*/

#pragma once

#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>

#include <types.hpp>
#include <array.hpp>
#include <map.hpp>
#include <ir.hpp>
#include <target.hpp>
#include <memorylayout.hpp>

namespace Designar
{
    /** Lowers an IRModule to x86-64 System V assembly text. Stateless
        (every method is `const`, nothing is mutated across calls) — a
        single X86_64CodeGenerator instance can lower any number of
        modules; it exists as a class at all only to give
        generate_function()'s several lowering helpers a natural home
        together, the same "a class purely to group a cohesive set of
        methods that share no mutable state" role LRParserBase's
        protected helpers play for the LR family in lrparser.hpp (there,
        the shared state is `grammar`/`prods`/etc.; here there genuinely
        is none — every helper below takes everything it needs as
        parameters). */
    class X86_64CodeGenerator
    {
    public:
        /** Convenience overload targeting this project's one real
            target, x86-64 System V, directly — see
            target_info_for(Architecture::X86_64). */
        std::string generate_module(const IRModule& module) const
        {
            return generate_module(module, x86_64_sysv_target_info());
        }

        /** The general entry point: lowers every function in `module`
            for `target`. `target.architecture` must be `X86_64` — this
            class only ever knows how to emit x86-64 mnemonics; handing
            it a `TargetInfo` for `AARCH64`/`RISCV64` (which
            target_info_for() cannot even construct yet — see
            target.hpp) would be a caller bug, reported the same way
            every other "this cannot possibly be right" input is
            reported in this file: a thrown exception naming exactly
            what went wrong, never a best-effort guess at what the
            caller "probably" meant. */
        std::string generate_module(const IRModule& module,
                                   const TargetInfo& target) const
        {
            if (target.architecture != Architecture::X86_64)
            {
                throw std::invalid_argument(
                    "X86_64CodeGenerator::generate_module: target "
                    "architecture is '" +
                    to_string(target.architecture) +
                    "', not x86_64 — this class only lowers to x86-64 "
                    "System V assembly (see this file's own comment on "
                    "why AArch64/RISC-V backends are separate, "
                    "not-yet-written files)");
            }

            std::ostringstream out;

            /** GNU `as`'s Intel-syntax mode, selected for exactly this
                translation unit's emitted text — see this file's own
                comment on why Intel syntax was chosen. `.text` puts
                every function that follows in the executable code
                section, the section a `call`/direct jump into any of
                these labels expects to find code in. */
            out << ".intel_syntax noprefix\n";
            out << ".text\n";

            for (const IRFunction& function : module.functions)
            {
                out << generate_function(function, target);
            }

            return out.str();
        }

    private:
        /** Every stack slot this backend hands out for one IRFunction,
            keyed by a short string tag distinguishing a named local
            (`"@" + name`, matching `IRValue::to_string()`'s own `"@"`
            convention for `IRValueKind::LOCAL`) from a virtual register
            (`"t" + id`, likewise matching `IRValue::to_string()`'s `"t"`
            convention for `IRValueKind::VIRTUAL_REGISTER`) — reusing
            that textual convention here is not load-bearing (nothing
            parses these keys back), it is simply a convenient,
            already-established way to keep the two namespaces from
            colliding without inventing a second tagging scheme. */
        struct SlotMap
        {
            HashMap<std::string, int_t> offsets;
            nat_t frame_size_bytes = 0;
        };

        static std::string local_key(const std::string& name)
        {
            return "@" + name;
        }

        static std::string vreg_key(nat_t id)
        {
            return "t" + std::to_string(id);
        }

        /** Scans every operand of `instruction` for a virtual register
            and folds its id into `max_id_plus_one` (raised to
            `id + 1` if higher) — how build_slot_map() below discovers
            how many virtual-register stack slots a function actually
            needs, since IRFunction itself only records "the next unused
            id", not "the highest id actually still referenced by a
            still-live instruction" (constant_fold(), for instance,
            deletes no instructions and introduces no new virtual
            registers, but a future pass that *does* delete dead
            instructions could leave `next_virtual_register_id` reporting
            a count higher than what remains in use — scanning actual
            operands, as done here, stays correct regardless). */
        static void note_max_vreg(const IRValue& value, nat_t& max_id_plus_one)
        {
            if (value.kind == IRValueKind::VIRTUAL_REGISTER &&
                value.register_id + 1 > max_id_plus_one)
            {
                max_id_plus_one = value.register_id + 1;
            }
        }

        /** Builds this function's full stack frame: every named
            local/parameter (via memorylayout.hpp's
            compute_frame_layout(), reused as-is — see this file's own
            comment on why that function is the reusable, target-
            parametrized half of memory organization) plus, immediately
            below the lowest local slot, one more slot per virtual
            register this function's instructions actually reference —
            the "extend it with vreg spill slots as codegen's own
            internal concern" split ir.hpp's own IRLocal comment already
            promises. The combined total is what actually gets rounded
            up to `target.stack_alignment_bytes`, not
            compute_frame_layout()'s own (locals-only) rounding — a
            frame that rounded locals and vreg slots separately could
            round twice and waste alignment padding, or worse, only
            appear aligned when it is not, once both regions are placed
            in the same physical frame below the same `rbp`. */
        SlotMap build_slot_map(const IRFunction& function,
                               const TargetInfo& target) const
        {
            SlotMap slot_map;
            nat_t running_total = 0;

            for (const auto& kv : function.locals)
            {
                if (kv.second.type == IRType::REAL)
                {
                    throw std::logic_error(
                        "X86_64CodeGenerator: local '" + kv.second.name +
                        "' has IRType::REAL — floating-point locals are "
                        "not lowered by this backend yet (see this "
                        "file's own comment on deferred REAL support)");
                }

                running_total += target.pointer_size_bytes;
                slot_map.offsets.insert(local_key(kv.second.name),
                                       -static_cast<int_t>(running_total));
            }

            nat_t max_vreg_id_plus_one = 0;

            for (const IRBasicBlock& block : function.blocks)
            {
                for (const IRInstruction& instruction : block.instructions)
                {
                    note_max_vreg(instruction.dest, max_vreg_id_plus_one);
                    note_max_vreg(instruction.src1, max_vreg_id_plus_one);
                    note_max_vreg(instruction.src2, max_vreg_id_plus_one);

                    for (const IRValue& arg : instruction.call_args)
                    {
                        note_max_vreg(arg, max_vreg_id_plus_one);
                    }
                }
            }

            for (nat_t id = 0; id < max_vreg_id_plus_one; ++id)
            {
                running_total += target.pointer_size_bytes;
                slot_map.offsets.insert(vreg_key(id),
                                       -static_cast<int_t>(running_total));
            }

            if (target.stack_alignment_bytes == 0)
            {
                throw std::invalid_argument(
                    "X86_64CodeGenerator: target.stack_alignment_bytes "
                    "must not be zero");
            }

            nat_t remainder = running_total % target.stack_alignment_bytes;
            slot_map.frame_size_bytes =
                remainder == 0 ? running_total
                              : running_total +
                                    (target.stack_alignment_bytes - remainder);

            return slot_map;
        }

        /** The `[rbp - N]` (or `[rbp + N]`, for a hypothetically
            positive offset — see memorylayout.hpp's LocalSlot comment)
            memory operand text for whichever slot `key` names. Every
            call site below already knows `key` must exist (it was
            derived from an operand build_slot_map() already scanned),
            so a missing entry here means this file's own bookkeeping is
            inconsistent with itself — a real bug, reported as
            `std::logic_error`, never silently producing bogus assembly
            referencing an undefined offset. */
        static std::string mem_operand(const SlotMap& slot_map,
                                       const std::string& key)
        {
            const int_t* offset = slot_map.offsets.search(key);

            if (offset == nullptr)
            {
                throw std::logic_error(
                    "X86_64CodeGenerator: no stack slot recorded for '" +
                    key +
                    "' — this indicates an inconsistency between "
                    "build_slot_map() and the instruction lowering that "
                    "consults it, not a problem with the input IR");
            }

            std::ostringstream out;
            out << "QWORD PTR [rbp" << (*offset >= 0 ? "+" : "") << *offset
                << "]";
            return out.str();
        }

        /** The bare `[rbp±N]` address form, with no `QWORD PTR` size
            prefix — what `lea` needs (ADDR_OF's own lowering, the only
            caller): `lea` only ever *computes* an address, it never
            dereferences memory at all, so there is nothing for a size
            specifier to disambiguate the size of in the first place;
            `mem_operand()` above deliberately always includes one
            because every one of *its* other call sites (`mov`
            to/from memory) does read or write memory at that address,
            where a size specifier is what tells the assembler how many
            bytes that access touches. */
        static std::string address_operand(const SlotMap& slot_map,
                                          const std::string& key)
        {
            const int_t* offset = slot_map.offsets.search(key);

            if (offset == nullptr)
            {
                throw std::logic_error(
                    "X86_64CodeGenerator: no stack slot recorded for '" +
                    key +
                    "' — this indicates an inconsistency between "
                    "build_slot_map() and the instruction lowering that "
                    "consults it, not a problem with the input IR");
            }

            std::ostringstream out;
            out << "[rbp" << (*offset >= 0 ? "+" : "") << *offset << "]";
            return out.str();
        }

        /** Emits `mov reg, <operand>` (or `lea`-free direct load) that
            leaves `value` sitting in `reg` — the one place every
            instruction below reads an IRValue operand from, whatever
            kind it is: an immediate int_t, a virtual register's slot, or
            a named local's slot. `IRValueKind::REAL_CONSTANT` and
            `IRValueKind::NONE` are caller bugs here (an instruction that
            reaches this method with either was never supposed to read
            an operand through it at all) and throw rather than emitting
            nonsense. */
        void emit_load_into(std::ostream& out, const SlotMap& slot_map,
                           const IRValue& value, const std::string& reg) const
        {
            switch (value.kind)
            {
            case IRValueKind::INT_CONSTANT:
                out << "    mov " << reg << ", " << value.int_value << "\n";
                return;
            case IRValueKind::VIRTUAL_REGISTER:
                out << "    mov " << reg << ", "
                    << mem_operand(slot_map, vreg_key(value.register_id))
                    << "\n";
                return;
            case IRValueKind::LOCAL:
                out << "    mov " << reg << ", "
                    << mem_operand(slot_map, local_key(value.name)) << "\n";
                return;
            case IRValueKind::REAL_CONSTANT:
                throw std::logic_error(
                    "X86_64CodeGenerator: encountered an IRType::REAL "
                    "operand — floating-point values are not lowered by "
                    "this backend yet (see this file's own comment)");
            case IRValueKind::NONE:
                throw std::logic_error(
                    "X86_64CodeGenerator: attempted to load an "
                    "IRValue::none() operand — this indicates an "
                    "instruction was lowered against an operand slot it "
                    "should never have read");
            }
        }

        /** The store counterpart to emit_load_into(): writes `reg` back
            to `dest`'s slot. `dest` must be `VIRTUAL_REGISTER` or
            `LOCAL` (never a constant, never `NONE`) — the same
            restriction IRBuilder::emit_move() already documents and
            relies on at IR-construction time; this method simply
            re-checks it at lowering time rather than trusting it
            silently, since a hand-built (not IRBuilder-constructed) IR
            is a supported way to reach this class too (see
            demo-ir.cpp/test-x86_64codegen.cpp, which sometimes build
            IRInstructions directly). */
        void emit_store_from(std::ostream& out, const SlotMap& slot_map,
                            const IRValue& dest, const std::string& reg) const
        {
            switch (dest.kind)
            {
            case IRValueKind::VIRTUAL_REGISTER:
                out << "    mov "
                    << mem_operand(slot_map, vreg_key(dest.register_id))
                    << ", " << reg << "\n";
                return;
            case IRValueKind::LOCAL:
                out << "    mov " << mem_operand(slot_map, local_key(dest.name))
                    << ", " << reg << "\n";
                return;
            default:
                throw std::logic_error(
                    "X86_64CodeGenerator: instruction destination must be "
                    "a virtual register or a named local");
            }
        }

        /** `setCC al; movzx reg, al` — the standard x86 idiom for
            turning a flags-register condition into a 0/1 integer value
            sitting in a general-purpose register, used by every
            CMP_* opcode's lowering below. Signed variants (`setl`/
            `setle`/`setg`/`setge`) are used throughout, matching
            `int_t`'s signedness (types.hpp: `int_t` is `int64_t`) —
            this backend has no unsigned integer IRType at all, so there
            is no case where the unsigned (`setb`/`setbe`/`seta`/`setae`)
            variants would ever be correct here. */
        static const char* set_cc_mnemonic(IROpcode opcode)
        {
            switch (opcode)
            {
            case IROpcode::CMP_EQ:
                return "sete";
            case IROpcode::CMP_NE:
                return "setne";
            case IROpcode::CMP_LT:
                return "setl";
            case IROpcode::CMP_LE:
                return "setle";
            case IROpcode::CMP_GT:
                return "setg";
            case IROpcode::CMP_GE:
                return "setge";
            default:
                throw std::logic_error(
                    "X86_64CodeGenerator::set_cc_mnemonic: opcode is not "
                    "a comparison opcode");
            }
        }

        static bool is_arithmetic_opcode(IROpcode opcode)
        {
            return opcode == IROpcode::ADD || opcode == IROpcode::SUB ||
                   opcode == IROpcode::MUL || opcode == IROpcode::DIV ||
                   opcode == IROpcode::MOD;
        }

        static bool is_compare_opcode(IROpcode opcode)
        {
            return opcode == IROpcode::CMP_EQ || opcode == IROpcode::CMP_NE ||
                   opcode == IROpcode::CMP_LT || opcode == IROpcode::CMP_LE ||
                   opcode == IROpcode::CMP_GT || opcode == IROpcode::CMP_GE;
        }

        /** Lowers one CALL instruction. Argument passing follows
            `target.integer_argument_registers` in order — this backend
            supports at most as many arguments as that list has entries
            (six, for x86-64 System V); a call with more than that would
            need System V's "remaining arguments go on the stack, pushed
            right-to-left, with the stack still 16-byte-aligned at the
            `call` itself" rule, deliberately not implemented here (a
            real, separate piece of work; this pass's IR/backend prove
            the pipeline end-to-end on a small but real subset, not
            every corner of the ABI). No caller-saved register is saved
            or restored around this `call`: see this file's own comment
            on why the "spill everything, nothing survives in a
            register across an instruction boundary" allocation strategy
            makes that always unnecessary here — every argument is
            loaded fresh from its own stack slot into its designated
            argument register immediately before the `call`, and nothing
            this function's own code still needs is ever sitting in a
            register the callee might clobber. */
        void lower_call(std::ostream& out, const SlotMap& slot_map,
                       const TargetInfo& target,
                       const IRInstruction& instruction) const
        {
            if (instruction.call_args.size() >
                target.integer_argument_registers.size())
            {
                throw std::logic_error(
                    "X86_64CodeGenerator: call to '" + instruction.callee +
                    "' passes " +
                    std::to_string(instruction.call_args.size()) +
                    " arguments, more than the " +
                    std::to_string(target.integer_argument_registers.size()) +
                    " this backend passes in registers — stack-passed "
                    "arguments beyond the register-argument count are "
                    "not implemented yet");
            }

            for (nat_t i = 0; i < instruction.call_args.size(); ++i)
            {
                emit_load_into(out, slot_map, instruction.call_args[i],
                              target.integer_argument_registers[i]);
            }

            out << "    call " << instruction.callee << "\n";

            if (!instruction.dest.is_none())
            {
                emit_store_from(out, slot_map, instruction.dest,
                               target.integer_return_register);
            }
        }

        /** Lowers one instruction of a function whose label prefix
            (`label_prefix`) is already known — `true_label`/
            `false_label` on a JMP/CJMP are this IRFunction's own basic
            block labels, and every basic block is emitted as
            `label_prefix + block.label` (see generate_function()) so
            that two different IRFunctions in the same IRModule using
            the same block label (e.g. both naming their entry block
            `"entry"`, exactly as demo-ir.cpp's own factorial/
            store_and_load functions do) never collide in the single,
            flat assembly-text symbol namespace every label lives in. */
        void lower_instruction(std::ostream& out, const SlotMap& slot_map,
                              const TargetInfo& target,
                              const std::string& label_prefix,
                              const IRInstruction& instruction) const
        {
            if (instruction.type == IRType::REAL &&
                (is_arithmetic_opcode(instruction.opcode) ||
                 instruction.opcode == IROpcode::NEG ||
                 instruction.opcode == IROpcode::MOVE ||
                 instruction.opcode == IROpcode::LOAD ||
                 instruction.opcode == IROpcode::STORE))
            {
                throw std::logic_error(
                    "X86_64CodeGenerator: encountered an IRType::REAL "
                    "instruction — floating-point codegen is not "
                    "implemented by this backend yet (see this file's "
                    "own comment on deferred REAL support)");
            }

            if (is_arithmetic_opcode(instruction.opcode))
            {
                if (instruction.opcode == IROpcode::DIV ||
                    instruction.opcode == IROpcode::MOD)
                {
                    /** Signed division: dividend in rax, sign-extended
                        into rdx:rax by `cqo` ("convert quad to octo",
                        the 64-bit form of `cdq`/`cwd`), divisor in any
                        other general-purpose register (`r11` here, this
                        backend's second scratch register — see
                        target.hpp's `TargetInfo::scratch_registers`).
                        Quotient comes back in `rax`, remainder in
                        `rdx` — DIV stores the former, MOD the latter,
                        the only two opcodes below that read from `rdx`
                        instead of from `r10`. */
                    emit_load_into(out, slot_map, instruction.src1, "rax");
                    out << "    cqo\n";
                    emit_load_into(out, slot_map, instruction.src2, "r11");
                    out << "    idiv r11\n";
                    emit_store_from(
                        out, slot_map, instruction.dest,
                        instruction.opcode == IROpcode::DIV ? "rax" : "rdx");
                    return;
                }

                emit_load_into(out, slot_map, instruction.src1, "r10");
                emit_load_into(out, slot_map, instruction.src2, "r11");

                switch (instruction.opcode)
                {
                case IROpcode::ADD:
                    out << "    add r10, r11\n";
                    break;
                case IROpcode::SUB:
                    out << "    sub r10, r11\n";
                    break;
                case IROpcode::MUL:
                    out << "    imul r10, r11\n";
                    break;
                default:
                    throw std::logic_error(
                        "X86_64CodeGenerator: unreachable arithmetic "
                        "opcode branch");
                }

                emit_store_from(out, slot_map, instruction.dest, "r10");
                return;
            }

            if (is_compare_opcode(instruction.opcode))
            {
                emit_load_into(out, slot_map, instruction.src1, "r10");
                emit_load_into(out, slot_map, instruction.src2, "r11");
                out << "    cmp r10, r11\n";
                out << "    " << set_cc_mnemonic(instruction.opcode) << " al\n";
                out << "    movzx r10, al\n";
                emit_store_from(out, slot_map, instruction.dest, "r10");
                return;
            }

            switch (instruction.opcode)
            {
            /** ADD/SUB/MUL/DIV/MOD/CMP_* already returned above (they
                are handled by the is_arithmetic_opcode()/
                is_compare_opcode() blocks before this switch, not by a
                case here) — listed explicitly, falling straight through
                to the shared "unreachable" throw below, purely so this
                switch has no `default:` label to silently swallow a
                genuinely new, forgotten-to-handle IROpcode value added
                in the future; every one of these six is provably dead
                code today. */
            case IROpcode::ADD:
            case IROpcode::SUB:
            case IROpcode::MUL:
            case IROpcode::DIV:
            case IROpcode::MOD:
            case IROpcode::CMP_EQ:
            case IROpcode::CMP_NE:
            case IROpcode::CMP_LT:
            case IROpcode::CMP_LE:
            case IROpcode::CMP_GT:
            case IROpcode::CMP_GE:
                throw std::logic_error(
                    "X86_64CodeGenerator::lower_instruction: unreachable — "
                    "arithmetic/comparison opcodes are handled earlier in "
                    "this function");

            case IROpcode::NEG:
                emit_load_into(out, slot_map, instruction.src1, "r10");
                out << "    neg r10\n";
                emit_store_from(out, slot_map, instruction.dest, "r10");
                return;

            case IROpcode::MOVE:
                emit_load_into(out, slot_map, instruction.src1, "r10");
                emit_store_from(out, slot_map, instruction.dest, "r10");
                return;

            case IROpcode::ADDR_OF:
            {
                if (instruction.src1.kind != IRValueKind::LOCAL)
                {
                    throw std::logic_error(
                        "X86_64CodeGenerator: ADDR_OF operand must be a "
                        "named local");
                }

                out << "    lea r10, "
                    << address_operand(slot_map,
                                      local_key(instruction.src1.name))
                    << "\n";
                emit_store_from(out, slot_map, instruction.dest, "r10");
                return;
            }

            case IROpcode::LOAD:
                emit_load_into(out, slot_map, instruction.src1, "r10");
                out << "    mov r11, [r10]\n";
                emit_store_from(out, slot_map, instruction.dest, "r11");
                return;

            case IROpcode::STORE:
                emit_load_into(out, slot_map, instruction.dest, "r10");
                emit_load_into(out, slot_map, instruction.src1, "r11");
                out << "    mov [r10], r11\n";
                return;

            case IROpcode::CALL:
                lower_call(out, slot_map, target, instruction);
                return;

            case IROpcode::JMP:
                out << "    jmp " << label_prefix << instruction.true_label
                    << "\n";
                return;

            case IROpcode::CJMP:
                emit_load_into(out, slot_map, instruction.src1, "r10");
                out << "    test r10, r10\n";
                out << "    jne " << label_prefix << instruction.true_label
                    << "\n";
                out << "    jmp " << label_prefix << instruction.false_label
                    << "\n";
                return;

            case IROpcode::RET:
                if (!instruction.src1.is_none())
                {
                    emit_load_into(out, slot_map, instruction.src1,
                                  target.integer_return_register);
                }

                /** Function epilogue: undo the prologue's `sub rsp,
                    frame_size` by resetting `rsp` straight to `rbp`
                    (equivalent to, and simpler than, tracking and
                    emitting the exact `add rsp, frame_size` — both leave
                    `rsp == rbp` here, but this form needs no frame-size
                    constant threaded through to every RET site), then
                    `pop rbp` restores the caller's frame pointer and
                    `ret` returns to the caller's return address (the
                    matching pair to the `call` instruction that got us
                    here — the last remaining value the initial `push
                    rbp` left, further down the stack than anything this
                    function's own frame ever touched, is that very
                    return address `call` pushed). */
                out << "    mov rsp, rbp\n";
                out << "    pop rbp\n";
                out << "    ret\n";
                return;
            }

            throw std::logic_error(
                "X86_64CodeGenerator::lower_instruction: unhandled opcode");
        }

        /** Lowers one IRFunction in full: prologue (save the caller's
            frame pointer, establish this function's own, reserve its
            stack frame, copy every incoming register argument into its
            local's slot), every basic block's instructions in turn, and
            relies on IRBasicBlock::has_terminator()'s invariant (every
            block ends in exactly one JMP/CJMP/RET — see ir.hpp) for the
            epilogue: there is no separate "function epilogue" emitted
            after the last block, because IROpcode::RET's own lowering
            (see lower_instruction()) already emits the complete
            epilogue inline, at every return site a function may have,
            exactly mirroring how a real function can have more than one
            `return` statement, each needing the same teardown. */
        std::string generate_function(const IRFunction& function,
                                     const TargetInfo& target) const
        {
            if (function.return_type == IRType::REAL)
            {
                throw std::logic_error(
                    "X86_64CodeGenerator: function '" + function.name +
                    "' returns IRType::REAL — floating-point return "
                    "values are not lowered by this backend yet (see "
                    "this file's own comment)");
            }

            if (function.parameter_order.size() >
                target.integer_argument_registers.size())
            {
                throw std::logic_error(
                    "X86_64CodeGenerator: function '" + function.name +
                    "' declares " +
                    std::to_string(function.parameter_order.size()) +
                    " parameters, more than the " +
                    std::to_string(target.integer_argument_registers.size()) +
                    " this backend accepts in registers — stack-passed "
                    "parameters are not implemented yet");
            }

            SlotMap slot_map = build_slot_map(function, target);
            std::string label_prefix = function.name + "_";

            std::ostringstream out;
            out << ".global " << function.name << "\n";
            out << function.name << ":\n";
            out << "    push rbp\n";
            out << "    mov rbp, rsp\n";
            out << "    sub rsp, " << slot_map.frame_size_bytes << "\n";

            for (nat_t i = 0; i < function.parameter_order.size(); ++i)
            {
                const std::string& param_name = function.parameter_order[i];
                out << "    mov "
                    << mem_operand(slot_map, local_key(param_name)) << ", "
                    << target.integer_argument_registers[i] << "\n";
            }

            for (const IRBasicBlock& block : function.blocks)
            {
                if (!block.has_terminator())
                {
                    throw std::logic_error(
                        "X86_64CodeGenerator: basic block '" + block.label +
                        "' of function '" + function.name +
                        "' does not end in a terminator (JMP/CJMP/RET) — "
                        "every IRBasicBlock produced by IRBuilder upholds "
                        "this invariant by construction (see ir.hpp), so "
                        "this indicates a hand-built IR that skipped it");
                }

                out << label_prefix << block.label << ":\n";

                for (const IRInstruction& instruction : block.instructions)
                {
                    lower_instruction(out, slot_map, target, label_prefix,
                                     instruction);
                }
            }

            return out.str();
        }
    };

} // end namespace Designar
