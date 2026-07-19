/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

/** @file demo-ir.cpp
    @brief Builds a small but real IRModule (ir.hpp) by hand — a
    recursive `factorial`, a `store_and_load` function that exercises
    address-taken locals (memorylayout.hpp/ir.hpp's ADDR_OF/LOAD/STORE),
    and a `compute_answer` function built specifically to show
    constant_fold() (ir.hpp) at work — prints the IR before and after
    running that one optimization pass, and then computes and prints
    each function's target-parametrized stack-frame layout
    (memorylayout.hpp's compute_frame_layout()) against two different
    architectures side by side, so the effect of target-parametrization
    on frame size/alignment is directly visible.

    This demo does no code generation, assembly, or execution of any
    kind: there is no code generator in this codebase. It is a
    demonstration of exactly three things — the IR, one optimization
    pass over it, and target-parametrized memory layout — and nothing
    more. */

#include <iostream>
#include <string>

using namespace std;

#include <ir.hpp>
#include <memorylayout.hpp>
#include <target.hpp>

using namespace Designar;

namespace
{
    /** `n <= 1 ? 1 : n * factorial(n - 1)` — deliberately recursive, so
        this one function alone exercises CJMP (the base-case test),
        CALL/RET (the recursive call and both return sites), and integer
        arithmetic (SUB/MUL) all together, over three basic blocks
        (`entry`, `base`, `recurse`). */
    void build_factorial(IRModule& module)
    {
        IRFunction& function = module.create_function("factorial", IRType::INT);
        function.declare_local("n", IRType::INT, /*is_parameter=*/true);

        IRBuilder builder(function);
        IRValue n = IRValue::local("n");

        builder.create_block("entry");
        IRValue is_base_case =
            builder.emit_cmp_le(IRType::INT, n, IRValue::int_constant(1));
        builder.emit_cjmp(is_base_case, "base", "recurse");

        builder.create_block("base");
        builder.emit_ret(IRValue::int_constant(1));

        builder.create_block("recurse");
        IRValue n_minus_1 = builder.emit_sub(IRType::INT, n, IRValue::int_constant(1));
        IRValue recursive_result = builder.emit_call(
            "factorial", DynArray<IRValue>({n_minus_1}), IRType::INT);
        IRValue product = builder.emit_mul(IRType::INT, n, recursive_result);
        builder.emit_ret(product);
    }

    /** `y = x; addr = &y; *addr = x + 1; return *addr;` — `y` is marked
        `address_taken` precisely because `emit_addr_of()` is called
        against it: see memorylayout.hpp's FrameLayout/LocalSlot comment
        for why a local an ADDR_OF is ever taken against must live in
        real memory (a register has no address at all). Expected result
        for `x = 9` is `10`, if this IR were ever executed — this demo
        never executes it, only lays it out. */
    void build_store_and_load(IRModule& module)
    {
        IRFunction& function =
            module.create_function("store_and_load", IRType::INT);
        function.declare_local("x", IRType::INT, /*is_parameter=*/true);
        function.declare_local("y", IRType::INT, /*is_parameter=*/false,
                              /*address_taken=*/true);

        IRBuilder builder(function);
        IRValue x = IRValue::local("x");
        IRValue y = IRValue::local("y");

        builder.create_block("entry");
        builder.emit_move(y, x);
        IRValue address_of_y = builder.emit_addr_of(y);
        IRValue x_plus_one = builder.emit_add(IRType::INT, x, IRValue::int_constant(1));
        builder.emit_store(address_of_y, x_plus_one);
        IRValue loaded = builder.emit_load(IRType::INT, address_of_y);
        builder.emit_ret(loaded);
    }

    /** `t0 = 6 + 7; t1 = 3 * 4; return t0 + t1;` — built specifically so
        constant_fold() has something real to do: `t0`'s and `t1`'s
        defining instructions both have two constant operands, so the
        pass folds each into a plain `MOVE` of the already-computed
        result (13 and 12). The final `t0 + t1` is deliberately *not*
        folded even after that: by the time constant_fold() looks at it,
        its operands are virtual registers (`t0`/`t1`), not constants —
        this pass folds one instruction's *own* two operands, it does
        not also propagate an already-folded value into every later
        instruction that reads it (that would be constant *propagation*,
        a related but separate optimization this project has not built). */
    void build_compute_answer(IRModule& module)
    {
        IRFunction& function =
            module.create_function("compute_answer", IRType::INT);

        IRBuilder builder(function);
        builder.create_block("entry");
        IRValue t0 = builder.emit_add(IRType::INT, IRValue::int_constant(6),
                                     IRValue::int_constant(7));
        IRValue t1 = builder.emit_mul(IRType::INT, IRValue::int_constant(3),
                                     IRValue::int_constant(4));
        IRValue t2 = builder.emit_add(IRType::INT, t0, t1);
        builder.emit_ret(t2);
    }

    /** Prints `function`'s frame layout under `target`, one LocalSlot per
        declared local plus the function's total, aligned
        `frame_size_bytes` — the concrete demonstration of
        memorylayout.hpp's compute_frame_layout() actually running. */
    void print_frame_layout(const IRFunction& function, const TargetInfo& target)
    {
        FrameLayout layout = compute_frame_layout(function, target);

        cout << "  target " << to_string(target.architecture)
             << " (pointer_size_bytes=" << target.pointer_size_bytes
             << ", stack_alignment_bytes=" << target.stack_alignment_bytes
             << "):\n";

        for (const auto& kv : function.locals)
        {
            const LocalSlot* slot = layout.slot_for(kv.second.name);
            cout << "    " << slot->name << ": size_bytes=" << slot->size_bytes
                 << ", frame_offset=" << slot->frame_offset << "\n";
        }

        cout << "    frame_size_bytes=" << layout.frame_size_bytes << "\n";
    }
}

int main()
{
    cout << "=== Designar IR / optimization / memory-layout demo ===\n\n";

    IRModule module("demo");
    build_factorial(module);
    build_store_and_load(module);
    build_compute_answer(module);

    cout << "--- IR before optimization ---\n";
    cout << module.to_string();

    bool any_changed = false;

    for (IRFunction& function : module.functions)
    {
        if (constant_fold(function))
        {
            any_changed = true;
        }
    }

    cout << "\n--- IR after constant_fold() (changed: "
         << (any_changed ? "yes" : "no") << ") ---\n";
    cout << module.to_string();

    cout << "\nHost architecture (host_architecture(), compile-time "
            "detection — see target.hpp): "
         << to_string(host_architecture()) << "\n";

    cout << "\n--- Frame layout (compute_frame_layout(), memorylayout.hpp) "
            "---\n";
    cout << "Laid out against both the host architecture and AArch64, side "
            "by side, to make target-parametrization concrete: the same "
            "IRFunction's frame size/alignment depends on which target it "
            "is laid out for, even though no code generator in this "
            "codebase consumes either layout yet.\n\n";

    TargetInfo host_target = target_info_for(host_architecture());
    TargetInfo aarch64_target = target_info_for(Architecture::AARCH64);

    for (const IRFunction& function : module.functions)
    {
        cout << "function " << function.name << ":\n";
        print_frame_layout(function, host_target);
        print_frame_layout(function, aarch64_target);
        cout << "\n";
    }

    return 0;
}
