/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

/** @file target.hpp
    @brief The seam every code-generation backend (x86_64codegen.hpp today;
    an AArch64 and a RISC-V equivalent are the deliberately-deferred future
    work this file's own doc comments keep pointing at) is written against:
    `Architecture`, `host_architecture()`, and `TargetInfo`.

    This is Phase 1 of a multi-pass code-generation initiative — see
    ir.hpp's file comment for the full phasing story (IR design, one
    optimization pass, memory layout, one real backend). This particular
    file's job is narrow: name the architectures this project knows about,
    say which one *this* translation unit was compiled for, and describe,
    in backend-agnostic terms, the handful of facts a code generator needs
    about whichever architecture it targets (pointer width, integer
    argument/return registers, which registers a callee must preserve,
    required stack alignment). Only `Architecture::X86_64` has a real
    `TargetInfo` behind it right now (see target_info_for() below);
    `AARCH64` and `RISCV64` are named here — and only here — so that
    x86_64codegen.hpp's *interface* (and any future aarch64codegen.hpp/
    riscv64codegen.hpp written against this same TargetInfo shape) needs
    no restructuring when a real implementation for either shows up later,
    only a new backend file plus a new branch in target_info_for()'s
    switch.

    Host vs. target, and why "host detection" here means a compile-time
    macro check, not a runtime CPUID probe: a compiler (or, here, a
    code-generation library playing that role) always targets *some*
    machine, chosen independently of whatever machine happens to be
    running the compiler itself — that is precisely what makes
    cross-compilation ("build on x86-64, emit AArch64") a meaningful,
    supported thing for a real compiler to do, via an explicit `-target`/
    `--march` flag. The *default*, absent such a flag, is universally "the
    same architecture this compiler binary itself runs on" — not because
    that is the only sensible target, but because it is the only default
    a compiler can pick with no other information at all, and it is by far
    the common case (`gcc`/`clang` with no `-target` emit code for the
    host they run on). host_architecture() below answers exactly that
    "what does *this build* target by default" question, and it can only
    be answered at compile time: it inspects the predefined macros the
    compiler invoked *for this translation unit* already defines
    (`__x86_64__`/`_M_X64`, `__aarch64__`/`_M_ARM64`, `__riscv` combined
    with `__riscv_xlen == 64`), because those macros are exactly what
    identifies which machine code the compiler is about to emit for this
    very file — nothing at *runtime* could tell a program anything more
    relevant than what it was already told at compile time. A runtime
    CPUID probe answers a different, unrelated question ("what does the
    processor this program happens to be executing on support *right
    now*", relevant to runtime dispatch between multiple precompiled code
    paths) — not "what architecture should this code generator emit code
    for by default", which is a build-time decision, not a run-time one.
    Overriding the default (cross-codegen: emit AArch64 while running on
    an x86-64 host) is an explicit, opt-in call to
    `target_info_for(Architecture::AARCH64)` instead of
    `target_info_for(host_architecture())` — exactly the `-target`-flag
    shape a real compiler offers, and exactly why `host_architecture()`
    and `target_info_for()` are two separate calls rather than one: a
    caller who wants the host's own architecture asks for it explicitly;
    a caller cross-compiling never calls host_architecture() at all.
    @ingroup Compilers
*/

#pragma once

#include <stdexcept>
#include <string>

#include <types.hpp>
#include <array.hpp>

namespace Designar
{
    /** The real, physical instruction set architectures this project's
        code generation aims at (see this file's own comment above for
        why "real ISAs, not a toy one" was a deliberate choice made before
        any of this code was written). Spelled the way this codebase
        spells other closed, small enumerations (Grammar::Associativity,
        ParserKind in parser.hpp): plain enum-class values, no trailing
        underscores, no "ARCHITECTURE_" prefix repetition. */
    enum class Architecture
    {
        X86_64,
        AARCH64,
        RISCV64
    };

    /** Human-readable name for `a`, used in diagnostics (an
        "unimplemented target" exception message needs to name the
        architecture it is complaining about) and in demo/test output. */
    inline std::string to_string(Architecture a)
    {
        switch (a)
        {
        case Architecture::X86_64:
            return "x86_64";
        case Architecture::AARCH64:
            return "aarch64";
        case Architecture::RISCV64:
            return "riscv64";
        }

        throw std::logic_error("to_string(Architecture): unhandled value");
    }

    /** Reports which `Architecture` *this translation unit* was compiled
        for, purely from the predefined macros the host compiler already
        defines while compiling it — see the file comment above for why
        that is the right question to ask here, and why it is answered at
        compile time rather than by probing the running processor.
        `constexpr` (not just a plain function) because the answer is a
        property of the compilation itself, known before any code runs at
        all — the same reason `QuicksortThreshold` in types.hpp is a
        `constexpr`, not a runtime-computed value.

        A compiler/architecture combination this file has no macro check
        for at all fails to *compile* (via `#error` below) rather than
        silently guessing or returning some default — an unrecognized
        host is exactly the situation where guessing would be actively
        dangerous (emitting code for the wrong architecture by default,
        with no diagnostic at all, is far worse than a build failure that
        tells a contributor precisely what to add and where). */
    constexpr Architecture host_architecture() noexcept
    {
#if defined(__x86_64__) || defined(_M_X64)
        return Architecture::X86_64;
#elif defined(__aarch64__) || defined(_M_ARM64)
        return Architecture::AARCH64;
#elif defined(__riscv) && (__riscv_xlen == 64)
        return Architecture::RISCV64;
#else
#error "Designar target.hpp: host_architecture() has no macro-based " \
       "mapping for whatever compiler/architecture combination is " \
       "compiling this translation unit. This project's code generation " \
       "only knows about Architecture::X86_64/AARCH64/RISCV64 so far " \
       "(see target.hpp's file comment); add a new predefined-macro " \
       "check here (and, eventually, a TargetInfo and a codegen backend) " \
       "before targeting a fourth architecture."
#endif
    }

    /** The backend-agnostic facts a code generator needs about whichever
        concrete architecture it is asked to target — the generic seam
        every per-architecture backend (only x86_64codegen.hpp exists so
        far) is written against, so that adding an AArch64 or RISC-V
        backend later means filling in one more of these plus a new
        lowering file, not redesigning this struct's shape.

        Deliberately plain data, not a polymorphic interface: nothing
        here needs virtual dispatch (a `TargetInfo` is a fully-described,
        static snapshot of one architecture's ABI-relevant facts, not a
        live object a backend calls methods on), and a plain aggregate is
        exactly this library's existing precedent for this kind of "bag
        of per-flavor facts a shared algorithm consults" role (compare
        LRProduction's `lhs`/`rhs` pair in lrparser.hpp, or `TargetInfo`'s
        own near-neighbor `LRConflict`).

        Every register name below is architecture-specific ABI text
        (`"rdi"`, `"x0"`, `"a0"`, ...), stored as `std::string` rather
        than some architecture-neutral enum, precisely because there is
        no architecture-neutral register numbering that would mean
        anything: a backend's own lowering code is the only thing that
        ever interprets these strings, so they can be exactly the
        mnemonics that backend already knows how to emit into assembly
        text, with zero translation layer in between. */
    struct TargetInfo
    {
        Architecture architecture;

        /** Size, in bytes, of a pointer/machine word on this target —
            8 for every 64-bit architecture this file names today; kept
            as an explicit field (not hardcoded "8" at every use site)
            since it is the one number a 32-bit target would need to
            change, and memorylayout.hpp's slot-size computation reads it
            from here rather than assuming 8. */
        nat_t pointer_size_bytes;

        /** The alignment, in bytes, the stack pointer must have at the
            point of a `call` instruction on this target — 16 for the
            x86-64 System V ABI (the ABI this project actually
            implements; see x86_64codegen.hpp), also 16 for AArch64's
            AAPCS64 and RISC-V's calling convention, though this project
            does not implement either yet. Deliberately a per-target
            field rather than a hardcoded constant anywhere alignment is
            computed (see memorylayout.hpp's compute_frame_layout()),
            since a future target with a different rule only has to set
            this field correctly, not touch the layout algorithm
            itself. */
        nat_t stack_alignment_bytes;

        /** Integer/pointer argument registers, in the exact order this
            target's calling convention assigns them to a callee's first,
            second, third, ... integer/pointer parameter — `{"rdi", "rsi",
            "rdx", "rcx", "r8", "r9"}` for x86-64 System V. A parameter
            beyond this list's length is passed on the stack by every
            real calling convention this project knows about; this
            project's baseline x86-64 backend does not implement that
            overflow case yet (see x86_64codegen.hpp), a deliberately
            scoped-down limitation documented there, not here. */
        DynArray<std::string> integer_argument_registers;

        /** The register an integer/pointer return value comes back in —
            `"rax"` for x86-64 System V. */
        std::string integer_return_register;

        /** Registers a callee must save and restore if it uses them at
            all (the caller is entitled to assume they still hold
            whatever they held before the call) — `{"rbx", "r12", "r13",
            "r14", "r15", "rbp"}` for x86-64 System V. This project's
            baseline register allocator (see x86_64codegen.hpp) never
            actually allocates a value to one of these across a call
            boundary in a way that would require saving them (its "spill
            everything to its own stack slot" strategy keeps every value
            live in memory, not in a callee-saved register, precisely
            because that sidesteps needing to emit any save/restore code
            for this list at all) — the field is still recorded here,
            for a future smarter allocator that *does* want to keep a
            long-lived value in a callee-saved register across a call. */
        DynArray<std::string> callee_saved_registers;

        /** Registers free for a backend to use as short-lived scratch
            space without saving/restoring them (caller-saved registers
            not already spoken for by `integer_argument_registers`/
            `integer_return_register`) — what this project's baseline
            "load into a scratch register only around each instruction"
            allocator (see x86_64codegen.hpp) actually uses. */
        DynArray<std::string> scratch_registers;
    };

    /** The one real implementation in this file: System V x86-64's
        integer-register ABI facts, exactly what x86_64codegen.hpp's
        lowering and memorylayout.hpp's frame-size rounding consult. */
    inline TargetInfo x86_64_sysv_target_info()
    {
        TargetInfo t;
        t.architecture = Architecture::X86_64;
        t.pointer_size_bytes = 8;
        t.stack_alignment_bytes = 16;
        t.integer_argument_registers =
            DynArray<std::string>({"rdi", "rsi", "rdx", "rcx", "r8", "r9"});
        t.integer_return_register = "rax";
        t.callee_saved_registers =
            DynArray<std::string>({"rbx", "r12", "r13", "r14", "r15", "rbp"});
        /** r10/r11 only: every other caller-saved register is already
            spoken for above by argument passing or the return value,
            and x86_64codegen.hpp's baseline lowering needs at most two
            scratch registers live at once (one per operand of a binary
            instruction) plus `rax` itself for results/`idiv`'s dividend,
            which is already available since it is the return register,
            not a third scratch slot. */
        t.scratch_registers = DynArray<std::string>({"r10", "r11"});
        return t;
    }

    /** Looks up the `TargetInfo` for `a` — the one function every caller
        (memorylayout.hpp, x86_64codegen.hpp, demo-ir.cpp) actually calls,
        whether `a` came from `host_architecture()` (the default, "target
        what this build itself runs on" case) or was supplied explicitly
        by a caller doing cross-codegen ("emit for a different
        architecture than the one running this code"). Throws for
        `AARCH64`/`RISCV64` rather than returning a half-filled
        `TargetInfo`: this project's honest, current state is that only
        x86-64 has a real backend behind it (see this file's own comment
        and x86_64codegen.hpp), and a `TargetInfo` with plausible-looking
        but made-up register names would silently misrepresent that,
        exactly the kind of overclaiming the project's docs are required
        to avoid. */
    inline TargetInfo target_info_for(Architecture a)
    {
        switch (a)
        {
        case Architecture::X86_64:
            return x86_64_sysv_target_info();
        case Architecture::AARCH64:
            throw std::logic_error(
                "target_info_for(Architecture::AARCH64): no AArch64 "
                "backend exists yet — this is deliberately deferred "
                "future work (see target.hpp's file comment); only "
                "Architecture::X86_64 is implemented so far.");
        case Architecture::RISCV64:
            throw std::logic_error(
                "target_info_for(Architecture::RISCV64): no RISC-V "
                "backend exists yet — this is deliberately deferred "
                "future work (see target.hpp's file comment); only "
                "Architecture::X86_64 is implemented so far.");
        }

        throw std::logic_error("target_info_for: unhandled Architecture value");
    }

} // end namespace Designar
