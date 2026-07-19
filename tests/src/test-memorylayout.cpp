/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <cassert>

#include <ir.hpp>
#include <memorylayout.hpp>
#include <target.hpp>

using namespace Designar;

namespace
{
    /** A function with two ordinary locals and one `address_taken` local
        — used below to confirm compute_frame_layout()'s own documented
        invariant that *every* local gets a real memory slot regardless
        of that flag (see memorylayout.hpp's compute_frame_layout()
        comment: there is no register allocator in this codebase for
        `address_taken == false` to matter to yet). */
    IRFunction build_mixed_locals_function()
    {
        IRFunction function("mixed", IRType::INT);
        function.declare_local("a", IRType::INT, /*is_parameter=*/true,
                              /*address_taken=*/false);
        function.declare_local("b", IRType::INT, /*is_parameter=*/false,
                              /*address_taken=*/false);
        function.declare_local("c", IRType::INT, /*is_parameter=*/false,
                              /*address_taken=*/true);
        return function;
    }
}

int main()
{
    // Every local — ordinary or address_taken — gets a real memory slot,
    // and slot_for() finds each one by name.
    {
        IRFunction function = build_mixed_locals_function();
        TargetInfo target = x86_64_sysv_target_info();

        FrameLayout layout = compute_frame_layout(function, target);

        const LocalSlot* a = layout.slot_for("a");
        const LocalSlot* b = layout.slot_for("b");
        const LocalSlot* c = layout.slot_for("c");

        assert(a != nullptr);
        assert(b != nullptr);
        assert(c != nullptr);

        assert(a->size_bytes == target.pointer_size_bytes);
        assert(b->size_bytes == target.pointer_size_bytes);
        assert(c->size_bytes == target.pointer_size_bytes);

        assert(!a->in_register);
        assert(!b->in_register);
        assert(!c->in_register);

        // Every slot's frame_offset is negative — the "locals live below
        // the frame base" convention this function's own arithmetic
        // follows (see memorylayout.hpp's LocalSlot comment).
        assert(a->frame_offset < 0);
        assert(b->frame_offset < 0);
        assert(c->frame_offset < 0);

        assert(layout.slot_for("missing") == nullptr);
    }

    // frame_size_bytes rounds up to target.stack_alignment_bytes when the
    // raw total is not already a multiple of it: three 8-byte locals is
    // 24 raw bytes, which is not a multiple of 16, so the aligned total
    // must be 32.
    {
        IRFunction function = build_mixed_locals_function();
        TargetInfo target = x86_64_sysv_target_info();

        FrameLayout layout = compute_frame_layout(function, target);

        assert(target.pointer_size_bytes == 8);
        assert(target.stack_alignment_bytes == 16);
        assert(layout.frame_size_bytes == 32);
    }

    // frame_size_bytes is left alone when the raw total is already a
    // multiple of target.stack_alignment_bytes: two 8-byte locals is 16
    // raw bytes, already a multiple of 16, so no extra padding is added.
    {
        IRFunction function("two_locals", IRType::INT);
        function.declare_local("a", IRType::INT);
        function.declare_local("b", IRType::INT);

        TargetInfo target = x86_64_sysv_target_info();
        FrameLayout layout = compute_frame_layout(function, target);

        assert(layout.frame_size_bytes == 16);
    }

    // Laying out the same IRFunction against two TargetInfos that differ
    // only in stack_alignment_bytes produces two different
    // frame_size_bytes results — the target-parametrization
    // compute_frame_layout()'s signature exists to make possible.
    {
        IRFunction function = build_mixed_locals_function();

        // Three 8-byte-pointer locals is 24 raw bytes, regardless of
        // target. A 16-byte alignment (real x86-64 System V) rounds
        // that up to 32; a synthetic, unusually large 48-byte alignment
        // (not one any real architecture target.hpp names — constructed
        // here purely to exercise the rounding arithmetic against a
        // second, clearly distinct alignment) rounds the same 24 raw
        // bytes up to 48 instead, so the two computed frame sizes are
        // unambiguously different.
        TargetInfo narrow_alignment_target;
        narrow_alignment_target.architecture = Architecture::X86_64;
        narrow_alignment_target.pointer_size_bytes = 8;
        narrow_alignment_target.stack_alignment_bytes = 16;

        TargetInfo wide_alignment_target;
        wide_alignment_target.architecture = Architecture::X86_64;
        wide_alignment_target.pointer_size_bytes = 8;
        wide_alignment_target.stack_alignment_bytes = 48;

        FrameLayout narrow_layout =
            compute_frame_layout(function, narrow_alignment_target);
        FrameLayout wide_layout =
            compute_frame_layout(function, wide_alignment_target);

        assert(narrow_layout.frame_size_bytes == 32);
        assert(wide_layout.frame_size_bytes == 48);
        assert(narrow_layout.frame_size_bytes != wide_layout.frame_size_bytes);
    }

    return 0;
}
