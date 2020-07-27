/*
  This file is part of Designar.
  
  Author: Alejandro Mujica (aledrums@gmail.com)
*/

# include <iostream>

using namespace std;

# include <range.H>

using namespace Designar;

int main()
{
  cout << "IntRange(10, 20, 3)\n";
  for (lint_t item : IntRange(10, 20, 3))
    cout << item << endl;

  cout << "IntRange(-10, 10, 1)\n";
  for (lint_t item : IntRange(-10, 10))
    cout << item << endl;

  cout << "IntRange(0, 10, 1)\n";
  for (lint_t item : IntRange(10))
    cout << item << endl;

  cout << "UIntRange(10, 20, 3)\n";
  for (nat_t item : UIntRange(10, 20, 3))
    cout << item << endl;

  cout << "UIntRange(10, 10, 1)\n";
  for (nat_t item : UIntRange(10, 10))
    cout << item << endl;

  cout << "IntRange(0, 10, 1)\n";
  for (nat_t item : UIntRange(10))
    cout << item << endl;

  cout << "RealRange(10, 20, 3)\n";
  for (real_t item : RealRange(10, 20, 3))
    cout << item << endl;

  cout << "RealRange(1, 2, 0.1)\n";
  for (real_t item : RealRange(1, 2, 0.1))
    cout << item << endl;

  cout << "RealRange(1, 2, 0.05)\n";
  for (real_t item : range(1., 2., 0.05))
    cout << item << endl;
  
  return 0;
}
