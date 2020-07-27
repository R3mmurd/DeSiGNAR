/*
  This file is part of Designar.
  
  Author: Alejandro Mujica (aledrums@gmail.com)
*/

# include <iostream>

using namespace std;

# include <graphalgorithms.H>

using namespace Designar;

using DGT = Digraph<string>;

void write_graph(DGT & g, const char * name)
{
  cout << "digraph " << name << "\n";
  cout << g.get_num_nodes() << " nodes, " << g.get_num_arcs()
       << " arcs\n\n";

  cout << "Node set of " << name << "\n";
  g.for_each_node([&] (auto n) { cout << n->get_info() << endl; });

  cout << "\nArc set of " << name << "\n";
  g.for_each_arc([&] (auto a)
		 {
		   const string & s = a->get_src_node()->get_info();
		   const string & t = a->get_tgt_node()->get_info();
		   cout << s << " -> " << t  << endl;
		 });

		 cout << "\n\n";
}

int main()
{
  DGT dag;
  DGT::Node * a = dag.insert_node("Medias");
  DGT::Node * b = dag.insert_node("Interiores");
  DGT::Node * c = dag.insert_node("Camisa");
  DGT::Node * d = dag.insert_node("Pantalón");
  DGT::Node * e = dag.insert_node("Corbata");
  DGT::Node * f = dag.insert_node("Zapatos");
  DGT::Node * g = dag.insert_node("Correa");
  DGT::Node * h = dag.insert_node("Chaleco");
  DGT::Node * i = dag.insert_node("Paltó");

  dag.insert_arc(a, f);
  dag.insert_arc(b, d);
  dag.insert_arc(c, e);
  dag.insert_arc(c, g);
  dag.insert_arc(d, f);
  dag.insert_arc(d, g);
  dag.insert_arc(e, h);
  dag.insert_arc(f, i);
  dag.insert_arc(g, i);
  dag.insert_arc(h, i);

  write_graph(dag, "dag");

  auto dfts = df_topological_sort(dag);

  cout << "Df topological sort: ";

  dfts.for_each([] (auto p) { cout << p->get_info() << ' '; });

  cout << endl;

  auto bfts = bf_topological_sort(dag);

  cout << "Bf topological sort: ";

  bfts.for_each([] (auto p) { cout << p->get_info() << ' '; });

  cout << endl;

  auto tr = topological_ranks(dag);

  cout << "Topological ranks\n";

  nat_t r = 0;
  for (auto & l : tr)
    {
      cout << "Rank " << ++r << ": ";
      
      for (auto p : l)
	cout << p->get_info() << ' ';

      cout << endl;
    }
  
  return 0;
}
