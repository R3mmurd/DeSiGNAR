/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

/** @file demo-ir.cpp
    @brief Builds a small but real IRModule (ir.hpp) by hand — a
    recursive `factorial`, a `store_and_load` function that exercises
    address-taken locals (memorylayout.hpp/ir.hpp's ADDR_OF/LOAD/STORE),
    and a `compute_answer` function built specifically to show
    constant_fold() (ir.hpp) at work — then lowers the whole module to
    real x86-64 System V assembly text via X86_64CodeGenerator
    (x86_64codegen.hpp), assembles + links + *runs* it with the host's
    own `as`/`ld` if this is an x86-64 Linux host with both available,
    and reports whether the machine code actually computed what the IR
    said it should. This is this project's own end-to-end proof of the
    Phase 1 pipeline described in ir.hpp's file comment: IR -> one
    optimization pass -> memory layout -> x86-64 codegen -> real,
    runnable machine code — not a mockup of any of those steps. */

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include <sys/wait.h>
#include <unistd.h>

using namespace std;

#include <ir.hpp>
#include <memorylayout.hpp>
#include <target.hpp>
#include <x86_64codegen.hpp>

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
        real memory (a register has no address at all), and
        x86_64codegen.hpp's ADDR_OF lowering (a real `lea`) for how that
        address actually gets computed and used by the matching
        LOAD/STORE. Expected result for `x = 9` is `10`. */
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
        a related but separate optimization this project has not built).
        Expected result either way is `25`, computed correctly at run
        time regardless of whether the addition itself was ever folded
        at compile time. */
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

    /** Runs `command` and returns true iff the shell reports it exited
        with status 0 — used both to probe for `as`/`ld` on this host
        and to check the assemble/link steps below actually succeeded,
        never to interpret the *generated program's* own exit code
        (see run_and_check_exit_code() for that, which needs the raw
        status, not a collapsed bool). */
    bool run_ok(const string& command)
    {
        return system(command.c_str()) == 0;
    }
}

int main()
{
    cout << "=== Designar IR / x86-64 codegen demo ===\n\n";

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

    X86_64CodeGenerator codegen;
    string assembly = codegen.generate_module(module);

    cout << "\n--- Generated x86-64 (Intel syntax) assembly ---\n";
    cout << assembly;

    if (host_architecture() != Architecture::X86_64)
    {
        cout << "\nThis host is not x86-64 (host_architecture() == "
             << to_string(host_architecture())
             << "), so the assembly above was only generated, not "
                "assembled or executed — running x86-64 machine code "
                "directly requires an x86-64 host. Cross-generating for "
                "x86-64 while running elsewhere is exactly what "
                "generate_module() above just did; actually *running* "
                "the result would need an x86-64 machine (or an "
                "emulator) instead of this one.\n";
        return 0;
    }

    if (!run_ok("which as > /dev/null 2>&1") ||
        !run_ok("which ld > /dev/null 2>&1"))
    {
        cout << "\n`as`/`ld` were not both found on this host, so the "
                "generated assembly above was only syntactically "
                "produced, not assembled/linked/executed. Install "
                "binutils to get the full end-to-end verification this "
                "demo is written to perform.\n";
        return 0;
    }

    /** A tiny, hand-written `_start` that calls straight into the
        machine code X86_64CodeGenerator produced above, with no libc
        and no `main`/crt0 involved at all — the generated functions'
        own calling convention (System V integer args in rdi/rsi/.../
        return in rax) is exactly what a hand-written caller needs to
        respect, and respecting it here from ordinary hand-written
        assembly is itself a small, independent check that
        x86_64codegen.hpp's lowering really did produce a
        System-V-conformant callee: `factorial`/`store_and_load` receive
        their one argument in `rdi` and return in `rax` exactly the way
        this driver calls them, with no adapter code of any kind in
        between. Exits 0 if every one of the three functions above
        returned exactly the value hand-computed for it, 1 otherwise —
        so a single process exit code is this demo's complete,
        machine-checkable verdict on whether the generated code is
        correct, not just "it assembled". */
    ostringstream driver;
    driver << assembly;
    driver << "\n";
    driver << ".global _start\n";
    driver << "_start:\n";
    driver << "    mov rdi, 5\n";
    driver << "    call factorial\n";
    driver << "    cmp rax, 120\n";
    driver << "    jne fail\n";
    driver << "    mov rdi, 9\n";
    driver << "    call store_and_load\n";
    driver << "    cmp rax, 10\n";
    driver << "    jne fail\n";
    driver << "    call compute_answer\n";
    driver << "    cmp rax, 25\n";
    driver << "    jne fail\n";
    driver << "    mov rdi, 0\n";
    driver << "    mov rax, 60\n";
    driver << "    syscall\n";
    driver << "fail:\n";
    driver << "    mov rdi, 1\n";
    driver << "    mov rax, 60\n";
    driver << "    syscall\n";

    string base = "/tmp/designar-demo-ir-" + to_string((nat_t)getpid());
    string asm_path = base + ".s";
    string obj_path = base + ".o";
    string exe_path = base + ".elf";

    ofstream asm_file(asm_path);
    asm_file << driver.str();
    asm_file.close();

    cout << "\n--- Assembling, linking, and running the generated code "
            "(no libc; `_start` calls straight into it) ---\n";

    if (!run_ok("as -o " + obj_path + " " + asm_path))
    {
        cerr << "Assembling the generated code with `as` failed — this "
                "would mean X86_64CodeGenerator produced syntactically "
                "invalid assembly, a real bug.\n";
        return 1;
    }

    if (!run_ok("ld -o " + exe_path + " " + obj_path))
    {
        cerr << "Linking the assembled object with `ld` failed.\n";
        return 1;
    }

    int status = system(exe_path.c_str());
    int exit_code = WIFEXITED(status) ? WEXITSTATUS(status) : -1;

    remove(asm_path.c_str());
    remove(obj_path.c_str());
    remove(exe_path.c_str());

    if (exit_code == 0)
    {
        cout << "\nSuccess: the generated machine code actually ran and "
                "computed factorial(5) == 120, store_and_load(9) == 10, "
                "and compute_answer() == 25, exactly as the IR says it "
                "should.\n";
    }
    else
    {
        cerr << "\nFAILED: the generated machine code exited with code "
             << exit_code
             << " instead of 0 — at least one of the three functions "
                "computed the wrong result.\n";
        return 1;
    }

    return 0;
}
