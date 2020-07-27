/*
  This file is part of Designar.
  
  Author: Alejandro Mujica (aledrums@gmail.com)
*/

# include <bitset.H>
# include <random.H>

using namespace std;
using namespace Designar;

int main()
{
  DynBitSet bs;
  assert(bs.is_empty());
  assert(bs.size() == 0);

  bs.append(true);
  assert(bs.size() == 1);
  assert(not bs.is_empty());

  assert(bs[0]);
  bs[0] = false;
  assert(not bs[0]);

  bs.remove_last();
  assert(bs.is_empty());
  assert(bs.size() == 0);

  DynBitSet bs1(64);
  assert(bs1.size() == 64);
  for (auto i = 0; i < 64; ++i)
    assert(not bs1[i]);

  DynBitSet bs2(64, true);
  assert(bs2.size() == 64);
  for (auto i = 0; i < 64; ++i)
    assert(bs2[i]);

  DynBitSet bs3{0,1,1,1,0,0};
  assert(bs3.size() == 6);
  assert(bs3[0] == 0);
  assert(bs3[1] == 0);
  assert(bs3[2] == 1);
  assert(bs3[3] == 1);
  assert(bs3[4] == 1);
  assert(bs3[5] == 0);

  assert(bs3.to_string() == "011100");

  cout << "Everything ok!\n";
  return 0;
}


