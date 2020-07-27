/*
  This file is part of Designar.
  
  Author: Alejandro Mujica (aledrums@gmail.com)
*/

# include <iostream>

using namespace std;

# include <relation.H>

using namespace Designar;

int main()
{
  constexpr nat_t N = 10;
  
  EquivalenceRelation rel(N);

  for (nat_t i = 0; i < N - 1; ++i)
    {
      assert(rel.are_connected(i, i));

      for (nat_t j = i + 1; j < N; ++j)
	assert(not rel.are_connected(i, j));
    }

  assert(rel.get_num_blocks() == N);

  rel.join(4, 3);
  assert(rel.get_num_blocks() == N - 1);
  assert(rel.are_connected(4, 3));


  rel.join(3, 8);
  assert(rel.get_num_blocks() == N - 2);
  assert(rel.are_connected(4, 3));
  assert(rel.are_connected(8, 3));
  assert(rel.are_connected(4, 8));

  rel.join(6, 5);
  assert(rel.are_connected(6, 5));
  assert(not rel.are_connected(6, 8));
  assert(rel.get_num_blocks() == N - 3);

  TRelation<string> t_rel({"P1", "P2", "P3", "P4", "P5"});

  assert(t_rel.size() == 5);
  assert(t_rel.get_num_blocks() == 5);
 
  assert(t_rel.are_connected("P1", "P1"));
  assert(t_rel.are_connected("P2", "P2"));
  assert(not t_rel.are_connected("P2", "P3"));

  t_rel.join("P1", "P2");
  assert(t_rel.are_connected("P1", "P2"));
  assert(not t_rel.are_connected("P2", "P3"));
  assert(t_rel.get_num_blocks() == 4);

  t_rel.join("P1", "P5");
  t_rel.join("P1", "P3");
  assert(t_rel.are_connected("P1", "P2"));
  assert(t_rel.are_connected("P1", "P3"));
  assert(t_rel.are_connected("P1", "P5"));
  assert(t_rel.are_connected("P2", "P3"));
  assert(t_rel.are_connected("P2", "P5"));
  assert(t_rel.are_connected("P3", "P5"));
  assert(t_rel.get_num_blocks() == 2);
  
  cout << "Everything ok!\n";
  
  return 0;
}
