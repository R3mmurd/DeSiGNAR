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

# include <array.H>
# include <map.H>

using namespace std;
using namespace Designar;

int main()
{
  ArrayMap<string, lint_t> array_map = {{"One",1},{"Two",2},
					{"Three",3},{"Four",4}};

  assert(array_map.size() == 4);
  assert(array_map["One"] == 1);
  assert(array_map["Two"] == 2);
  assert(array_map["Three"] == 3);
  assert(array_map["Four"] == 4);

  array_map.append("Five", 5);

  assert(array_map.size() == 5);

  assert(array_map["Five"] == 5);

  array_map["Six"] = 6;

  assert(array_map.size() == 6);

  assert(array_map["Six"] == 6);

  assert(array_map.equal({{"One",1},{"Two",2},{"Three",3},{"Four",4},{"Five",5},{"Six",6}}));

  
  auto sum = array_map.fold(map_item(string(""), 0), [] (const auto & p,
							 const auto & acc)
			    {
			      return map_item(key(acc)+key(p)+"+",
					      value(acc)+value(p));
			    });

  sum.first.pop_back();
  assert(sum.first == "One+Two+Three+Four+Five+Six" and sum.second == 21);

  auto prod = array_map.fold(map_item(string(""), 1), [] (const auto & p,
							  const auto & acc)
			     {
			       return map_item(key(acc)+key(p)+"*",
					       value(acc)*value(p));
			     });
  
  prod.first.pop_back();
  assert(prod.first == "One*Two*Three*Four*Five*Six" and prod.second == 720);

  TreeMap<string, lint_t> tree_map = {{"One",1},{"Two",2},
				      {"Three",3},{"Four",4}};

  assert(tree_map.size() == 4);
  assert(tree_map["One"] == 1);
  assert(tree_map["Two"] == 2);
  assert(tree_map["Three"] == 3);
  assert(tree_map["Four"] == 4);

  tree_map.append("Five", 5);

  assert(tree_map.size() == 5);

  assert(tree_map["Five"] == 5);

  tree_map["Six"] = 6;

  assert(tree_map.size() == 6);

  assert(tree_map["Six"] == 6);
  
  assert(tree_map.equal({{"One",1},{"Two",2},{"Three",3},{"Four",4},{"Five",5},{"Six",6}}));

  sum.first.clear();
  sum.second = 0;
  
  sum = tree_map.fold(map_item(string(""), 0), [] (const auto & p,
						   const auto & acc)
		      {
			return map_item(key(acc)+key(p)+"+",
					value(acc)+value(p));
		      });



  
  sum.first.pop_back();
  assert(sum.first == "Five+Four+One+Six+Three+Two" and sum.second == 21);

  prod.first.clear();
  prod.second = 0;
  
  prod = tree_map.fold(map_item(string(""), 1), [] (const auto & p,
						    const auto & acc)
		       {
			 return map_item(key(acc)+key(p)+"*",
					 value(acc)*value(p));
		       });
  
  prod.first.pop_back();
  assert(prod.first == "Five*Four*One*Six*Three*Two" and prod.second == 720);
  
  HashMap<string, lint_t> hash_map = {{"One",1},{"Two",2},
				      {"Three",3},{"Four",4}};

  assert(hash_map.size() == 4);
  assert(hash_map["One"] == 1);
  assert(hash_map["Two"] == 2);
  assert(hash_map["Three"] == 3);
  assert(hash_map["Four"] == 4);

  hash_map.append("Five", 5);

  assert(hash_map.size() == 5);

  assert(hash_map["Five"] == 5);

  hash_map["Six"] = 6;

  assert(hash_map.size() == 6);

  assert(hash_map["Six"] == 6);
  
  
  sum.first.clear();
  sum.second = 0;
  
  sum = hash_map.fold(map_item(string(""), 0), [] (const auto & p,
						   const auto & acc)
		      {
			return map_item(key(acc)+key(p)+"+",
					value(acc)+value(p));
		      });



  
  sum.first.pop_back();
  assert(sum.second == 21);

  prod.first.clear();
  prod.second = 0;
  
  prod = hash_map.fold(map_item(string(""), 1), [] (const auto & p,
						     const auto & acc)
			{
			  return map_item(key(acc)+key(p)+"*",
					  value(acc)*value(p));
			});
  
  prod.first.pop_back();
  assert(prod.second == 720);
  
  cout << "Everything ok!\n";
  
  return 0;
}
