/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <iostream>

using namespace std;

#include <grammar.hpp>

using namespace Designar;

namespace
{
  void print_set(const string& label, const HashSet<string>& s)
  {
    cout << label << " = { ";

    for (const string& sym : s)
    {
      cout << (sym == Grammar::EPSILON ? "eps" : sym) << " ";
    }

    cout << "}\n";
  }
} // end anonymous namespace

int main()
{
  // The classic expression grammar:
  //   E  -> T E'
  //   E' -> + T E' | eps
  //   T  -> F T'
  //   T' -> * F T' | eps
  //   F  -> ( E ) | id
  Grammar g;
  g.set_start_symbol("E");

  g.add_production("E", Grammar::Sequence({"T", "E'"}));

  g.add_production("E'", Grammar::Sequence({"+", "T", "E'"}));
  g.add_production("E'", Grammar::Sequence()); // eps

  g.add_production("T", Grammar::Sequence({"F", "T'"}));

  g.add_production("T'", Grammar::Sequence({"*", "F", "T'"}));
  g.add_production("T'", Grammar::Sequence()); // eps

  g.add_production("F", Grammar::Sequence({"(", "E", ")"}));
  g.add_production("F", Grammar::Sequence({"id"}));

  auto first = g.compute_first();
  auto follow = g.compute_follow(first);

  cout << "FIRST sets:\n";

  for (const string& nt : g.get_nonterminals())
  {
    print_set("FIRST(" + nt + ")", *first.search(nt));
  }

  cout << "\nFOLLOW sets:\n";

  for (const string& nt : g.get_nonterminals())
  {
    print_set("FOLLOW(" + nt + ")", *follow.search(nt));
  }

  return 0;
}
