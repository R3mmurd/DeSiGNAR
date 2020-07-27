/*
  This file is part of Designar.
  
  Author: Alejandro Mujica (aledrums@gmail.com)
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

  key(sum).pop_back();
  assert(key(sum) == "One+Two+Three+Four+Five+Six" and value(sum) == 21);

  auto prod = array_map.fold(map_item(string(""), 1), [] (const auto & p,
							  const auto & acc)
			     {
			       return map_item(key(acc)+key(p)+"*",
					       value(acc)*value(p));
			     });
  
  key(prod).pop_back();
  assert(key(prod) == "One*Two*Three*Four*Five*Six" and value(prod) == 720);

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

  key(sum).clear();
  value(sum) = 0;
  
  sum = tree_map.fold(map_item(string(""), 0), [] (const auto & p,
						   const auto & acc)
		      {
			return map_item(key(acc)+key(p)+"+",
					value(acc)+value(p));
		      });



  
  key(sum).pop_back();
  assert(key(sum) == "Five+Four+One+Six+Three+Two" and value(sum) == 21);

  key(prod).clear();
  value(prod) = 0;
  
  prod = tree_map.fold(map_item(string(""), 1), [] (const auto & p,
						    const auto & acc)
		       {
			 return map_item(key(acc)+key(p)+"*",
					 value(acc)*value(p));
		       });
  
  key(prod).pop_back();
  assert(key(prod) == "Five*Four*One*Six*Three*Two" and value(prod) == 720);
  
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
  
  
  key(sum).clear();
  value(sum) = 0;
  
  sum = hash_map.fold(map_item(string(""), 0), [] (const auto & p,
						   const auto & acc)
		      {
			return map_item(key(acc)+key(p)+"+",
					value(acc)+value(p));
		      });



  
  key(sum).pop_back();
  assert(value(sum) == 21);

  key(prod).clear();
  value(prod) = 0;
  
  prod = hash_map.fold(map_item(string(""), 1), [] (const auto & p,
						     const auto & acc)
			{
			  return map_item(key(acc)+key(p)+"*",
					  value(acc)*value(p));
			});
  
  key(prod).pop_back();
  assert(value(prod) == 720);
  
  cout << "Everything ok!\n";
  
  return 0;
}
