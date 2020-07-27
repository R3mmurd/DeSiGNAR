/*
  This file is part of Designar.
  
  Author: Alejandro Mujica (aledrums@gmail.com)
*/

# include <iostream>

using namespace std;

# include <buildgraph.H>
# include <now.H>

using namespace Designar;

using GT = Graph<nat_t>;

int main(int argc, char * argv[]) 
{
  nat_t num_nodes  = argc < 2 ? 40  : stoul(argv[1]);
  real_t prob_arcs = argc < 3 ? 0.3 : stod(argv[2]);
  rng_seed_t seed  = argc < 4 ? 10  : stoul(argv[3]);
  
  Now now;
  
  cout << "Building random graph...\n";
  now.start();
  GT g = er_random_graph<GT>(num_nodes, prob_arcs, seed, true);
  auto dt1 = now.elapsed();
  cout << "Done!\n";
  
  cout << "Computing min cut with slow algorithm...\n";
 
  now.start();
  auto t1 = KargerMinCut<GT>().compute_min_cut(g);
  auto dt2 = now.elapsed();
  cout << "Slow algorithm done!\n";
  
  cout << "Computing min cut with fast algorithm...\n";
  now.start();
  auto t2 = KargerMinCut<GT>().compute_min_cut_fast(g);
  auto dt3 = now.elapsed();
  cout << "Fast algorithm done!\n";

  cout << "Graph built in " << dt1 << " ms.\n";
  
  cout << "Slow algorithm got " << get<2>(t1).size() << " computed in "
       << dt2 << " ms.\n";
  
  cout << "Fast algorithm got " << get<2>(t2).size() << " computed in "
       << dt3 << " ms.\n";
  
  return 0;
}
