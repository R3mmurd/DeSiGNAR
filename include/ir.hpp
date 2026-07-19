/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

/** @file ir.hpp
    @brief A three-address-code intermediate representation (IRValue,
    IRInstruction, IRBasicBlock, IRFunction, IRModule), a small builder
    API (IRBuilder) for emitting it, and one IR-level optimization pass
    (constant_fold()).

    This IR is designed to be a natural lowering target for an AST built
    some other way — a Parser<NodeType>-driven semantic action, most
    likely (see parser.hpp) — to walk and emit IRBuilder calls from, and a
    natural input both to further IR-level optimization passes (only
    constant_fold(), below, is implemented so far) and to memorylayout.
    hpp's frame-layout computation, which maps this IR's declared locals
    to concrete stack storage. Wiring an actual `Parser<NodeType>`
    integration up is explicitly out of scope here; nothing below
    references parser.hpp or grammar.hpp at all. Nothing about this IR's
    shape assumes a particular downstream consumer either: a code
    generator is one hypothetical future consumer among others (an
    interpreter directly executing the IR is another), not a committed
    next step for this project.

    What is deliberately *not* attempted here — named explicitly, so a
    future contributor knows it was a choice, not an oversight — is:
    static single assignment form (SSA, with its phi nodes, is a real
    added layer of complexity over the plain three-address code below,
    useful for optimizations this project has not built yet) and a pass
    *pipeline* (constant_fold() is written to the same `IRFunction& ->
    bool changed` shape a pipeline driver would need, but no driver
    exists yet — see that function's own comment; one pass alone has
    nothing to form a pipeline with).

    Three-address code, not a stack machine or a direct-threaded
    interpretation of an AST, because it is the standard, most teachable
    intermediate form for optimization and for staying naturally close to
    real assembly (each IRInstruction below has at most one operator and
    a small, fixed number of operands, the same shape a real machine
    instruction has) — a shape that would keep a hypothetical future
    code generator's lowering simple and mechanical, without this file
    committing to building one.

    Basic blocks are first-class here (IRBasicBlock, holding a flat
    DynArray<IRInstruction> that always ends in exactly one *terminator*
    instruction — JMP, CJMP, or RET, see IRInstruction::is_terminator())
    rather than IRFunction simply holding one flat instruction list with
    jump targets as instruction indices, because basic blocks are what
    make every later "what depends on what" question (register
    allocation liveness, and any future dataflow-based optimization pass)
    tractable to answer at all: within a basic block, control never
    branches or merges, so "instruction N's operand is instruction M's
    result" is a simple, local fact; every place control *can* change is
    exactly the boundary between two IRBasicBlocks. Building this
    structure in from day one, rather than block-ifying a flat list
    later, is the same "get the shape right before building on top of
    it" call this library already made for LR family (see lrparser.hpp:
    the shared LRParserBase driver only works because states/items were
    modeled as first-class things from the start, not bolted on).

    Ownership: every IR node below is a plain value type held directly
    inside a DynArray (IRFunction::locals is the one exception, a HashMap
    keyed by name — see its own comment) — never a heap-allocated node
    behind an owning raw pointer needing an explicit destroy() call, the
    pattern llparser.hpp's ParseTreeNode needs because a parse tree is
    genuinely recursive (an MTreeNode owns its children, which
    themselves own further children, an arbitrarily deep structure that
    cannot be flattened without losing the parent/child relationship
    that *is* the tree). Nothing here has that shape: an IRModule is a
    flat collection of IRFunctions, an IRFunction is a flat collection of
    IRBasicBlocks, and an IRBasicBlock is a flat collection of
    IRInstructions — there is no node that owns a variable number of
    other nodes of its own type, so a DynArray of values (exactly this
    library's existing precedent for that shape — compare Grammar owning
    its own productions as HashMap<Symbol, DynArray<Sequence>> values,
    not as a tree of owned production nodes) is not just *available* here,
    it is the correct, leak-proof-by-construction choice: destroying an
    IRModule destroys its IRFunctions destroys their IRBasicBlocks
    destroys their IRInstructions, entirely via ordinary C++ value
    destruction, with no destroy()/delete call anywhere in this file.
    @ingroup Compilers
*/

#pragma once

#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

#include <types.hpp>
#include <array.hpp>
#include <map.hpp>

namespace Designar
{
    /** The two operand/result types this IR's arithmetic distinguishes —
        `INT` for integer arithmetic (backed by `int_t`, this library's
        64-bit signed integer alias — see types.hpp) and `REAL` for
        floating-point arithmetic (backed by `real_t`, this library's
        `double` alias — types.hpp's own comment is explicit that
        `real_t` *is* how this library spells "floating point"). Carried
        on IRInstruction itself (not inferred from operand kinds) because
        an instruction's *operand* kind alone cannot always disambiguate
        it — e.g. a MOVE between two virtual registers says nothing about
        whether the value flowing through them is an int_t or a real_t,
        but any future code generator lowering that MOVE would absolutely
        need to know — a typical machine moves an INT through a
        general-purpose register and a REAL through a dedicated
        floating-point register, two different instructions with no way
        to pick between them from the MOVE's operands alone). No
        lowering of `REAL` exists in this codebase yet; there is no
        backend of any kind here to lower it. */
    enum class IRType
    {
        INT,
        REAL
    };

    /** What kind of operand/result an IRValue actually is. `NONE` is a
        real, meaningful state (not just "uninitialized"): plenty of
        instructions have no destination at all (STORE, JMP, CJMP, a
        void RET) or no second source operand (NEG, MOVE, ADDR_OF), and
        `IRValue::none()` is what every one of those unused slots holds,
        so a printer/backend can tell "no operand here" apart from "an
        operand that happens to be virtual register 0" without a
        separate boolean alongside every single IRValue field. */
    enum class IRValueKind
    {
        NONE,
        VIRTUAL_REGISTER,
        INT_CONSTANT,
        REAL_CONSTANT,
        LOCAL
    };

    /** One IR-level operand or result: a virtual register (an unbounded,
        purely symbolic id — IRFunction::new_virtual_register() hands out
        0, 1, 2, ... and never reuses one, so "how many virtual registers
        exist" is never a constraint the builder has to think about,
        exactly the point of *virtual* registers — real register
        pressure is entirely a backend/register-allocator concern, not an
        IR-construction one), an immediate constant (int_t or real_t,
        tagged by `kind` so a backend/optimization pass knows which union
        member is live without also consulting the surrounding
        instruction's IRType), or a named local/parameter (resolved to
        actual storage only later, by memorylayout.hpp's
        compute_frame_layout() plus a backend's own lowering — this
        struct itself has no notion of "where" a LOCAL lives).

        Deliberately a plain aggregate with every field always present
        (not a tagged union/std::variant) — the same "small, fixed set of
        cases, direct fields beat an indirection layer" call this
        library already makes for LRProduction/LR1Item in lrparser.hpp;
        an IRValue is copied extremely often (every instruction has two
        or three of them) and a std::variant would add indirection/
        exception-safety machinery this fixed-shape struct has no need
        for. */
    struct IRValue
    {
        IRValueKind kind = IRValueKind::NONE;

        /** Meaningful only when `kind == VIRTUAL_REGISTER`. */
        nat_t register_id = 0;

        /** Meaningful only when `kind == INT_CONSTANT`. */
        int_t int_value = 0;

        /** Meaningful only when `kind == REAL_CONSTANT`. */
        real_t real_value = 0.0;

        /** Meaningful only when `kind == LOCAL` — the name a matching
            IRFunction::declare_local() call registered; looked up again
            (by name, not by some earlier-resolved index) at codegen time
            purely because that keeps IRValue itself decoupled from any
            particular IRFunction's locals table, the same reasoning
            Grammar::Symbol being a plain string (not an interned index
            into some particular Grammar) already follows in grammar.hpp. */
        std::string name;

        static IRValue none()
        {
            return IRValue{};
        }

        static IRValue virtual_register(nat_t id)
        {
            IRValue v;
            v.kind = IRValueKind::VIRTUAL_REGISTER;
            v.register_id = id;
            return v;
        }

        static IRValue int_constant(int_t value)
        {
            IRValue v;
            v.kind = IRValueKind::INT_CONSTANT;
            v.int_value = value;
            return v;
        }

        static IRValue real_constant(real_t value)
        {
            IRValue v;
            v.kind = IRValueKind::REAL_CONSTANT;
            v.real_value = value;
            return v;
        }

        static IRValue local(std::string local_name)
        {
            IRValue v;
            v.kind = IRValueKind::LOCAL;
            v.name = std::move(local_name);
            return v;
        }

        bool is_none() const
        {
            return kind == IRValueKind::NONE;
        }

        bool is_constant() const
        {
            return kind == IRValueKind::INT_CONSTANT ||
                   kind == IRValueKind::REAL_CONSTANT;
        }

        /** A short textual form (`"t3"`, `"42"`, `"3.5"`, `"@x"`), used
            by IRInstruction::to_string() below and therefore by
            demo-ir.cpp/test-ir.cpp's human-readable dumps — never parsed
            back, purely diagnostic. */
        std::string to_string() const
        {
            std::ostringstream out;

            switch (kind)
            {
            case IRValueKind::NONE:
                out << "-";
                break;
            case IRValueKind::VIRTUAL_REGISTER:
                out << "t" << register_id;
                break;
            case IRValueKind::INT_CONSTANT:
                out << int_value;
                break;
            case IRValueKind::REAL_CONSTANT:
                out << real_value;
                break;
            case IRValueKind::LOCAL:
                out << "@" << name;
                break;
            }

            return out.str();
        }
    };

    /** Every operation this IR can express. Grouped below by what a
        backend has to do with each group — arithmetic/comparison
        (produce a value from one or two operands, no control-flow
        effect), data movement (MOVE/ADDR_OF/LOAD/STORE, also no
        control-flow effect), and terminators (CALL is *not* a
        terminator — a call always returns to the very next instruction
        in the same basic block, unlike JMP/CJMP/RET, see
        IRInstruction::is_terminator()). */
    enum class IROpcode
    {
        /** `dest = src1 OP src2`, `type` says whether this is int_t or
            real_t arithmetic (see IRType's own comment for why that
            cannot always be inferred from the operands alone). */
        ADD,
        SUB,
        MUL,
        DIV,
        MOD,

        /** `dest = -src1` (src2 unused, holds IRValue::none()). */
        NEG,

        /** `dest = (src1 OP src2) ? 1 : 0`, result always IRType::INT
            regardless of `type` (`type` here describes what src1/src2
            *are being compared as*, not what the 0/1 result is) — the
            standard "comparisons produce a boolean-as-integer" C-family
            convention, chosen so CJMP below can test the result exactly
            the way it tests any other IRType::INT value, with no
            separate boolean IRType needed anywhere in this file. */
        CMP_EQ,
        CMP_NE,
        CMP_LT,
        CMP_LE,
        CMP_GT,
        CMP_GE,

        /** `dest = src1` (a plain copy — of a constant, a virtual
            register, or a named local, into a virtual register or a
            named local). */
        MOVE,

        /** `dest = &src1`, where `src1` must be `IRValueKind::LOCAL` —
            the operation that *forces* a local's storage to be real
            memory rather than a candidate for a register: see
            memorylayout.hpp's comment on `IRLocal::address_taken` for
            why a value that a running program can take the address of
            can never live only in a register (a register has no
            address an ADDR_OF could hand back at all). */
        ADDR_OF,

        /** `dest = *src1` — src1 must hold a runtime address (as
            produced by ADDR_OF, or passed in from a caller as an
            IRType::INT value); this is genuine pointer dereference, not
            "read a named local" (reading a named local directly is
            simply MOVE with an `IRValueKind::LOCAL` source — LOAD/STORE
            exist for the address-taken/pointer case that MOVE cannot
            express at all). */
        LOAD,

        /** `*dest = src1` — dest must hold a runtime address, the
            store-through-pointer counterpart to LOAD. */
        STORE,

        /** `dest = callee(call_args...)` — a direct call by name (this
            IR has no function-pointer/indirect-call value kind yet, a
            scope cut documented here rather than silently; nothing
            about the IRFunction/IRModule shape below would prevent
            adding one later). Deliberately *not* a terminator: control
            always resumes at the very next instruction in the same
            basic block once the callee returns, the same reason a real
            machine's `call` instruction does not end a basic block in
            any conventional definition of the term either. Parameter
            passing is modeled here purely as "here is the ordered list
            of IRValues this call passes" (`IRInstruction::call_args`)
            with no calling-convention detail (which register/stack slot
            each argument lands in) anywhere in this file — assigning
            arguments to physical registers/stack slots per some real ABI
            is entirely a hypothetical future code generator's job, never
            this IR's. */
        CALL,

        /** Unconditional jump to `true_label` (the basic block boundary
            this file's comment calls the "make register allocation and
            optimization tractable" reason for having blocks at all). */
        JMP,

        /** `if (src1 != 0) goto true_label; else goto false_label;` —
            always both-armed (no "fall through if false" variant), so
            every IRBasicBlock this instruction ends always has exactly
            two successors named explicitly, never an implicit
            "whichever block happens to be next in the DynArray" one;
            that omission would make basic blocks order-dependent in a
            way that breaks the moment an optimization pass reorders or
            removes one. */
        CJMP,

        /** Returns from the current function; `src1` is the returned
            value (`IRValue::none()` for a function whose IRFunction::
            return_type is meaningless, i.e. effectively void — this
            file does not model a distinct "void" IRType, since the
            only place that would matter is exactly this one already-
            explicit `IRValue::none()` case). */
        RET
    };

    /** One three-address instruction: an opcode, the two operand types
        every arithmetic/comparison/data-movement opcode needs
        (`dest`/`src1`/`src2` — unused slots hold `IRValue::none()`), and
        the handful of opcode-specific extra fields (jump-target labels,
        a callee name, a call's argument list) that do not apply to most
        instructions but are cheap enough (a couple of strings, one
        DynArray that stays empty unless this is a CALL) to simply carry
        on every IRInstruction value rather than modeling a separate
        instruction subtype per opcode — the same "one shape, not one
        subclass per case" call already made by IRValue above.

        Held by value in IRBasicBlock::instructions (see this file's own
        comment on ownership) — an IRInstruction owns nothing that
        outlives it (its `std::string`/`DynArray` fields are the
        instruction's own data, never a pointer/reference elsewhere), so
        copying, moving, or destroying one is exactly as safe and
        leak-free as copying, moving, or destroying any other ordinary
        C++ value. */
    struct IRInstruction
    {
        IROpcode opcode;
        IRType type = IRType::INT;

        IRValue dest = IRValue::none();
        IRValue src1 = IRValue::none();
        IRValue src2 = IRValue::none();

        /** JMP's only target; CJMP's "condition true" target. Unused
            (left empty) for every other opcode. */
        std::string true_label;

        /** CJMP's "condition false" target only. */
        std::string false_label;

        /** CALL's callee name only. */
        std::string callee;

        /** CALL's argument list only, in left-to-right parameter order —
            empty for every other opcode. */
        DynArray<IRValue> call_args;

        /** Whether this instruction ends its basic block. Exactly JMP,
            CJMP, and RET — CALL is deliberately excluded (see CALL's own
            comment above); every IRBasicBlock below is required to end
            in exactly one of these three and to contain none of them
            anywhere else, the invariant that keeps "which block does
            control reach next" always answerable by looking only at a
            block's last instruction. */
        bool is_terminator() const
        {
            return opcode == IROpcode::JMP || opcode == IROpcode::CJMP ||
                   opcode == IROpcode::RET;
        }

        /** A one-line, human-readable rendering (`"t2 = t0 + t1"`,
            `"cjmp t3, then, else"`, ...) — purely diagnostic, used by
            IRFunction::to_string()/demo-ir.cpp/test-ir.cpp, never parsed
            back into an IRInstruction. */
        std::string to_string() const
        {
            std::ostringstream out;

            auto binop = [&](const char* sym)
            {
                out << dest.to_string() << " = " << src1.to_string() << " "
                    << sym << " " << src2.to_string();
            };

            switch (opcode)
            {
            case IROpcode::ADD:
                binop("+");
                break;
            case IROpcode::SUB:
                binop("-");
                break;
            case IROpcode::MUL:
                binop("*");
                break;
            case IROpcode::DIV:
                binop("/");
                break;
            case IROpcode::MOD:
                binop("%");
                break;
            case IROpcode::NEG:
                out << dest.to_string() << " = -" << src1.to_string();
                break;
            case IROpcode::CMP_EQ:
                binop("==");
                break;
            case IROpcode::CMP_NE:
                binop("!=");
                break;
            case IROpcode::CMP_LT:
                binop("<");
                break;
            case IROpcode::CMP_LE:
                binop("<=");
                break;
            case IROpcode::CMP_GT:
                binop(">");
                break;
            case IROpcode::CMP_GE:
                binop(">=");
                break;
            case IROpcode::MOVE:
                out << dest.to_string() << " = " << src1.to_string();
                break;
            case IROpcode::ADDR_OF:
                out << dest.to_string() << " = &" << src1.to_string();
                break;
            case IROpcode::LOAD:
                out << dest.to_string() << " = *" << src1.to_string();
                break;
            case IROpcode::STORE:
                out << "*" << dest.to_string() << " = " << src1.to_string();
                break;
            case IROpcode::CALL:
                if (!dest.is_none())
                {
                    out << dest.to_string() << " = ";
                }

                out << "call " << callee << "(";

                for (nat_t i = 0; i < call_args.size(); ++i)
                {
                    if (i > 0)
                    {
                        out << ", ";
                    }

                    out << call_args[i].to_string();
                }

                out << ")";
                break;
            case IROpcode::JMP:
                out << "jmp " << true_label;
                break;
            case IROpcode::CJMP:
                out << "cjmp " << src1.to_string() << ", " << true_label
                    << ", " << false_label;
                break;
            case IROpcode::RET:
                out << "ret";

                if (!src1.is_none())
                {
                    out << " " << src1.to_string();
                }

                break;
            }

            return out.str();
        }
    };

    /** A basic block: a label naming it (unique within its IRFunction —
        how JMP/CJMP name their targets and how IRFunction::find_block()
        looks one up) plus a flat, ordered list of instructions that must
        end in exactly one terminator (see IRInstruction::is_terminator())
        and must not contain one anywhere else. IRBuilder is the only
        thing in this file that constructs these, and it upholds that
        invariant by construction (see IRBuilder::emit_jmp()/emit_cjmp()/
        emit_ret()'s shared comment) rather than this struct enforcing it
        itself — the same "the builder is the gatekeeper, the data
        structure itself stays a plain aggregate" split IRValue/
        IRInstruction already use. */
    struct IRBasicBlock
    {
        std::string label;
        DynArray<IRInstruction> instructions;

        bool has_terminator() const
        {
            return !instructions.is_empty() &&
                   instructions[instructions.size() - 1].is_terminator();
        }
    };

    /** One declared local or parameter of an IRFunction: a name, its
        IRType, whether it is one of the function's own parameters
        (`is_parameter`, in the order IRFunction::parameter_order records
        — this is how memorylayout.hpp knows which locals would need
        their *incoming* value copied out of an argument register/stack
        slot at function entry, rather than starting out uninitialized,
        if a future code generator existed to do that copying), and
        `address_taken` — see memorylayout.hpp's FrameLayout/LocalSlot
        comment for exactly what that flag forces a memory-layout
        computation to do and why. */
    struct IRLocal
    {
        std::string name;
        IRType type = IRType::INT;
        bool is_parameter = false;

        /** Set by IRFunction::declare_local()'s caller (never inferred
            here — this file has no dataflow analysis that could infer
            it) whenever an ADDR_OF instruction is ever going to be
            emitted against this local. A local with this flag left
            false is only ever a *candidate* for a register in some
            hypothetical future register allocator — no such allocator
            exists in this codebase, so right now this flag only changes
            what compute_frame_layout() records (see memorylayout.hpp:
            it currently gives every local, `address_taken` or not, a
            real memory slot); it is recorded here so that a future
            allocator, were one ever written, would have everything it
            needs to know already tracked rather than needing this
            struct extended retroactively. */
        bool address_taken = false;
    };

    /** One function: a name, its return IRType, the ordered names of its
        own parameters (a subset of `locals`, in calling order), every
        local/parameter it declares (keyed by name — a HashMap, not a
        DynArray, specifically because IRValue::LOCAL/IRInstruction look
        a local up *by name* at codegen time, potentially many times
        across many instructions, so this is the one place in this file
        an O(1)-search container earns its keep over a linear scan, the
        same tradeoff Grammar's own `productions`/`precedence` maps make
        in grammar.hpp), and its basic blocks (see this file's own
        comment on why blocks, not a flat instruction list). Also a
        virtual-register counter, so IRBuilder::new_virtual_register()
        never has to scan existing instructions to find an unused id. */
    class IRFunction
    {
    public:
        std::string name;
        IRType return_type = IRType::INT;
        DynArray<std::string> parameter_order;
        HashMap<std::string, IRLocal> locals;
        DynArray<IRBasicBlock> blocks;

        explicit IRFunction(std::string function_name,
                            IRType function_return_type = IRType::INT)
            : name(std::move(function_name)), return_type(function_return_type)
        {
            // empty
        }

        /** Hands out the next unused virtual register id for this
            function — ids are never reused, even across passes that
            delete instructions (constant_fold() below, for instance,
            never introduces a fresh virtual register at all, but a
            future pass that does should still feel free to always call
            this rather than ever trying to recycle an id, since nothing
            about "is id N still referenced anywhere" is tracked here). */
        IRValue new_virtual_register()
        {
            return IRValue::virtual_register(next_virtual_register_id++);
        }

        /** Registers a local/parameter named `local_name`; throws if that
            name is already declared (a duplicate declaration is always a
            caller bug — this IR has no notion of shadowing/scoping at
            all, every local of a function lives in one flat namespace). */
        IRLocal& declare_local(std::string local_name, IRType type,
                              bool is_parameter = false,
                              bool address_taken = false)
        {
            if (locals.search(local_name) != nullptr)
            {
                throw std::invalid_argument(
                    "IRFunction::declare_local: '" + local_name +
                    "' is already declared on function '" + name + "'");
            }

            IRLocal local{local_name, type, is_parameter, address_taken};

            if (is_parameter)
            {
                parameter_order.append(local_name);
            }

            return *locals.insert(std::move(local_name), std::move(local));
        }

        /** Appends a new, empty basic block labeled `label` and returns
            its index; throws if `label` is already used by this
            function — labels are how JMP/CJMP/IRFunction::find_block()
            name a block, so a duplicate would make that lookup
            ambiguous. The returned index stays valid for the rest of
            this IRFunction's lifetime (IRBasicBlock is held by value in
            a DynArray that only ever grows via this method — nothing in
            this file ever removes a block once appended, so no earlier
            index is ever invalidated by a later create_block() call;
            DynArray::append() only ever reallocates its own backing
            storage, it never changes what a *previously returned index*
            refers to). */
        nat_t create_block(const std::string& label)
        {
            if (find_block(label) != nullptr)
            {
                throw std::invalid_argument(
                    "IRFunction::create_block: label '" + label +
                    "' is already used in function '" + name + "'");
            }

            nat_t index = blocks.size();
            IRBasicBlock block;
            block.label = label;
            blocks.append(std::move(block));
            return index;
        }

        IRBasicBlock* find_block(const std::string& label)
        {
            for (nat_t i = 0; i < blocks.size(); ++i)
            {
                if (blocks[i].label == label)
                {
                    return &blocks[i];
                }
            }

            return nullptr;
        }

        const IRBasicBlock* find_block(const std::string& label) const
        {
            for (nat_t i = 0; i < blocks.size(); ++i)
            {
                if (blocks[i].label == label)
                {
                    return &blocks[i];
                }
            }

            return nullptr;
        }

        /** A full, human-readable dump of every block/instruction —
            purely diagnostic (demo-ir.cpp prints this both before and
            after running constant_fold(), so the pass's effect is
            visible, and test-ir.cpp uses it for failure messages). */
        std::string to_string() const
        {
            std::ostringstream out;
            out << "function " << name << ":\n";

            for (const IRBasicBlock& block : blocks)
            {
                out << block.label << ":\n";

                for (const IRInstruction& instruction : block.instructions)
                {
                    out << "    " << instruction.to_string() << "\n";
                }
            }

            return out.str();
        }

    private:
        nat_t next_virtual_register_id = 0;
    };

    /** A translation unit's worth of IR: a name and its functions, held
        by value (see this file's own comment on ownership) — destroying
        an IRModule destroys every IRFunction it holds, transitively
        destroying every basic block and instruction inside each, with no
        explicit cleanup call needed anywhere. */
    class IRModule
    {
    public:
        std::string name;
        DynArray<IRFunction> functions;

        explicit IRModule(std::string module_name) : name(std::move(module_name))
        {
            // empty
        }

        /** Appends a new, empty IRFunction and returns a reference to it
            — a reference, not an index, is safe to keep across further
            IRBuilder calls on *that same function* (which never appends
            to `functions` itself), but must not be kept across a
            further call to create_function() on `this` module (which
            can reallocate `functions`' backing storage, the same
            dangling-reference hazard this library documents at several
            other append-then-keep-a-reference call sites, e.g.
            LRParserBase::lr0_closure()'s comment in lrparser.hpp). */
        IRFunction& create_function(std::string function_name,
                                    IRType return_type = IRType::INT)
        {
            functions.append(IRFunction(std::move(function_name), return_type));
            return functions[functions.size() - 1];
        }

        IRFunction* find_function(const std::string& function_name)
        {
            for (nat_t i = 0; i < functions.size(); ++i)
            {
                if (functions[i].name == function_name)
                {
                    return &functions[i];
                }
            }

            return nullptr;
        }

        std::string to_string() const
        {
            std::ostringstream out;
            out << "module " << name << "\n";

            for (const IRFunction& function : functions)
            {
                out << function.to_string();
            }

            return out.str();
        }
    };

    /** Emits instructions into one IRFunction, one basic block at a
        time — the "natural AST-lowering target" this file's own comment
        promises: a caller walking some AST (built however it likes;
        wiring this up to Parser<NodeType>'s semantic actions is out of
        scope for this pass, see this file's own comment) calls
        `builder.emit_add(...)`/`builder.emit_cjmp(...)`/etc. in the
        order it wants those instructions to execute, and this class
        takes care of allocating destination virtual registers and
        appending each finished IRInstruction to whichever basic block is
        currently selected.

        Holds a reference to the IRFunction it emits into (not a copy —
        an IRBuilder is a thin, short-lived cursor over a function that
        already exists, not an owner of one); a caller is responsible for
        keeping that IRFunction alive for as long as the IRBuilder over
        it is in use, exactly the same "keeps a reference, does not
        own it" relationship Grammar has with every LRParserBase-derived
        parser built over it (see lrparser.hpp). */
    class IRBuilder
    {
        IRFunction& function;
        nat_t current_block_index;

        IRBasicBlock& current_block()
        {
            return function.blocks[current_block_index];
        }

        void append(IRInstruction instruction)
        {
            if (current_block().has_terminator())
            {
                throw std::logic_error(
                    "IRBuilder: cannot append another instruction to basic "
                    "block '" +
                    current_block().label +
                    "' after it already ends in a terminator (JMP/CJMP/"
                    "RET) — select a different block first (see "
                    "set_insertion_block()/create_block())");
            }

            current_block().instructions.append(std::move(instruction));
        }

        IRValue emit_binary(IROpcode opcode, IRType type, IRValue lhs,
                            IRValue rhs)
        {
            IRValue dest = function.new_virtual_register();
            IRInstruction instruction;
            instruction.opcode = opcode;
            instruction.type = type;
            instruction.dest = dest;
            instruction.src1 = std::move(lhs);
            instruction.src2 = std::move(rhs);
            append(std::move(instruction));
            return dest;
        }

    public:
        /** Starts emitting into `f`'s *last* basic block, i.e. whichever
            one `f` already had (typically none yet — a fresh IRFunction
            starts with zero blocks, so a caller almost always follows
            this constructor immediately with `create_block()`). */
        explicit IRBuilder(IRFunction& f)
            : function(f),
              current_block_index(f.blocks.is_empty() ? 0 : f.blocks.size() - 1)
        {
            // empty
        }

        /** Creates a new basic block on the underlying function and
            immediately selects it as the insertion point — the common
            case (a caller almost never wants to create a block without
            also starting to fill it in); use
            `set_insertion_block(function.create_block(label))` directly
            instead only for the rarer case of forward-declaring a block
            (e.g. a loop's exit block, whose label a CJMP needs to name
            before the block's own instructions are ready to emit) ahead
            of filling it in. */
        nat_t create_block(const std::string& label)
        {
            current_block_index = function.create_block(label);
            return current_block_index;
        }

        void set_insertion_block(nat_t block_index)
        {
            current_block_index = block_index;
        }

        IRValue new_virtual_register()
        {
            return function.new_virtual_register();
        }

        IRValue emit_add(IRType type, IRValue lhs, IRValue rhs)
        {
            return emit_binary(IROpcode::ADD, type, std::move(lhs), std::move(rhs));
        }

        IRValue emit_sub(IRType type, IRValue lhs, IRValue rhs)
        {
            return emit_binary(IROpcode::SUB, type, std::move(lhs), std::move(rhs));
        }

        IRValue emit_mul(IRType type, IRValue lhs, IRValue rhs)
        {
            return emit_binary(IROpcode::MUL, type, std::move(lhs), std::move(rhs));
        }

        IRValue emit_div(IRType type, IRValue lhs, IRValue rhs)
        {
            return emit_binary(IROpcode::DIV, type, std::move(lhs), std::move(rhs));
        }

        IRValue emit_mod(IRType type, IRValue lhs, IRValue rhs)
        {
            return emit_binary(IROpcode::MOD, type, std::move(lhs), std::move(rhs));
        }

        IRValue emit_neg(IRType type, IRValue operand)
        {
            IRValue dest = function.new_virtual_register();
            IRInstruction instruction;
            instruction.opcode = IROpcode::NEG;
            instruction.type = type;
            instruction.dest = dest;
            instruction.src1 = std::move(operand);
            append(std::move(instruction));
            return dest;
        }

        IRValue emit_cmp_eq(IRType type, IRValue lhs, IRValue rhs)
        {
            return emit_binary(IROpcode::CMP_EQ, type, std::move(lhs), std::move(rhs));
        }

        IRValue emit_cmp_ne(IRType type, IRValue lhs, IRValue rhs)
        {
            return emit_binary(IROpcode::CMP_NE, type, std::move(lhs), std::move(rhs));
        }

        IRValue emit_cmp_lt(IRType type, IRValue lhs, IRValue rhs)
        {
            return emit_binary(IROpcode::CMP_LT, type, std::move(lhs), std::move(rhs));
        }

        IRValue emit_cmp_le(IRType type, IRValue lhs, IRValue rhs)
        {
            return emit_binary(IROpcode::CMP_LE, type, std::move(lhs), std::move(rhs));
        }

        IRValue emit_cmp_gt(IRType type, IRValue lhs, IRValue rhs)
        {
            return emit_binary(IROpcode::CMP_GT, type, std::move(lhs), std::move(rhs));
        }

        IRValue emit_cmp_ge(IRType type, IRValue lhs, IRValue rhs)
        {
            return emit_binary(IROpcode::CMP_GE, type, std::move(lhs), std::move(rhs));
        }

        /** `dest = src` — `dest` must be `VIRTUAL_REGISTER` or `LOCAL`
            (never a constant, never `NONE`); returns `dest` unchanged,
            purely for call-site convenience (`x = builder.emit_move(x,
            v)` reads naturally when `dest` already exists, e.g. an
            existing local being written to, unlike every `emit_*` above
            which always allocates a *fresh* destination register). */
        IRValue emit_move(IRValue dest, IRValue src)
        {
            IRInstruction instruction;
            instruction.opcode = IROpcode::MOVE;
            instruction.dest = dest;
            instruction.src1 = std::move(src);
            append(std::move(instruction));
            return dest;
        }

        /** `dest = &local` — see IROpcode::ADDR_OF's own comment;
            `local` must be `IRValueKind::LOCAL`. */
        IRValue emit_addr_of(IRValue local)
        {
            if (local.kind != IRValueKind::LOCAL)
            {
                throw std::invalid_argument(
                    "IRBuilder::emit_addr_of: operand must be an "
                    "IRValueKind::LOCAL — taking the address of a "
                    "virtual register or a constant has no meaning in "
                    "this IR");
            }

            IRValue dest = function.new_virtual_register();
            IRInstruction instruction;
            instruction.opcode = IROpcode::ADDR_OF;
            instruction.type = IRType::INT;
            instruction.dest = dest;
            instruction.src1 = std::move(local);
            append(std::move(instruction));
            return dest;
        }

        IRValue emit_load(IRType type, IRValue address)
        {
            IRValue dest = function.new_virtual_register();
            IRInstruction instruction;
            instruction.opcode = IROpcode::LOAD;
            instruction.type = type;
            instruction.dest = dest;
            instruction.src1 = std::move(address);
            append(std::move(instruction));
            return dest;
        }

        void emit_store(IRValue address, IRValue value)
        {
            IRInstruction instruction;
            instruction.opcode = IROpcode::STORE;
            instruction.dest = std::move(address);
            instruction.src1 = std::move(value);
            append(std::move(instruction));
        }

        /** `dest = callee(args...)` (returning `dest`) — see
            IROpcode::CALL's own comment on why parameter passing here is
            just "an ordered list of IRValues", with every calling-
            convention detail left to a backend. */
        IRValue emit_call(const std::string& callee, DynArray<IRValue> args,
                          IRType return_type)
        {
            IRValue dest = function.new_virtual_register();
            IRInstruction instruction;
            instruction.opcode = IROpcode::CALL;
            instruction.type = return_type;
            instruction.dest = dest;
            instruction.callee = callee;
            instruction.call_args = std::move(args);
            append(std::move(instruction));
            return dest;
        }

        /** A void call — same as emit_call() but with no result kept
            (dest still gets a fresh virtual register internally, purely
            because IRInstruction::dest has no separate "no result"
            representation beyond `IRValue::none()`, which this method
            uses directly instead of allocating one nothing will ever
            read). */
        void emit_call_ignore_result(const std::string& callee,
                                    DynArray<IRValue> args)
        {
            IRInstruction instruction;
            instruction.opcode = IROpcode::CALL;
            instruction.dest = IRValue::none();
            instruction.callee = callee;
            instruction.call_args = std::move(args);
            append(std::move(instruction));
        }

        /** Ends the current basic block with an unconditional jump.
            Every terminator-emitting method here (this one, emit_cjmp(),
            emit_ret()) is how IRBasicBlock's "ends in exactly one
            terminator" invariant is actually upheld: append() above
            refuses to add anything to a block that already has one, and
            nothing in this file appends a JMP/CJMP/RET any way other
            than through these three methods. */
        void emit_jmp(const std::string& target_label)
        {
            IRInstruction instruction;
            instruction.opcode = IROpcode::JMP;
            instruction.true_label = target_label;
            append(std::move(instruction));
        }

        void emit_cjmp(IRValue condition, const std::string& true_label,
                      const std::string& false_label)
        {
            IRInstruction instruction;
            instruction.opcode = IROpcode::CJMP;
            instruction.src1 = std::move(condition);
            instruction.true_label = true_label;
            instruction.false_label = false_label;
            append(std::move(instruction));
        }

        void emit_ret(IRValue value = IRValue::none())
        {
            IRInstruction instruction;
            instruction.opcode = IROpcode::RET;
            instruction.src1 = std::move(value);
            append(std::move(instruction));
        }
    };

    namespace IRDetail
    {
        /** Evaluates `op` on two already-folded int_t constants; shared
            between constant_fold()'s INT arithmetic and comparison cases
            below. Division/modulo by zero are handled by the caller
            (constant_fold() never calls this for a DIV/MOD whose divisor
            is zero — see its own comment on why folding that case at
            compile time would be wrong, not just inconvenient). */
        inline int_t fold_int_arith(IROpcode op, int_t a, int_t b)
        {
            switch (op)
            {
            case IROpcode::ADD:
                return a + b;
            case IROpcode::SUB:
                return a - b;
            case IROpcode::MUL:
                return a * b;
            case IROpcode::DIV:
                return a / b;
            case IROpcode::MOD:
                return a % b;
            default:
                throw std::logic_error(
                    "IRDetail::fold_int_arith: opcode is not an integer "
                    "arithmetic opcode");
            }
        }

        inline real_t fold_real_arith(IROpcode op, real_t a, real_t b)
        {
            switch (op)
            {
            case IROpcode::ADD:
                return a + b;
            case IROpcode::SUB:
                return a - b;
            case IROpcode::MUL:
                return a * b;
            case IROpcode::DIV:
                return a / b;
            default:
                throw std::logic_error(
                    "IRDetail::fold_real_arith: opcode is not a real "
                    "arithmetic opcode with a folding rule (MOD has no "
                    "real_t case at all — see constant_fold())");
            }
        }

        /** CMP_EQ/CMP_NE's `==`/`!=` below trip `-Wfloat-equal` for this
            template's `T = real_t` instantiation (used for `IRType::
            REAL` operands) — this library's own
            `-DDESIGNAR_WARNINGS_AS_ERRORS=ON` CI build treats that
            warning as an error (see CMakeLists.txt's own comment on
            why: exact floating-point equality is usually a sign of a
            bug). It is not one here: constant-folding CMP_EQ/CMP_NE
            must compute *exactly* what the un-folded instruction's own
            runtime comparison would have (a real machine compares
            IRType::REAL operands against the raw IEEE-754 equality
            flag, with no tolerance of any kind) — silently swapping
            in an approximate/epsilon comparison here to satisfy the
            warning would make the *optimized* program's semantics
            subtly diverge from the un-optimized one, which is a far
            worse bug than the warning is trying to prevent. The
            `#pragma` below suppresses it for exactly this one
            genuinely-intentional comparison, narrowly, rather than
            rephrasing the comparison into something that no longer
            means what it must mean. */
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-equal"
#endif
        template <class T>
        inline bool fold_compare(IROpcode op, T a, T b)
        {
            switch (op)
            {
            case IROpcode::CMP_EQ:
                return a == b;
            case IROpcode::CMP_NE:
                return a != b;
            case IROpcode::CMP_LT:
                return a < b;
            case IROpcode::CMP_LE:
                return a <= b;
            case IROpcode::CMP_GT:
                return a > b;
            case IROpcode::CMP_GE:
                return a >= b;
            default:
                throw std::logic_error(
                    "IRDetail::fold_compare: opcode is not a comparison "
                    "opcode");
            }
        }
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif
    } // end namespace IRDetail

    /** The first (and, for this pass of the project, only) IR-level
        optimization pass: constant folding — replacing an arithmetic or
        comparison instruction whose operands are *both* already-known
        constants with a plain MOVE of the already-computed result,
        e.g. `t2 = 2 + 3` becomes `t2 = 5`. Chosen over the other
        candidate this project considered (dead/unreachable-basic-block
        elimination) because it is the simplest classical optimization
        to implement *correctly* end-to-end and to demonstrate concretely
        (demo-ir.cpp folds a hand-built expression and prints the IR
        before/after so the effect is directly visible) — establishing
        the pattern a future pass pipeline would follow, not attempting
        to be a complete or even a particularly impactful optimizer yet.

        Signature is `IRFunction& -> bool` (returns whether anything
        changed) specifically so a future pass *pipeline* driver can run
        a fixed list of such passes in a loop until none of them report a
        change, without this pass needing to change shape when that
        driver is eventually written — see this file's own comment on
        why no such driver exists yet (one pass alone has nothing to
        form a pipeline with).

        Division and modulo by a constant zero are deliberately *never*
        folded, even though both operands are known constants: dividing
        by zero is undefined behavior for int_t at compile time (this
        function runs in the optimizer, a C++ program, and triggering
        real UB while compiling someone else's program is never
        acceptable) and, more importantly, is part of the *program this
        IR describes* own observable semantics — division/modulo by zero
        is a real, defined-to-be-undefined-or-trapping operation on every
        real machine (e.g. x86-64's `idiv` raises `#DE`, a hardware fault,
        at run time), and leaving the unfolded DIV/MOD instruction in
        place preserves that; folding it away at compile time would
        silently change what the described program does, on any future
        consumer that actually executes or lowers it. */
    inline bool constant_fold(IRFunction& function)
    {
        bool changed = false;

        for (IRBasicBlock& block : function.blocks)
        {
            for (IRInstruction& instruction : block.instructions)
            {
                bool is_int_arith = instruction.opcode == IROpcode::ADD ||
                                   instruction.opcode == IROpcode::SUB ||
                                   instruction.opcode == IROpcode::MUL ||
                                   instruction.opcode == IROpcode::DIV ||
                                   instruction.opcode == IROpcode::MOD;

                bool is_real_arith = instruction.opcode == IROpcode::ADD ||
                                    instruction.opcode == IROpcode::SUB ||
                                    instruction.opcode == IROpcode::MUL ||
                                    instruction.opcode == IROpcode::DIV;

                bool is_compare = instruction.opcode == IROpcode::CMP_EQ ||
                                 instruction.opcode == IROpcode::CMP_NE ||
                                 instruction.opcode == IROpcode::CMP_LT ||
                                 instruction.opcode == IROpcode::CMP_LE ||
                                 instruction.opcode == IROpcode::CMP_GT ||
                                 instruction.opcode == IROpcode::CMP_GE;

                if (!is_int_arith && !is_compare)
                {
                    continue;
                }

                if (!instruction.src1.is_constant() ||
                    !instruction.src2.is_constant())
                {
                    continue;
                }

                if (instruction.type == IRType::INT)
                {
                    if (instruction.src1.kind != IRValueKind::INT_CONSTANT ||
                        instruction.src2.kind != IRValueKind::INT_CONSTANT)
                    {
                        continue;
                    }

                    int_t a = instruction.src1.int_value;
                    int_t b = instruction.src2.int_value;

                    bool is_div_or_mod = instruction.opcode == IROpcode::DIV ||
                                       instruction.opcode == IROpcode::MOD;

                    if (is_div_or_mod && b == 0)
                    {
                        continue;
                    }

                    IRValue folded;

                    if (is_compare)
                    {
                        folded = IRValue::int_constant(
                            IRDetail::fold_compare(instruction.opcode, a, b)
                                ? 1
                                : 0);
                    }
                    else
                    {
                        folded = IRValue::int_constant(
                            IRDetail::fold_int_arith(instruction.opcode, a, b));
                    }

                    instruction.opcode = IROpcode::MOVE;
                    instruction.src1 = folded;
                    instruction.src2 = IRValue::none();
                    changed = true;
                }
                else
                {
                    if (!is_real_arith && !is_compare)
                    {
                        continue;
                    }

                    if (instruction.src1.kind != IRValueKind::REAL_CONSTANT ||
                        instruction.src2.kind != IRValueKind::REAL_CONSTANT)
                    {
                        continue;
                    }

                    real_t a = instruction.src1.real_value;
                    real_t b = instruction.src2.real_value;

                    IRValue folded;

                    if (is_compare)
                    {
                        folded = IRValue::int_constant(
                            IRDetail::fold_compare(instruction.opcode, a, b)
                                ? 1
                                : 0);
                    }
                    else
                    {
                        folded = IRValue::real_constant(
                            IRDetail::fold_real_arith(instruction.opcode, a, b));
                    }

                    instruction.opcode = IROpcode::MOVE;
                    instruction.type =
                        is_compare ? IRType::INT : IRType::REAL;
                    instruction.src1 = folded;
                    instruction.src2 = IRValue::none();
                    changed = true;
                }
            }
        }

        return changed;
    }

} // end namespace Designar
