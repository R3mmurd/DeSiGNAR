/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <cassert>
#include <cmath>
#include <stdexcept>

#include <ir.hpp>

using namespace Designar;

namespace
{
    bool close(real_t a, real_t b, real_t eps = 1e-9)
    {
        return std::abs(a - b) < eps;
    }
}

int main()
{
    // IRValue factories/queries.
    {
        assert(IRValue::none().is_none());
        assert(!IRValue::virtual_register(3).is_none());
        assert(IRValue::virtual_register(3).kind == IRValueKind::VIRTUAL_REGISTER);
        assert(IRValue::virtual_register(3).register_id == 3);

        IRValue c = IRValue::int_constant(42);
        assert(c.is_constant());
        assert(c.int_value == 42);
        assert(c.to_string() == "42");

        IRValue r = IRValue::real_constant(1.5);
        assert(r.is_constant());
        assert(close(r.real_value, 1.5));

        IRValue l = IRValue::local("x");
        assert(!l.is_constant());
        assert(l.kind == IRValueKind::LOCAL);
        assert(l.to_string() == "@x");
    }

    // IRBuilder: emitting arithmetic allocates fresh, ever-increasing
    // virtual registers and appends exactly the instruction asked for.
    {
        IRFunction function("f", IRType::INT);
        function.declare_local("a", IRType::INT, /*is_parameter=*/true);
        function.declare_local("b", IRType::INT, /*is_parameter=*/true);

        IRBuilder builder(function);
        builder.create_block("entry");

        IRValue a = IRValue::local("a");
        IRValue b = IRValue::local("b");

        IRValue sum = builder.emit_add(IRType::INT, a, b);
        assert(sum.kind == IRValueKind::VIRTUAL_REGISTER);
        assert(sum.register_id == 0);

        IRValue diff = builder.emit_sub(IRType::INT, a, b);
        assert(diff.register_id == 1);

        builder.emit_ret(sum);

        assert(function.blocks.size() == 1);
        const IRBasicBlock& block = function.blocks[0];
        assert(block.instructions.size() == 3);
        assert(block.instructions[0].opcode == IROpcode::ADD);
        assert(block.instructions[0].dest.register_id == 0);
        assert(block.instructions[1].opcode == IROpcode::SUB);
        assert(block.instructions[2].opcode == IROpcode::RET);
        assert(block.instructions[2].is_terminator());
        assert(!block.instructions[0].is_terminator());
        assert(block.has_terminator());
    }

    // IRBuilder refuses to append past a terminator, and refuses a
    // duplicate block label / a duplicate local declaration.
    {
        IRFunction function("g", IRType::INT);
        IRBuilder builder(function);
        builder.create_block("entry");
        builder.emit_ret(IRValue::int_constant(0));

        bool threw = false;

        try
        {
            builder.emit_ret(IRValue::int_constant(1));
        }
        catch (const std::logic_error&)
        {
            threw = true;
        }

        assert(threw);

        threw = false;

        try
        {
            function.create_block("entry");
        }
        catch (const std::invalid_argument&)
        {
            threw = true;
        }

        assert(threw);

        function.declare_local("x", IRType::INT);
        threw = false;

        try
        {
            function.declare_local("x", IRType::REAL);
        }
        catch (const std::invalid_argument&)
        {
            threw = true;
        }

        assert(threw);
    }

    // ADDR_OF only accepts a LOCAL operand.
    {
        IRFunction function("h", IRType::INT);
        IRBuilder builder(function);
        builder.create_block("entry");

        bool threw = false;

        try
        {
            builder.emit_addr_of(IRValue::virtual_register(0));
        }
        catch (const std::invalid_argument&)
        {
            threw = true;
        }

        assert(threw);
    }

    // constant_fold(): folds an instruction whose two operands are both
    // already-known constants into a MOVE of the computed result, and
    // reports whether anything changed.
    {
        IRFunction function("answer", IRType::INT);
        IRBuilder builder(function);
        builder.create_block("entry");

        IRValue folded_sum =
            builder.emit_add(IRType::INT, IRValue::int_constant(2),
                            IRValue::int_constant(3));
        IRValue folded_cmp =
            builder.emit_cmp_lt(IRType::INT, IRValue::int_constant(2),
                               IRValue::int_constant(3));
        // Not foldable yet: one operand is a virtual register, not a
        // constant, even though it happens to hold a folded value —
        // constant_fold() folds one instruction's own two operands, it
        // does not propagate an already-folded value into later
        // instructions (see ir.hpp's own comment on this distinction).
        IRValue not_yet_foldable =
            builder.emit_add(IRType::INT, folded_sum, IRValue::int_constant(1));
        builder.emit_ret(not_yet_foldable);

        bool changed = constant_fold(function);
        assert(changed);

        const IRBasicBlock& block = function.blocks[0];
        assert(block.instructions[0].opcode == IROpcode::MOVE);
        assert(block.instructions[0].src1.kind == IRValueKind::INT_CONSTANT);
        assert(block.instructions[0].src1.int_value == 5);

        assert(block.instructions[1].opcode == IROpcode::MOVE);
        assert(block.instructions[1].src1.int_value == 1);

        assert(block.instructions[2].opcode == IROpcode::ADD);
        assert(block.instructions[2].src1.kind == IRValueKind::VIRTUAL_REGISTER);

        // A second run finds nothing left to fold.
        bool changed_again = constant_fold(function);
        assert(!changed_again);
    }

    // constant_fold() never folds a division/modulo by a constant zero
    // — that is undefined behavior for int_t at compile time, and any
    // future consumer lowering/executing this IR must still reach the
    // real division-by-zero fault/trap, so folding it away would
    // silently change the described program's own runtime behavior
    // (see ir.hpp's own comment on this).
    {
        IRFunction function("div_by_zero", IRType::INT);
        IRBuilder builder(function);
        builder.create_block("entry");
        IRValue result = builder.emit_div(IRType::INT, IRValue::int_constant(10),
                                         IRValue::int_constant(0));
        builder.emit_ret(result);

        bool changed = constant_fold(function);
        assert(!changed);
        assert(function.blocks[0].instructions[0].opcode == IROpcode::DIV);
    }

    // Real (floating-point) constant folding.
    {
        IRFunction function("real_answer", IRType::REAL);
        IRBuilder builder(function);
        builder.create_block("entry");
        IRValue product = builder.emit_mul(IRType::REAL, IRValue::real_constant(2.5),
                                          IRValue::real_constant(4.0));
        builder.emit_ret(product);

        bool changed = constant_fold(function);
        assert(changed);
        assert(function.blocks[0].instructions[0].opcode == IROpcode::MOVE);
        assert(function.blocks[0].instructions[0].src1.kind ==
              IRValueKind::REAL_CONSTANT);
        assert(close(function.blocks[0].instructions[0].src1.real_value, 10.0));
    }

    // IRModule owns its functions by value; find_function() locates one
    // by name.
    {
        IRModule module("m");
        module.create_function("one", IRType::INT);
        module.create_function("two", IRType::INT);

        assert(module.functions.size() == 2);
        assert(module.find_function("two") != nullptr);
        assert(module.find_function("two")->name == "two");
        assert(module.find_function("missing") == nullptr);
    }

    return 0;
}
