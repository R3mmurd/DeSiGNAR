/*
  This file is part of Designar.
  
  Author: Alejandro Mujica (aledrums@gmail.com)
*/

# include <iostream>

using namespace std;

# include <math.H>

int main()
{
  assert(Designar::pow(0UL,0UL) == 1);
  assert(Designar::pow(10UL,0UL) == 1);
  assert(Designar::pow(10UL,1UL) == 10);
  assert(Designar::pow(2UL,3UL) == 8);
  assert(Designar::pow(3UL,3UL) == 27);
  assert(Designar::pow(-3,2UL) == 9);
  assert(Designar::pow(-3,3UL) == -27);
  auto r = Designar::pow(-3,3UL);  
  assert(Designar::pow(-3,-3) == 1 / r);
  Designar::pow(-3.5,-3.);
  Designar::pow(-3.5,2);
  Designar::pow(-3.5,2UL);
  Designar::pow(3UL,2.);
  Designar::pow(3,2.);
  cout << "Everything ok!\n";

  return 0;
};
