/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <cassert>
#include <iostream>
#include <fstream>

using namespace std;

#include <buildgraph.hpp>

using namespace Designar;

using GT = Graph<int, int, int>;
using DGT = Digraph<int, int, int>;

template <typename G>
bool graph_equal(const G& g1, const G& g2)
{
  if (g1.get_info() != g2.get_info())
    return false;

  if ((g1.get_num_nodes() != g2.get_num_nodes()) or
      (g1.get_num_arcs() != g2.get_num_arcs()))
    return false;

  auto cmp_nodes = [](auto p, auto q)
  {
    return p->get_info() < q->get_info();
  };

  auto cmp_arcs = [&cmp_nodes](auto a1, auto a2)
  {
    if (cmp_nodes(a1->get_src_node(), a2->get_src_node()))
      return true;
    else if (cmp_nodes(a2->get_src_node(), a1->get_src_node()))
      return false;

    if (cmp_nodes(a1->get_tgt_node(), a2->get_tgt_node()))
      return true;
    else if (cmp_nodes(a2->get_tgt_node(), a1->get_tgt_node()))
      return false;

    return a1->get_info() < a2->get_info();
  };

  auto n1 = sort(g1.nodes(), cmp_nodes);
  auto a1 = sort(g1.arcs(), cmp_arcs);

  auto n2 = sort(g2.nodes(), cmp_nodes);
  auto a2 = sort(g2.arcs(), cmp_arcs);

  bool rn = n1.equal(n2, [](auto p, auto q)
                     { return p->get_info() == q->get_info(); });

  bool ra = a1.equal(a2, [](auto a, auto b)
                     { return (a->get_src_node()->get_info() == b->get_src_node()->get_info()) && (a->get_tgt_node()->get_info() == b->get_tgt_node()->get_info()) && (a->get_info() == b->get_info()); });

  return rn && ra;
}

int main()
{
  nat_t nc = 0;
  nat_t ac = 0;

  GT g1 = er_random_graph<GT>(
      100, 0.7, false,
      [&nc](auto& p)
      {
        p->get_info() = ++nc;
      },
      [&ac](auto& a)
      {
        a->get_info() = ++ac;
      });

  OutputGraph<GT> outg;
  InputGraph<GT> ing;

  ofstream out;
  ifstream in;

  out.open("tmp-g1-txt.dsgg");
  outg.write_in_text_mode(g1, out);
  out.close();
  in.open("tmp-g1-txt.dsgg");
  assert(in);

  GT g2 = ing.read_in_text_mode(in);
  in.close();
  remove("tmp-g1-txt.dsgg");
  assert(graph_equal(g1, g2));

  // write_in_bin_mode()/read_in_bin_mode() write/read raw struct bytes
  // (ofstream::write/ifstream::read on POD data), so the stream must be
  // opened with ios::binary: without it, Windows' text-mode translation
  // rewrites embedded 0x0A bytes to 0x0D 0x0A on write (corrupting the
  // byte stream) and treats an embedded 0x1A byte as an early EOF on
  // read — either of which can desync the reader from the writer's byte
  // layout badly enough to hang rather than just fail an assert.
  out.open("tmp-g1-bin.dsgg", ios::binary);
  outg.write_in_bin_mode(g1, out);
  out.close();
  in.open("tmp-g1-bin.dsgg", ios::binary);
  assert(in);

  GT g3 = ing.read_in_bin_mode(in);
  in.close();
  remove("tmp-g1-bin.dsgg");
  assert(graph_equal(g1, g3));

  nc = 0;
  ac = 0;

  DGT dg1 = er_random_graph<DGT>(
      100, 0.7, false,
      [&nc](auto& p)
      {
        p->get_info() = ++nc;
      },
      [&ac](auto& a)
      {
        a->get_info() = ++ac;
      });

  OutputGraph<DGT> outdg;
  InputGraph<DGT> indg;

  out.open("tmp-dg1-txt.dsgg");
  outdg.write_in_text_mode(dg1, out);
  out.close();
  in.open("tmp-dg1-txt.dsgg");
  assert(in);

  DGT dg2 = indg.read_in_text_mode(in);
  in.close();
  remove("tmp-dg1-txt.dsgg");
  assert(graph_equal(dg1, dg2));

  // @see the ios::binary note above the first write_in_bin_mode() call.
  out.open("tmp-dg1-bin.dsgg", ios::binary);
  outdg.write_in_bin_mode(dg1, out);
  out.close();
  in.open("tmp-dg1-bin.dsgg", ios::binary);
  assert(in);

  DGT dg3 = indg.read_in_bin_mode(in);
  in.close();
  remove("tmp-dg1-bin.dsgg");
  assert(graph_equal(dg1, dg3));

  cout << "Everything ok!\n";
  return 0;
}
