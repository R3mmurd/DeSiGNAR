/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

/** @file target.hpp
    @brief The target-parametrized ABI facts memorylayout.hpp's frame-layout
    computation needs: `Architecture`, `host_architecture()`, and
    `TargetInfo`.

    Code generation — turning this project's IR (ir.hpp) into runnable
    machine code for some real architecture — is a distinct concern from
    memory layout, and is not currently planned as part of this codebase:
    no instruction-selecting backend exists here, for any architecture, and
    this file makes no attempt to provide what one would need (register
    names, calling-convention lowering rules, and so on). What
    compute_frame_layout() (memorylayout.hpp) actually needs from a target
    is narrow — how many bytes a pointer/machine word occupies, and what
    alignment the stack must have at its final size — and `TargetInfo`
    below describes exactly that and nothing more. If a code generator is
    ever written against this codebase, it would need a substantially
    richer per-architecture description than `TargetInfo` provides (registers,
    argument-passing rules, callee-saved sets, ...); deliberately, none of
    that is attempted here.

    Host vs. target, and why "host detection" here means a compile-time
    macro check, not a runtime CPUID probe: a program's build can always be
    read as targeting *some* machine, chosen independently of whatever
    machine happens to be running the toolchain itself — that is precisely
    what makes cross-compilation ("build on x86-64, target AArch64") a
    meaningful, supported thing to reason about, via an explicit `-target`/
    `--march` flag in a real compiler. The *default*, absent such a flag, is
    universally "the same architecture this toolchain binary itself runs
    on" — not because that is the only sensible target, but because it is
    the only default a build can pick with no other information at all, and
    it is by far the common case (`gcc`/`clang` with no `-target` emit code
    for the host they run on). host_architecture() below answers exactly
    that "what does *this build* target by default" question, and it can
    only be answered at compile time: it inspects the predefined macros the
    compiler invoked *for this translation unit* already defines
    (`__x86_64__`/`_M_X64`, `__aarch64__`/`_M_ARM64`, `__riscv` combined
    with `__riscv_xlen == 64`), because those macros are exactly what
    identifies which machine code the compiler is about to emit for this
    very file — nothing at *runtime* could tell a program anything more
    relevant than what it was already told at compile time. A runtime
    CPUID probe answers a different, unrelated question ("what does the
    processor this program happens to be executing on support *right
    now*", relevant to runtime dispatch between multiple precompiled code
    paths) — not "what architecture is this build's default target", which
    is a build-time fact, not a run-time one. Asking for a *different*
    architecture's `TargetInfo` than the host's own (e.g. to see how the
    same IRFunction would lay out its frame on AArch64 while running this
    on an x86-64 host, exactly what demo-ir.cpp does) is an explicit call
    to `target_info_for(Architecture::AARCH64)` instead of
    `target_info_for(host_architecture())` — exactly why `host_architecture()`
    and `target_info_for()` are two separate calls rather than one: a
    caller who wants the host's own architecture asks for it explicitly; a
    caller wanting some other architecture's ABI facts never calls
    host_architecture() at all.
    @ingroup Compilers
*/

#pragma once

#include <stdexcept>
#include <string>

#include <types.hpp>

namespace Designar
{
    /** The real, physical instruction set architectures this file
        describes ABI facts for. Spelled the way this codebase spells
        other closed, small enumerations (Grammar::Associativity,
        ParserKind in parser.hpp): plain enum-class values, no trailing
        underscores, no "ARCHITECTURE_" prefix repetition. */
    enum class Architecture
    {
        X86_64,
        AARCH64,
        RISCV64
    };

    /** Human-readable name for `a`, used in diagnostics and in demo/test
        output. */
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
        dangerous (silently reporting the wrong architecture, with no
        diagnostic at all, is far worse than a build failure that tells a
        contributor precisely what to add and where). */
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
       "compiling this translation unit. This file only knows about " \
       "Architecture::X86_64/AARCH64/RISCV64 so far (see target.hpp's " \
       "file comment); add a new predefined-macro check here before " \
       "targeting a fourth architecture."
#endif
    }

    /** The target-parametrized ABI facts memorylayout.hpp's
        compute_frame_layout() actually needs about whichever architecture
        it is laying out a frame for: how wide a pointer/machine word is,
        and what alignment the stack must have at its final size.

        This is deliberately narrower than a real "target info for code
        generation" would need to be — there is no code generator in this
        codebase to serve, so nothing here describes registers, argument-
        passing rules, or any other calling-convention lowering detail;
        adding those would be overclaiming what this file actually
        supports. If a code generator is ever added to this project later,
        it would need a materially richer per-architecture description
        than this struct, built at that time.

        Deliberately plain data, not a polymorphic interface: nothing here
        needs virtual dispatch (a `TargetInfo` is a fully-described, static
        snapshot of a couple of ABI facts, not a live object anything calls
        methods on), and a plain aggregate is exactly this library's
        existing precedent for this kind of "bag of per-flavor facts a
        shared algorithm consults" role (compare LRProduction's `lhs`/`rhs`
        pair in lrparser.hpp). */
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
            x86-64 System V ABI, also 16 for AArch64's AAPCS64 and for
            RISC-V's own calling convention (all three are genuine,
            citable ABI facts, independent of whether any code generator
            for the architecture exists in this codebase). Deliberately a
            per-target field rather than a hardcoded constant anywhere
            alignment is computed (see memorylayout.hpp's
            compute_frame_layout()), since a future target with a
            different rule only has to set this field correctly, not
            touch the layout algorithm itself. */
        nat_t stack_alignment_bytes;
    };

    /** System V x86-64 ABI facts: 8-byte pointers, 16-byte call-site stack
        alignment. */
    inline TargetInfo x86_64_sysv_target_info()
    {
        TargetInfo t;
        t.architecture = Architecture::X86_64;
        t.pointer_size_bytes = 8;
        t.stack_alignment_bytes = 16;
        return t;
    }

    /** AArch64 AAPCS64 ABI facts: 8-byte pointers, 16-byte call-site stack
        alignment. Only these two fields are filled in — see this file's
        own comment on why `TargetInfo` deliberately stops there; no
        AArch64 code generator exists in this codebase to need anything
        more. */
    inline TargetInfo aarch64_target_info()
    {
        TargetInfo t;
        t.architecture = Architecture::AARCH64;
        t.pointer_size_bytes = 8;
        t.stack_alignment_bytes = 16;
        return t;
    }

    /** RISC-V (RV64) calling-convention ABI facts: 8-byte pointers,
        16-byte call-site stack alignment. Only these two fields are
        filled in — see this file's own comment on why `TargetInfo`
        deliberately stops there; no RISC-V code generator exists in this
        codebase to need anything more. */
    inline TargetInfo riscv64_target_info()
    {
        TargetInfo t;
        t.architecture = Architecture::RISCV64;
        t.pointer_size_bytes = 8;
        t.stack_alignment_bytes = 16;
        return t;
    }

    /** Looks up the `TargetInfo` for `a` — the one function memorylayout.
        hpp/demo-ir.cpp actually call, whether `a` came from
        host_architecture() (the default, "describe what this build itself
        runs on" case) or was supplied explicitly by a caller that wants to
        see the same IRFunction laid out for a different architecture
        (exactly what demo-ir.cpp does, side by side, to make the
        target-parametrization concrete). Returns a real, complete
        `TargetInfo` for all three `Architecture` values — unlike a
        hypothetical "target info for code generation", the two ABI facts
        this struct actually claims to describe (pointer size, stack
        alignment) are equally real and citable for x86-64, AArch64, and
        RISC-V, so there is nothing to defer here. */
    inline TargetInfo target_info_for(Architecture a)
    {
        switch (a)
        {
        case Architecture::X86_64:
            return x86_64_sysv_target_info();
        case Architecture::AARCH64:
            return aarch64_target_info();
        case Architecture::RISCV64:
            return riscv64_target_info();
        }

        throw std::logic_error("target_info_for: unhandled Architecture value");
    }

} // end namespace Designar
