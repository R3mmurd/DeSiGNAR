/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <iostream>
#include <string>
#include <vector>

using namespace std;

#include <automata.hpp>

using namespace Designar;

int main()
{
  vector<string> tests = {"abb", "aabb", "babb", "ab", "abbb"};

  // Thompson's construction, straight from a regex — no NFA states or
  // transitions written out by hand.
  NFA<char> nfa = regex_to_nfa("(a|b)*abb");

  cout << "NFA \"(a|b)*abb\" (" << nfa.get_num_states() << " states):" << endl;

  for (const string& s : tests)
  {
    cout << "  accepts(\"" << s << "\"): " << (nfa.accepts(s) ? "yes" : "no") << endl;
  }

  // The subset construction: same language, deterministic states.
  DFA<char> dfa = nfa.to_dfa();

  cout << "DFA after subset construction (" << dfa.get_num_states() << " states):" << endl;

  for (const string& s : tests)
  {
    cout << "  accepts(\"" << s << "\"): " << (dfa.accepts(s) ? "yes" : "no") << endl;
  }

  return 0;
}
