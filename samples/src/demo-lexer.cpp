/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <iostream>

using namespace std;

#include <lexer.hpp>

using namespace Designar;

int main()
{
    Lexer lexer;
    lexer.add_token("IF", "if");
    lexer.add_token("ELSE", "else");
    lexer.add_token("ID",
                    "(a|b|c|d|e|f|g|h|i|j|k|l|m|n|o|p|q|r|s|t|u|v|w|x|y|z)"
                    "(a|b|c|d|e|f|g|h|i|j|k|l|m|n|o|p|q|r|s|t|u|v|w|x|y|z|"
                    "0|1|2|3|4|5|6|7|8|9)*");
    lexer.add_token("NUM", "(0|1|2|3|4|5|6|7|8|9)(0|1|2|3|4|5|6|7|8|9)*");
    lexer.add_token("ASSIGN", "=");
    lexer.add_token("PLUS", "\\+");
    lexer.add_token("LPAREN", "\\(");
    lexer.add_token("RPAREN", "\\)");
    lexer.add_token("WS", " ");

    DynArray<Token> tokens = lexer.tokenize("if (x = y1 + 10) else result = 0");

    for (nat_t i = 0; i < tokens.size(); ++i)
    {
        const Token& t = tokens[i];

        if (t.name == "WS")
        {
            continue;
        }

        cout << t.name << "(\"" << t.lexeme << "\") @" << t.position << endl;
    }

    return 0;
}
