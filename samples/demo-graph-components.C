/*
  This file is part of Designar.
  
  Author: Alejandro Mujica (aledrums@gmail.com)
*/

# include <iostream>

using namespace std;

# include <graphalgorithms.H>

using namespace Designar;

using GT  = Graph<string>;
using DGT = Digraph<string>;

template <class G>
void print_graph(const G & g, const string & name)
{
  cout << name << endl;

  cout << g.get_num_nodes() << " nodes:\n";

  g.for_each_node([] (auto p)
		  {
		    cout << p->get_info() << endl;
		  });

  cout << g.get_num_arcs() << " arcs:\n";

  string c = g.is_digraph() ? "->" : " -- ";

  g.for_each_arc([&c] ( auto a)
		 {
		    auto p = a->get_src_node();
		    auto q = a->get_tgt_node();
		   cout << p->get_info() << c
			<< q->get_info() << endl;
		 });
    
}

int main()
{
  GT g;

  GT::Node * ga = g.insert_node("a");
  GT::Node * gb = g.insert_node("b");
  GT::Node * gc = g.insert_node("c");
  GT::Node * gd = g.insert_node("d");
  GT::Node * ge = g.insert_node("e");
  g.insert_node("f");

  g.insert_arc(ga, gb);
  g.insert_arc(gb, gc);
  g.insert_arc(gc, ga);

  g.insert_arc(gd, ge);

  print_graph(g, "Graph g");

  auto l = compute_connected_components(g);

  nat_t i = 0;
  for (const GT & gg : l)
    {
      stringstream s;
      s << "Component " << ++i;
      print_graph(gg, s.str());
    }

  GT gg;

  GT::Node * gga = gg.insert_node("a");
  GT::Node * ggb = gg.insert_node("b");
  GT::Node * ggc = gg.insert_node("c");
  GT::Node * ggd = gg.insert_node("d");
  GT::Node * gge = gg.insert_node("e");
  GT::Node * ggf = gg.insert_node("f");
  GT::Node * ggg = gg.insert_node("g");
  GT::Node * ggh = gg.insert_node("h");
  GT::Node * ggi = gg.insert_node("i");
  GT::Node * ggj = gg.insert_node("j");
  GT::Node * ggk = gg.insert_node("k");
  GT::Node * ggl = gg.insert_node("l");
  GT::Node * ggm = gg.insert_node("m");
  GT::Node * ggn = gg.insert_node("n");

  gg.insert_arc(ggm, ggn);
  gg.insert_arc(ggm, ggi);
  gg.insert_arc(ggi, ggn);
  gg.insert_arc(ggi, ggl);
  gg.insert_arc(ggi, ggj);
  gg.insert_arc(ggj, gga);
  gg.insert_arc(gga, ggb);
  gg.insert_arc(gga, ggd);
  gg.insert_arc(ggb, ggd);
  gg.insert_arc(ggb, gge);
  gg.insert_arc(ggd, gge);
  gg.insert_arc(ggd, ggc);
  gg.insert_arc(gge, ggc);
  gg.insert_arc(gge, ggf);
  gg.insert_arc(ggc, ggf);
  gg.insert_arc(ggf, ggk);
  gg.insert_arc(ggf, ggg);
  gg.insert_arc(ggf, ggh);
  gg.insert_arc(ggg, ggh);

  print_graph(gg, "gg");
		
  auto cn = compute_cut_nodes(gg);

  cout << "gg Cut nodes: ";
  
  for (auto p : cn)
    cout << p->get_info() << ' ';
  cout << endl << endl;

  auto t = compute_cut_nodes_connected_components(gg, cn);
  
  i = 0;
  for (const GT & gg : get<0>(t))
    {
      stringstream s;
      s << "Cut nodes Component " << ++i;
      print_graph(gg, s.str());
    }

  print_graph(get<1>(t), "Cut graph");

  cout << "Cross arcs\n";
  for (GT::Arc * a : get<2>(t))
    {
      auto p = a->get_src_node();
      auto q = a->get_tgt_node();
      cout << p->get_info() << " -- "
	   << q->get_info() << endl;      
    }
  
  DGT dg;
  
  DGT::Node * dga = dg.insert_node("a");
  DGT::Node * dgb = dg.insert_node("b");
  DGT::Node * dgc = dg.insert_node("c");
  DGT::Node * dgd = dg.insert_node("d");
  DGT::Node * dge = dg.insert_node("e");
  DGT::Node * dgf = dg.insert_node("f");

  dg.insert_arc(dga, dgb);
  dg.insert_arc(dgb, dgc);
  dg.insert_arc(dgc, dga);

  dg.insert_arc(dgc, dgd);
  dg.insert_arc(dgd, dge);
  dg.insert_arc(dge, dgd);
  dg.insert_arc(dgf, dge);


  print_graph(dg, "Digraph dg");

  auto dl = Kosaraju_compute_strong_connected_components(dg);
  
  i = 0;
  for (const DGT & dgg : dl)
    {
      stringstream s;
      s << "Component " << ++i;
      print_graph(dgg, s.str());
    }
  
  return 0;
}
