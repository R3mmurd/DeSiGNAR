/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <iostream>

using namespace std;

#include <parser.hpp>

using namespace Designar;

namespace
{
    /** A single AST node shape shared by *both* grammars this demo
        builds — the small statement language below (via SLRParser and
        LALRParser, driven through the very same registered grammar) and
        the simpler standalone expression grammar further down (via
        LLParser). Most `Kind`s are real, permanent AST shapes; `TAG`,
        `TAIL`, and `NIL` are transient bookkeeping shapes that never
        survive past the `reduce()` call that consumes them (the same
        role demo-lrparser.cpp's `ExprNode::OP_TAG` plays there). */
    struct AstNode
    {
        enum Kind
        {
            NUMBER,    // a literal: value set
            VAR,       // a variable reference/binding: name set
            BINOP,     // a binary operation: op, left, right set
            LET_STMT,  // `let name = left ;`
            EXPR_STMT, // `left ;` (evaluated and printed)
            PROGRAM,   // a statement list: stmts set
            TAG,       // transient: an operator/punctuation/keyword
                       // leaf, freed inside whatever reduce() consumes it
            TAIL,      // transient: one link of the LL sub-grammar's
                       // right-recursive "pending operations" chain (see
                       // fold_tail() below) — op/left (the operand)/
                       // right (the rest of the chain) set
            NIL        // transient: the empty-chain end of a TAIL list
                       // (what an eps production synthesizes)
        } kind;

        real_t value = 0;
        std::string name;
        std::string op;
        AstNode* left = nullptr;
        AstNode* right = nullptr;
        DynArray<AstNode*> stmts; // PROGRAM only

        static AstNode* number(real_t v)
        {
            AstNode* n = new AstNode;
            n->kind = NUMBER;
            n->value = v;
            return n;
        }

        static AstNode* var(const std::string& name)
        {
            AstNode* n = new AstNode;
            n->kind = VAR;
            n->name = name;
            return n;
        }

        static AstNode* tag(const std::string& lexeme)
        {
            AstNode* n = new AstNode;
            n->kind = TAG;
            n->op = lexeme;
            return n;
        }

        static AstNode* binop(const std::string& op, AstNode* l, AstNode* r)
        {
            AstNode* n = new AstNode;
            n->kind = BINOP;
            n->op = op;
            n->left = l;
            n->right = r;
            return n;
        }
    };

    void destroy_ast(AstNode*& n)
    {
        if (n == nullptr)
        {
            return;
        }

        destroy_ast(n->left);
        destroy_ast(n->right);

        for (nat_t i = 0; i < n->stmts.size(); ++i)
        {
            destroy_ast(n->stmts[i]);
        }

        delete n;
        n = nullptr;
    }

    /** Collapses one of the LL sub-grammar's right-recursive "pending
        operations" chains — built by `Eprime`/`Tprime`'s own reduce()
        actions below, one TAIL link per `+ T`/`* F` seen — into real,
        left-associative BINOP nodes, exactly the fixup a hand-written
        recursive-descent parser for this same eliminated-left-recursion
        grammar has to do (compare demo-llparser.cpp's `eval_Eprime`/
        `eval_Tprime`, which do the equivalent fold by hand over a parse
        tree instead of over a builder-produced chain). `acc` starts as
        whatever came before the tail (`T` for `E -> T Eprime`, `F` for
        `T -> F Tprime`); each TAIL link is freed as it's folded in, and
        the terminating NIL sentinel is freed too. */
    AstNode* fold_tail(AstNode* acc, AstNode* tail)
    {
        while (tail->kind == AstNode::TAIL)
        {
            AstNode* combined = AstNode::binop(tail->op, acc, tail->left);
            AstNode* rest = tail->right;
            tail->left = nullptr; // ownership moved into combined
            tail->right = nullptr;
            delete tail;
            acc = combined;
            tail = rest;
        }

        delete tail; // the NIL sentinel
        return acc;
    }

    /** Evaluates an expression subtree, looking up `VAR` references in
        `env` — a small, throwing lookup (an undefined variable is a
        genuine runtime error in this toy language, not something to
        default to zero for). */
    real_t interpret_expr(AstNode* n, HashMap<std::string, real_t>& env)
    {
        switch (n->kind)
        {
        case AstNode::NUMBER:
            return n->value;

        case AstNode::VAR:
        {
            real_t* v = env.search(n->name);

            if (v == nullptr)
            {
                throw std::runtime_error("undefined variable '" + n->name +
                                         "'");
            }

            return *v;
        }

        case AstNode::BINOP:
        {
            real_t l = interpret_expr(n->left, env);
            real_t r = interpret_expr(n->right, env);

            if (n->op == "+") return l + r;
            if (n->op == "-") return l - r;
            if (n->op == "*") return l * r;
            return l / r;
        }

        default:
            throw std::logic_error(
                "interpret_expr: not an expression node");
        }
    }

    /** Runs every statement of `prog` in order against a fresh variable
        environment: `let` updates it, a bare expression statement
        prints its value — the "interpret" half of the pipeline this
        demo builds, exactly as demo-lrparser.cpp's own `interpret()`
        does for its single-expression AST. */
    void interpret_program(AstNode* prog, std::ostream& out)
    {
        HashMap<std::string, real_t> env;

        for (nat_t i = 0; i < prog->stmts.size(); ++i)
        {
            AstNode* stmt = prog->stmts[i];

            if (stmt->kind == AstNode::LET_STMT)
            {
                real_t v = interpret_expr(stmt->left, env);
                real_t* slot = env.search(stmt->name);

                if (slot != nullptr)
                {
                    *slot = v;
                }
                else
                {
                    env.insert(stmt->name, v);
                }
            }
            else
            {
                out << "=> " << interpret_expr(stmt->left, env) << "\n";
            }
        }
    }

    void translate_expr(AstNode* n, std::ostream& out)
    {
        switch (n->kind)
        {
        case AstNode::NUMBER:
            out << n->value;
            return;
        case AstNode::VAR:
            out << n->name;
            return;
        case AstNode::BINOP:
            out << "(";
            translate_expr(n->left, out);
            out << " " << n->op << " ";
            translate_expr(n->right, out);
            out << ")";
            return;
        default:
            return;
        }
    }

    /** The "translate" half: a toy rewrite into C-like statements —
        just enough to show a second, independent downstream pass over
        the very same AST the interpreter above walks, the same "one
        AST, two consumers" shape demo-lrparser.cpp's interpret()/
        translate_to_postfix() pair demonstrates for a single
        expression. */
    void translate_program(AstNode* prog, std::ostream& out)
    {
        for (nat_t i = 0; i < prog->stmts.size(); ++i)
        {
            AstNode* stmt = prog->stmts[i];

            if (stmt->kind == AstNode::LET_STMT)
            {
                out << "double " << stmt->name << " = ";
                translate_expr(stmt->left, out);
                out << ";\n";
            }
            else
            {
                out << "print(";
                translate_expr(stmt->left, out);
                out << ");\n";
            }
        }
    }

    /** Registers every token and rule of the small statement language
        this demo interprets/translates:

            Program  -> StmtList
            StmtList -> StmtList Stmt | Stmt
            Stmt     -> let ID = E ; | E ;
            E        -> E + E | E - E | E * E | E / E | ( E ) | NUM | ID

        — left-recursive, so only `ParserKind::SLR`/`LR`/`LALR` can
        drive it (see the LL sub-grammar further down for the
        left-recursion-eliminated shape LL(1) requires instead). Called
        twice below, once per `Parser<AstNode*>` built with a different
        `ParserKind`, to make the "which algorithm drives the parse is a
        runtime choice" story concrete: the exact same sequence of
        add_token()/add_rule() calls produces two independently-usable
        parsers. */
    void build_statement_grammar(Parser<AstNode*>& p)
    {
        p.set_start_symbol("Program");

        const std::string letters =
            "(a|b|c|d|e|f|g|h|i|j|k|l|m|n|o|p|q|r|s|t|u|v|w|x|y|z)";
        const std::string digits = "(0|1|2|3|4|5|6|7|8|9)";

        // LET is registered before ID so that maximal munch's
        // equal-length tie (matching "let" as both the keyword and a
        // valid identifier) is broken in the keyword's favor — see
        // Lexer::add_token()'s own doc comment in lexer.hpp.
        p.add_token("LET", "let",
                   [](const Token& t) { return AstNode::tag(t.lexeme); });
        p.add_token("ID", letters + "(" + letters + "|" + digits + ")*",
                   [](const Token& t) { return AstNode::var(t.lexeme); });
        p.add_token("NUM", digits + digits + "*",
                   [](const Token& t)
                   { return AstNode::number(std::stod(t.lexeme)); });
        p.add_token("ASSIGN", "=",
                   [](const Token& t) { return AstNode::tag(t.lexeme); });
        p.add_token("SEMI", ";",
                   [](const Token& t) { return AstNode::tag(t.lexeme); });
        p.add_token("PLUS", "\\+",
                   [](const Token& t) { return AstNode::tag(t.lexeme); });
        p.add_token("MINUS", "-",
                   [](const Token& t) { return AstNode::tag(t.lexeme); });
        p.add_token("TIMES", "\\*",
                   [](const Token& t) { return AstNode::tag(t.lexeme); });
        p.add_token("DIVIDE", "/",
                   [](const Token& t) { return AstNode::tag(t.lexeme); });
        p.add_token("LPAREN", "\\(",
                   [](const Token& t) { return AstNode::tag(t.lexeme); });
        p.add_token("RPAREN", "\\)",
                   [](const Token& t) { return AstNode::tag(t.lexeme); });
        p.skip_token("WS", " ");

        p.set_precedence("PLUS", 1, Grammar::Associativity::LEFT);
        p.set_precedence("MINUS", 1, Grammar::Associativity::LEFT);
        p.set_precedence("TIMES", 2, Grammar::Associativity::LEFT);
        p.set_precedence("DIVIDE", 2, Grammar::Associativity::LEFT);

        p.add_rule("Program", Grammar::Sequence({"StmtList"}),
                  [](DynArray<AstNode*>&& c) { return c[0]; });

        p.add_rule("StmtList", Grammar::Sequence({"Stmt"}),
                  [](DynArray<AstNode*>&& c)
                  {
                      AstNode* list = new AstNode;
                      list->kind = AstNode::PROGRAM;
                      list->stmts.append(c[0]);
                      return list;
                  });

        p.add_rule("StmtList", Grammar::Sequence({"StmtList", "Stmt"}),
                  [](DynArray<AstNode*>&& c)
                  {
                      c[0]->stmts.append(c[1]);
                      return c[0];
                  });

        p.add_rule(
            "Stmt", Grammar::Sequence({"LET", "ID", "ASSIGN", "E", "SEMI"}),
            [](DynArray<AstNode*>&& c)
            {
                AstNode* n = new AstNode;
                n->kind = AstNode::LET_STMT;
                n->name = c[1]->name;
                n->left = c[3];
                destroy_ast(c[0]);
                destroy_ast(c[1]);
                destroy_ast(c[2]);
                destroy_ast(c[4]);
                return n;
            });

        p.add_rule("Stmt", Grammar::Sequence({"E", "SEMI"}),
                  [](DynArray<AstNode*>&& c)
                  {
                      AstNode* n = new AstNode;
                      n->kind = AstNode::EXPR_STMT;
                      n->left = c[0];
                      destroy_ast(c[1]);
                      return n;
                  });

        auto binop_rule = [](const char* op)
        {
            return [op](DynArray<AstNode*>&& c)
            {
                AstNode* n = AstNode::binop(op, c[0], c[2]);
                destroy_ast(c[1]);
                return n;
            };
        };

        p.add_rule("E", Grammar::Sequence({"E", "PLUS", "E"}),
                  binop_rule("+"));
        p.add_rule("E", Grammar::Sequence({"E", "MINUS", "E"}),
                  binop_rule("-"));
        p.add_rule("E", Grammar::Sequence({"E", "TIMES", "E"}),
                  binop_rule("*"));
        p.add_rule("E", Grammar::Sequence({"E", "DIVIDE", "E"}),
                  binop_rule("/"));
        p.add_rule("E", Grammar::Sequence({"LPAREN", "E", "RPAREN"}),
                  [](DynArray<AstNode*>&& c)
                  {
                      AstNode* n = c[1];
                      destroy_ast(c[0]);
                      destroy_ast(c[2]);
                      return n;
                  });
        p.add_rule("E", Grammar::Sequence({"NUM"}),
                  [](DynArray<AstNode*>&& c) { return c[0]; });
        p.add_rule("E", Grammar::Sequence({"ID"}),
                  [](DynArray<AstNode*>&& c) { return c[0]; });
    }

    /** Registers the classic left-recursion-eliminated arithmetic
        grammar (the same shape demo-llparser.cpp parses into a generic
        ParseTreeNode) onto an `LL`-kind Parser instead:

            E  -> T Eprime
            Eprime -> + T Eprime | eps
            T  -> F Tprime
            Tprime -> * F Tprime | eps
            F  -> ( E ) | NUM

        `Eprime`/`Tprime` can't just synthesize a value the way `E`/`T`
        do — by the time either reduces, it has no "left operand" to
        combine with yet, only whatever comes *after* it — so each
        instead synthesizes one link of a `TAIL` chain (`fold_tail()`'s
        input), and `E`/`T`'s own actions fold that chain against their
        `T`/`F` into real, left-associative BINOP nodes. This is the
        synthesized-attribute-only counterpart of the inherited-
        attribute accumulation demo-llparser.cpp's `eval_Eprime`/
        `eval_Tprime` do by hand over a parse tree — same underlying
        problem (recovering left-associativity from a right-recursive
        elimination of left recursion), solved inside `reduce()` instead
        of in a separate post-parse walk. */
    void build_ll_expression_grammar(Parser<AstNode*>& p)
    {
        p.set_start_symbol("E");

        const std::string digits = "(0|1|2|3|4|5|6|7|8|9)";

        p.add_token("NUM", digits + digits + "*",
                   [](const Token& t)
                   { return AstNode::number(std::stod(t.lexeme)); });
        p.add_token("PLUS", "\\+",
                   [](const Token& t) { return AstNode::tag(t.lexeme); });
        p.add_token("TIMES", "\\*",
                   [](const Token& t) { return AstNode::tag(t.lexeme); });
        p.add_token("LPAREN", "\\(",
                   [](const Token& t) { return AstNode::tag(t.lexeme); });
        p.add_token("RPAREN", "\\)",
                   [](const Token& t) { return AstNode::tag(t.lexeme); });
        p.skip_token("WS", " ");

        p.add_rule("E", Grammar::Sequence({"T", "Eprime"}),
                  [](DynArray<AstNode*>&& c)
                  { return fold_tail(c[0], c[1]); });
        p.add_rule("Eprime", Grammar::Sequence({"PLUS", "T", "Eprime"}),
                  [](DynArray<AstNode*>&& c)
                  {
                      AstNode* n = new AstNode;
                      n->kind = AstNode::TAIL;
                      n->op = "+";
                      n->left = c[1];
                      n->right = c[2];
                      destroy_ast(c[0]);
                      return n;
                  });
        p.add_rule("Eprime", Grammar::Sequence(),
                  [](DynArray<AstNode*>&&)
                  {
                      AstNode* n = new AstNode;
                      n->kind = AstNode::NIL;
                      return n;
                  });
        p.add_rule("T", Grammar::Sequence({"F", "Tprime"}),
                  [](DynArray<AstNode*>&& c)
                  { return fold_tail(c[0], c[1]); });
        p.add_rule("Tprime", Grammar::Sequence({"TIMES", "F", "Tprime"}),
                  [](DynArray<AstNode*>&& c)
                  {
                      AstNode* n = new AstNode;
                      n->kind = AstNode::TAIL;
                      n->op = "*";
                      n->left = c[1];
                      n->right = c[2];
                      destroy_ast(c[0]);
                      return n;
                  });
        p.add_rule("Tprime", Grammar::Sequence(),
                  [](DynArray<AstNode*>&&)
                  {
                      AstNode* n = new AstNode;
                      n->kind = AstNode::NIL;
                      return n;
                  });
        p.add_rule("F", Grammar::Sequence({"LPAREN", "E", "RPAREN"}),
                  [](DynArray<AstNode*>&& c)
                  {
                      AstNode* n = c[1];
                      destroy_ast(c[0]);
                      destroy_ast(c[2]);
                      return n;
                  });
        p.add_rule("F", Grammar::Sequence({"NUM"}),
                  [](DynArray<AstNode*>&& c) { return c[0]; });
    }
} // end anonymous namespace

int main()
{
    // Every Parser<AstNode*> built in this demo shares the same node
    // destroyer: whatever NodeType is still alive on the internal
    // builder's stack when a syntax error throws gets freed exactly the
    // same way a normally-completed parse's result would be, if the
    // caller passed it to destroy_ast() themselves.
    auto destroyer = [](AstNode*& n) { destroy_ast(n); };

    std::string program =
        "let x = 3 + 4 * 2 ; let y = x - 1 ; y * 2 ; x + y ;";

    cout << "Program:\n  " << program << "\n\n";

    // Same grammar, same tokens, same rules — built through the facade
    // twice, once per ParserKind, to show the underlying algorithm is a
    // runtime choice rather than something baked into the grammar or
    // the Parser<NodeType> type itself.
    for (ParserKind kind : {ParserKind::SLR, ParserKind::LALR})
    {
        Parser<AstNode*> parser(kind, destroyer);
        build_statement_grammar(parser);

        cout << (kind == ParserKind::SLR ? "-- SLR --\n" : "-- LALR --\n");
        cout << "Conflicts resolved by declared precedence: "
             << parser.get_conflicts().size() << "\n";

        AstNode* ast = parser.parse(program);

        cout << "Interpreted:\n";
        interpret_program(ast, cout);

        cout << "Translated:\n";
        translate_program(ast, cout);

        destroy_ast(ast);
        cout << endl;
    }

    // The same NodeType (AstNode*), a different grammar entirely, and a
    // different ParserKind: LL(1) over the classic left-recursion-
    // eliminated expression grammar.
    {
        Parser<AstNode*> ll_parser(ParserKind::LL, destroyer);
        build_ll_expression_grammar(ll_parser);

        std::string expr = "2 + 3 * 4";
        cout << "-- LL --\nExpression: " << expr << "\n";

        AstNode* ast = ll_parser.parse(expr);

        HashMap<std::string, real_t> empty_env;
        cout << "Interpreted (evaluated) result: "
             << interpret_expr(ast, empty_env) << "\n";
        cout << "Translated (infix): ";
        translate_expr(ast, cout);
        cout << endl;

        destroy_ast(ast);
    }

    // A deliberately malformed input: missing right-hand operand after
    // `+`. Caught and reported with its position, and — because
    // LRParserBase::parse()'s own try/catch calls our node_destroyer on
    // every AstNode* already built before the error, via the
    // ParserActionBuilder this facade drives internally — nothing built
    // so far leaks.
    try
    {
        Parser<AstNode*> parser(ParserKind::LALR, destroyer);
        build_statement_grammar(parser);

        AstNode* bad = parser.parse("let z = 3 + ;");
        destroy_ast(bad);
    }
    catch (const std::runtime_error& e)
    {
        cout << "\nParsing \"let z = 3 + ;\" failed as expected: "
             << e.what() << endl;
    }

    return 0;
}
