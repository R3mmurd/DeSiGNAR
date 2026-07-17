/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <iostream>
#include <cassert>
#include <lexer.hpp>

using namespace std;
using namespace Designar;

int main()
{
  // A tiny arithmetic-expression lexer: keywords registered before
  // the general identifier pattern, so "if" wins the tie against
  // "id" (both match length 2) by registration order.
  Lexer lexer;
  lexer.add_token("IF", "if");
  lexer.add_token("ID", "(a|b|c|d|e|f|g|h|i|j)(a|b|c|d|e|f|g|h|i|j)*");
  lexer.add_token("NUM", "(0|1|2|3|4|5|6|7|8|9)(0|1|2|3|4|5|6|7|8|9)*");
  lexer.add_token("PLUS", "\\+");
  lexer.add_token("STAR", "\\*");
  lexer.add_token("LPAREN", "\\(");
  lexer.add_token("RPAREN", "\\)");
  lexer.add_token("WS", " ");

  {
    DynArray<Token> tokens = lexer.tokenize("if + ifa");

    assert(tokens.size() == 5);
    assert(tokens[0].name == "IF" && tokens[0].lexeme == "if");
    assert(tokens[1].name == "WS");
    assert(tokens[2].name == "PLUS" && tokens[2].lexeme == "+");
    assert(tokens[3].name == "WS");
    assert(tokens[4].name == "ID" && tokens[4].lexeme == "ifa");

    cout << "Lexer: keyword vs identifier maximal-munch/tie-break Everything ok!\n";
  }

  {
    DynArray<Token> tokens = lexer.tokenize("a12*(bcd+34)");

    // "a12" is not a NUM (starts with a letter) and the ID pattern
    // only covers letters, so this actually splits into ID("a") then
    // NUM("12") since the identifier alphabet used here has no
    // digits — verifying maximal munch stops exactly where a
    // pattern's own DFA stops accepting, not at some arbitrary
    // heuristic boundary.
    assert(tokens.size() == 8);
    assert(tokens[0].name == "ID" && tokens[0].lexeme == "a");
    assert(tokens[1].name == "NUM" && tokens[1].lexeme == "12");
    assert(tokens[2].name == "STAR" && tokens[2].lexeme == "*");
    assert(tokens[3].name == "LPAREN" && tokens[3].lexeme == "(");
    assert(tokens[4].name == "ID" && tokens[4].lexeme == "bcd");
    assert(tokens[5].name == "PLUS" && tokens[5].lexeme == "+");
    assert(tokens[6].name == "NUM" && tokens[6].lexeme == "34");
    assert(tokens[7].name == "RPAREN" && tokens[7].lexeme == ")");

    cout << "Lexer: multi-token maximal munch across a full expression Everything ok!\n";
  }

  // Position tracking: every token records where it started in the
  // original input.
  {
    DynArray<Token> tokens = lexer.tokenize("a bb");
    assert(tokens[0].position == 0);
    assert(tokens[1].position == 1);
    assert(tokens[2].position == 2);

    cout << "Lexer: token position tracking Everything ok!\n";
  }

  // An unrecognized character (not covered by any registered
  // pattern) must throw rather than silently skip or misparse.
  {
    bool threw = false;

    try
    {
      lexer.tokenize("a $ b");
    }
    catch (const runtime_error&)
    {
      threw = true;
    }

    assert(threw);

    cout << "Lexer: unrecognized character reports a lexical error Everything ok!\n";
  }

  // An empty input tokenizes to zero tokens, not an error.
  {
    DynArray<Token> tokens = lexer.tokenize("");
    assert(tokens.size() == 0);

    cout << "Lexer: empty input Everything ok!\n";
  }

  cout << "Everything ok!\n";
  return 0;
}
