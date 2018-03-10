/*
  This file is part of Designar.
  Copyright (C) 2017 by Alejandro J. Mujica

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.

  Any user request of this software, write to 

  Alejandro Mujica

  aledrums@gmail.com
*/

# include <bitset.H>

using namespace std;
using namespace Designar;

string to_str(bool v)
{
  return v ? "true" : "false";
}

template <typename It>
string to_str(const It & it)
{
  stringstream s;
  s << "Iterator in position " << it.get_position();
  return s.str();
}

template <class Array, typename T = int>
void demo_array_iterator(const initializer_list<T> & xs)
{
  Array a{xs};
  
  cout << "Declaring iterator explicitly\n";
  for (typename Array::Iterator it(a); it.has_current(); it.next())
    cout << it.get_current() << ' ';
  cout << endl;

  cout << "Iterating like stl\n";
  for (auto it = a.begin(); it != a.end(); ++it)
    cout << *it << ' ';
  cout << endl;
  
  cout << "Iterating by using range for\n";
  for (auto item : a)
    cout << item << ' ';
  cout << endl;
  
  auto it1 = a.begin();
  auto it2 = it1;
  ++it2;
  
  cout << to_str(it1) << " < " << to_str(it2)
       << " = " << to_str(it1 < it2) << endl;
  
  cout << to_str(it1) << " <= " << to_str(it2) << " = "
       << to_str(it1 <= it2) << endl;
  
  cout << to_str(it1) << " > " << to_str(it2) << " = "
       << to_str(it1 > it2) << endl;
  
  cout << to_str(it1) << " >= " << to_str(it2) << " = "
       << to_str(it1 >= it2) << endl;

  cout << "(" << to_str(it2) << ") += int(3) => ";
  it2 += 3;
  cout << to_str(it2) << endl;

  cout << "--(" << to_str(it2) << ") => ";
  --it2;
  cout << to_str(it2) << endl;
}

int main()
{
  initializer_list<int> xs{1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20};

  cout << "Array Iterators\n"
       << "===============\n";

  cout << "\nIterators for FixedArray<int>\n";
  demo_array_iterator<FixedArray<int>>(xs);

  cout << "\nIterators for DynArray<int>\n";
  demo_array_iterator<DynArray<int>>(xs);

  cout << "\nIterators for ArraySet<int>\n";
  demo_array_iterator<ArraySet<int>>(xs);

  cout << "\nIterators for SortedArraySet<int>\n";
  demo_array_iterator<SortedArraySet<int>>(xs);

  cout << "\nIterators for DynBitSet\n";
  demo_array_iterator<DynBitSet, bool>({1,1,1,0,0,0,1,0,1,0,0,1,0,0,0,1,1,1});
  
  return 0;
}
