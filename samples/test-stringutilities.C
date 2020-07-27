/*
  This file is part of Designar.
  
  Author: Alejandro Mujica (aledrums@gmail.com)
*/

# include <cassert>

# include <iostream>

# include <list.H>
# include <stringutilities.H>

using namespace std;
using namespace Designar;

int main()
{
  string fst_str = "foo, faa, bar";
  string sep = ", ";

  cout << "Testing split_string(" << dq(fst_str) << ", " << dq(sep) << ")\n";
  DLList<string> fst_split_res = split_string<DLList<string>>(fst_str, ", ");
  assert(fst_split_res.equal({"foo", "faa", "bar"}));

  sep = " - ";
  cout << "Testing join_string(" << dq(sep)
       << ", {\"foo\", \"faa\", \"bar\"})\n";
  string fst_join_res = join_string(" - ", fst_split_res);
  assert(fst_join_res == "foo - faa - bar");

  sep = " any str ";
  cout << "Testing split_string(\"\", " << dq(sep) << ")\n";
  assert(split_string<DLList<string>>("", sep).equal({}));

  cout << "Testing join_string(" << dq(sep) << ", {})\n";
  assert(join_string<DLList<string>>(sep, {}) == "");

  string snd_str = "foo1faa2bar3boo";
  string s_pattern = "\\d";
  regex pattern = regex("\\d");
  cout << "Testing split_string_re(" << dq(snd_str) << ", "
       << dq(s_pattern) << ")\n";
  DLList<string> snd_split_res =
    split_string_re<DLList<string>>(snd_str, pattern);
  assert(snd_split_res.equal({"foo", "faa", "bar", "boo"}));

  assert(split_string<DLList<string>>(fst_str, " xxx ").equal({fst_str}));
  
  cout << "Everything ok!\n";
  return 0;
}
