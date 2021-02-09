/*
  This file is part of Designar.
  
  Author: Alejandro Mujica (aledrums@gmail.com)
*/

# include <graphalgorithms.H>

using namespace std;
using namespace Designar;

using GT  = Graph<string, lint_t>;
using DGT = Digraph<string, lint_t>;

template <class Graph>
void write_graph(Graph & g, const char & name)
{
  string connector = "--";
  
  if (g.is_digraph())
    {
      cout << "di";
      connector = "->";
    }
  cout << "graph " << name << "\n";
  cout << g.get_num_nodes() << " nodes, " << g.get_num_arcs()
       << " arcs\n\n";

  cout << "Node set of " << name << "\n";
  g.for_each_node([&](auto n) { cout << n->get_info() << endl; });

  cout << "\nArc set of " << name << "\n";
  g.for_each_arc([&](auto a)
		 {
		   const string & s = a->get_src_node()->get_info();
		   const string & t = a->get_tgt_node()->get_info();
		   cout << s << connector << t << "(" << a->get_info()
			<< ")" << endl;
		 });

		 cout << "\n\n";
}

template <class Graph>
void write_node(Graph & g, typename Graph::Node * p)
{
  string connector = g.is_digraph() ? "->" : "--";
  
  g.for_each_adjacent_arc(p, [&] (auto a)
			  {
			    const string c =
			      a->get_connected_node(p)->get_info();
			    cout << p->get_info() << connector << c
				 << "(" << a->get_info() << ")" << endl;
			  });
}

using Dot  = DotGraph<GT>;
using DDot = DotGraph<DGT>;

int main()
{
  GT g, h, i;
  
  auto n1 = g.insert_node("Lara");
  auto n2 = g.insert_node("Portuguesa");
  auto n3 = g.insert_node("Barinas");
  auto n4 = g.insert_node("Merida");
  auto n5 = g.insert_node("Trujillo");
  auto n6 = g.insert_node("Zulia");

  g.insert_arc(n1, n2, 100);
  g.insert_arc(n2, n3, 150);
  g.insert_arc(n3, n4, 150);
  g.insert_arc(n4, n5, 180);
  g.insert_arc(n5, n6, 200);
  g.insert_arc(n1, n5, 300);
  
  write_node(g, n4);

  write_graph(g, 'g');
  write_graph(h, 'h');
  write_graph(i, 'i');

  h = g;

  write_graph(g, 'g');
  write_graph(h, 'h');
  write_graph(i, 'i');

  i = move(g);

  write_graph(g, 'g');
  write_graph(h, 'h');
  write_graph(i, 'i');
  
  cout << "Depth first traverse (prefix): ";
  
  depth_first_traverse(i, [] (auto p)
	      {
		cout << p->get_info() << " ";
	      });
  
  cout << endl << endl;

  cout << "Depth first traverse (suffix): ";
  
  depth_first_traverse_suffix(i, [] (auto p)
	      {
		cout << p->get_info() << " ";
	      });
  
  cout << endl << endl;

  Path<GT> path = depth_first_search_path(i, n1, n4);

  cout << "Path between " << n1->get_info() << " and " << n4->get_info() << ": ";

  path.for_each([] (auto c, auto r)
		{
		  cout << c->get_info() << " ";

		  if (r != nullptr)
		    cout << r->get_info() << " ";
		});

  cout << endl << endl;
  
  cout << "Breadth first traverse: ";
  
  breadth_first_traverse(i, [] (auto p)
			 {
			   cout << p->get_info() << " ";
			 });
  
  cout << endl << endl;

  path.clear();

  path = breadth_first_search_path(i, n1, n4);

  cout << "Path between " << n1->get_info() << " and " << n4->get_info() << ": ";

  path.for_each([] (auto c, auto r)
		{
		  cout << c->get_info() << " ";

		  if (r != nullptr)
		    cout << r->get_info() << " ";
		});

  cout << endl;
   
  Dot().write_graph(i, "graph.dot");
  
  GT Kruskal_tree = Kruskal<GT>().build_min_spanning_tree(i);
  Dot().write_graph(Kruskal_tree, "Kruskal.dot");

  GT Prim_tree = Prim<GT>().build_min_spanning_tree(i);
  Dot().write_graph(Prim_tree, "Prim.dot");

  GT Dijkstra_tree = Dijkstra<GT>().build_min_path_tree(i, n1);
  Dot().write_graph(Dijkstra_tree, "Dijkstra.dot");
  
  DGT dg, dh, di;

  auto dn1 = dg.insert_node("Lara");
  auto dn2 = dg.insert_node("Portuguesa");
  auto dn3 = dg.insert_node("Barinas");
  auto dn4 = dg.insert_node("Merida");
  auto dn5 = dg.insert_node("Trujillo");
  auto dn6 = dg.insert_node("Zulia");

  dg.insert_arc(dn1, dn2, 100);
  dg.insert_arc(dn2, dn3, 150);
  dg.insert_arc(dn3, dn4, 150);
  dg.insert_arc(dn4, dn5, 300);
  dg.insert_arc(dn5, dn6, 200);
  dg.insert_arc(dn1, dn5, 300);

  write_node(dg, dn4);
  
  write_graph(dg, 'g');
  write_graph(dh, 'h');
  write_graph(di, 'i');
  
  dh = dg;
  
  write_graph(dg, 'g');
  write_graph(dh, 'h');
  write_graph(di, 'i');
  
  di = move(dg);
  
  write_graph(dg, 'g');
  write_graph(dh, 'h');
  write_graph(di, 'i');
  
  cout << "Depth first traverse (prefix): "; 
  
  depth_first_traverse(di, [] (auto p)
	       {
		 cout << p->get_info() << " ";
	       });
  
  cout << endl << endl;

  cout << "Depth first traverse (suffix): ";
  
  depth_first_traverse_suffix(di, [] (auto p)
	       {
		 cout << p->get_info() << " ";
	       });
  
  cout << endl << endl;

  Path<DGT> dpath = depth_first_search_path(di, dn1, dn4);

  cout << "Path between " << dn1->get_info() << " and "
       << dn4->get_info() << ": ";

  dpath.for_each([] (auto c, auto r)
		 {
		   cout << c->get_info() << " ";
		   
		   if (r != nullptr)
		     cout << r->get_info() << " ";
		 });

  cout << endl << endl;
  
  cout << "Breadth first traverse: ";
  
  breadth_first_traverse(di, [] (auto p)
			 {
			   cout << p->get_info() << " ";
			 });

  cout << endl << endl;

  dpath.clear();

  dpath = breadth_first_search_path(di, dn1, dn4);

  cout << "Path between " << dn1->get_info() << " and "
       << dn4->get_info() << ": ";
  
  dpath.for_each([] (auto c, auto r)
		 {
		   cout << c->get_info() << " ";
		   
		   if (r != nullptr)
		     cout << r->get_info() << " ";
		 });
  
  cout << endl;

  assert(di.is_digraph());
  
  DDot().write_graph(di, "digraph.dot");
  
  auto bfres = BellmanFord<DGT>().build_min_path_tree(di, dn1);

  if (get<0>(bfres))
    DDot().write_graph(get<1>(bfres), "BellmanFord.dot");
 
  cout << "Everything ok\n";
  
  return 0;
}
