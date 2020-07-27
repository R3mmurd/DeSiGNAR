/*
  This file is part of Designar.
  
  Author: Alejandro Mujica (aledrums@gmail.com)
*/

# include <cassert>
# include <iostream>

using namespace std;

# include <range.H>
# include <list.H>

using namespace Designar;

int main()
{
  Range<int> r1(5);
  assert(r1.min() == 0);
  assert(r1.max() == 5);
  assert(r1.step_size() == 1);
  assert(r1.size() == 5);
  assert(r1.filter([](auto item){ return item % 2 == 0; }).equal({0,2,4}));
  assert(r1.map([](auto item){ return item * 2; }).equal({0,2,4,6,8}));
  assert(r1.fold(0, [](auto item, auto acc){ return item + acc; }) == 10);
  assert(r1.fold(1, [](auto item, auto acc){ return item * acc; }) == 0);
  assert(r1.all([] (auto item) { return item < 10; }));
  assert(r1.exists([] (auto item) { return not (item & 1); }));
  assert(r1.none([] (auto item) { return item >= 10; }));
  assert(r1.to_list().equal({0,1,2,3,4}));
  
  Range<int> r2(-5, 5);
  assert(r2.min() == -5);
  assert(r2.max() == 5);
  assert(r2.step_size() == 1);
  assert(r2.size() == 10);

  Range<int> r3(0, 10, 2);
  assert(r3.min() == 0);
  assert(r3.max() == 10);
  assert(r3.step_size() == 2);
  assert(r3.size() == 5);
  assert(r3.filter([](auto item){ return item % 2 == 0; }).equal({0,2,4,6,8}));
  assert(r3.map([](auto item){ return item * 2; }).equal({0,4,8,12,16}));
  assert(r3.fold(0, [](auto item, auto acc){ return item + acc; }) == 20);
  assert(r3.fold(1, [](auto item, auto acc){ return item * acc; }) == 0);
  assert(r3.all([] (auto item) { return not (item & 1); }));
  assert(r3.exists([] (auto item) { return not (item & 1); }));
  assert(r3.none([] (auto item) { return item & 1; }));
  assert(r3.to_list().equal({0,2,4,6,8}));

  Range<int> r4(-5, 5, 3);
  assert(r4.min() == -5);
  assert(r4.max() == 5);
  assert(r4.step_size() == 3);
  assert(r4.size() == 4);

  Range<int> r5(-5, 5, 4);
  assert(r5.min() == -5);
  assert(r5.max() == 5);
  assert(r5.step_size() == 4);
  assert(r5.size() == 3);

  Range<int> r6(-5, 5, 5);
  assert(r6.min() == -5);
  assert(r6.max() == 5);
  assert(r6.step_size() == 5);
  assert(r6.size() == 2);

  Range<int> r7(-5, 5, 10);
  assert(r7.min() == -5);
  assert(r7.max() == 5);
  assert(r7.step_size() == 10);
  assert(r7.size() == 1);

  Range<int> r8(5, 5);
  assert(r8.min() == 5);
  assert(r8.max() == 5);
  assert(r8.step_size() == 1);
  assert(r8.size() == 0);
  

  Range<double> r9(0, 10., 0.1);
  assert(num_equal(r9.min(), 0.));
  assert(num_equal(r9.max(), 10.));
  assert(num_equal(r9.step_size(), 0.1));
  assert(r9.size() == 100);

  Range<double> r10(-5., 5., 0.01);
  assert(num_equal(r10.min(), -5.));
  assert(num_equal(r10.max(), 5.));
  assert(num_equal(r10.step_size(), 0.01));
  assert(r10.size() == 1000);

  Range<double> r11(-5., 5., 0.02);
  assert(num_equal(r11.min(), -5.));
  assert(num_equal(r11.max(), 5.));
  assert(num_equal(r11.step_size(), 0.02));
  assert(r11.size() == 500);
  
  cout << "Everything ok!\n";
  return 0;
}
