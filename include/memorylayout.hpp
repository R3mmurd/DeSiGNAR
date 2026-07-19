/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

/** @file memorylayout.hpp
    @brief compute_frame_layout(): maps an IRFunction's declared locals/
    parameters (ir.hpp) to concrete stack-frame storage, parametrized by
    a TargetInfo (target.hpp) — the "memory organization" piece of this
    project's larger IR/optimization/codegen initiative (see ir.hpp's
    file comment for the full phasing story).

    Split out of ir.hpp into its own file, rather than folded into
    IRFunction itself, for the same reason target.hpp is its own file
    rather than a few structs sitting inside x86_64codegen.hpp: this is
    substantial enough — and, more importantly, *architecture-dependent*
    enough (x86-64 System V's 16-byte call-site alignment is not a
    universal constant; AArch64/RISC-V have their own rules) — to deserve
    being read and reasoned about on its own, independent of any one
    backend's instruction-selection concerns.

    Why frame-slot assignment is a function of a *target*, not a fixed
    algorithm run once: computing "where does each local live, relative
    to a frame base" sounds architecture-neutral (it is just arithmetic
    on sizes and offsets), but the one number that arithmetic has to
    respect — the final frame size's required alignment — is an ABI fact
    that differs per architecture (16 bytes for x86-64 System V and for
    AArch64's AAPCS64, a different rule for RISC-V's own calling
    convention). Hardcoding "round up to 16" here would silently bake in
    an x86-64-only assumption into a file that is supposed to be the
    reusable, target-agnostic half of memory organization — exactly the
    mistake this file's own compute_frame_layout() signature (taking a
    `const TargetInfo&` explicitly) is written to make impossible to make
    by accident.
    @ingroup Compilers
*/

#pragma once

#include <stdexcept>
#include <string>

#include <types.hpp>
#include <map.hpp>
#include <ir.hpp>
#include <target.hpp>

namespace Designar
{
    /** Where one local/parameter actually lives once compute_frame_layout()
        has run: its byte size, and its offset from the frame base.

        `frame_offset` is signed (`int_t`, not `nat_t`) and, for every
        target this project actually implements (x86-64 System V, via
        x86_64codegen.hpp), is *negative* — the standard "locals live
        below the saved frame pointer" convention a `push rbp; mov rbp,
        rsp; sub rsp, N` prologue sets up, where a local's address is
        `rbp + frame_offset` with `frame_offset < 0`. This struct itself
        does not hardcode that sign convention as a rule (a hypothetical
        target that grows its frame the other way could produce positive
        offsets from this same struct), but every offset
        compute_frame_layout() actually produces today follows it.

        `in_register` is always `false` coming out of this pass's
        compute_frame_layout() — see its own comment — but the field
        exists (rather than being omitted until a real register
        allocator exists) so that a future smarter allocator can extend
        this same struct instead of needing an incompatible new one. */
    struct LocalSlot
    {
        std::string name;
        nat_t size_bytes = 0;
        int_t frame_offset = 0;

        /** Always `false` for now — see compute_frame_layout()'s own
            comment on why this pass's baseline never sets it. A local
            with `in_register == true` would have `frame_offset`/
            `size_bytes` be meaningless (nothing about "where in memory"
            applies to a value that never leaves a register at all); no
            code in this pass ever produces that combination, but a
            future allocator consuming this struct needs to know it is
            a possibility to check for. */
        bool in_register = false;
    };

    /** The result of laying out one IRFunction's locals for one target:
        every local's LocalSlot, keyed by name (a HashMap for the same
        "looked up by name, potentially often, from codegen" reason
        IRFunction::locals itself is one — see ir.hpp), plus the frame's
        total size in bytes, already rounded up to satisfy
        `target.stack_alignment_bytes` at the point a prologue's `sub
        rsp, frame_size_bytes` executes (see compute_frame_layout()'s own
        comment for the exact arithmetic and why it produces a correctly
        call-site-aligned stack for x86-64 System V specifically). */
    class FrameLayout
    {
    public:
        HashMap<std::string, LocalSlot> slots;
        nat_t frame_size_bytes = 0;

        const LocalSlot* slot_for(const std::string& name) const
        {
            return slots.search(name);
        }
    };

    /** Assigns every local/parameter IRFunction::locals declares a stack
        slot, for `target`'s ABI.

        Deliberately assigns *every* local a real memory slot,
        register-candidate or not: this project's only implemented
        backend (x86_64codegen.hpp) uses a deliberately baseline register
        allocator ("spill everything to its own stack slot, reload into a
        scratch register only around each instruction that touches it" —
        see that file's own comment on why, and on graph-coloring/linear-
        scan allocation being the documented, deferred next step), so
        *nothing* actually lives only in a register in this pass's
        generated code regardless of `IRLocal::address_taken`. This
        function still records the flag faithfully on the way in — a
        future smarter allocator, built without changing this function's
        signature or FrameLayout's shape, is exactly what would start
        actually honoring `!address_taken` by putting some locals in
        `LocalSlot::in_register == true` slots instead of memory. What
        can never change, for *any* future allocator, is that an
        `address_taken` local must always get a real memory slot: taking
        its address (IROpcode::ADDR_OF) requires handing back a real,
        loadable/storable runtime address, and a register has no address
        at all — there is no `lea`-equivalent that produces "the address
        of whatever value currently sits in this register", because a
        register is not addressable memory in the first place. So
        `address_taken` is a hard constraint every future allocator must
        keep respecting, not merely this pass's own default.

        Every declared local (parameter or not) gets `target.
        pointer_size_bytes` (8, for x86-64) worth of stack space
        regardless of whether it is `IRType::INT` or `IRType::REAL`: both
        `int_t` and `real_t` (types.hpp) are 8 bytes on every target this
        project's TargetInfo actually describes, so a single fixed slot
        size keeps this function simple without yet needing a per-IRType
        size table — a real concern only once a target with a genuinely
        different word size shows up, which is explicitly out of scope
        here (see target.hpp: only x86-64 is implemented).

        Slots are laid out in `IRFunction::locals`' HashMap iteration
        order, each one frame_offset = -(running_total) after adding its
        own size, i.e. the first local processed ends up at the frame's
        lowest (most negative) offset — no particular local ordering is
        promised or relied upon by any of this pass's own code (nothing
        needs one: FrameLayout is a name -> slot map, and offsets are
        looked up by name, not iterated positionally); a future pass
        that *does* want a specific ordering (e.g. to keep related
        fields adjacent for cache locality) is free to reimplement this
        function's loop, since nothing about FrameLayout's own shape
        assumes any particular ordering either. */
    inline FrameLayout compute_frame_layout(const IRFunction& function,
                                           const TargetInfo& target)
    {
        FrameLayout layout;
        nat_t running_total = 0;

        for (const auto& kv : function.locals)
        {
            const IRLocal& local = kv.second;

            nat_t size = target.pointer_size_bytes;
            running_total += size;

            LocalSlot slot;
            slot.name = local.name;
            slot.size_bytes = size;
            slot.frame_offset = -static_cast<int_t>(running_total);
            slot.in_register = false;

            layout.slots.insert(local.name, slot);
        }

        if (target.stack_alignment_bytes == 0)
        {
            throw std::invalid_argument(
                "compute_frame_layout: target.stack_alignment_bytes must "
                "not be zero");
        }

        nat_t remainder = running_total % target.stack_alignment_bytes;
        nat_t aligned_total =
            remainder == 0 ? running_total
                           : running_total + (target.stack_alignment_bytes -
                                              remainder);

        layout.frame_size_bytes = aligned_total;
        return layout;
    }

} // end namespace Designar
