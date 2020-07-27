/*
  This file is part of Designar.
  
  Author: Alejandro Mujica (aledrums@gmail.com)
*/

# include <relation.H>

namespace Designar
{

  nat_t EquivalenceRelation::find(nat_t p) const
  {
    while (p != id[p])
      p = id[p];

    return p;
  }

  EquivalenceRelation::EquivalenceRelation(nat_t n)
    : id(n), sz(n), num_blocks(n)
  {
    for (nat_t i = 0; i < n; ++i)
      {
	id[i] = i;
	sz[i] = 1;
      }
  }
  
  void EquivalenceRelation::join(nat_t p, nat_t q)
  {
    nat_t i = find(p);
    nat_t j = find(q);

    if (i == j)
      return;

    if (sz[i] < sz[j])
      {
	id[i] = j;
	sz[j] += sz[i];
      }
    else
      {
	id[j] = i;
	sz[i] += sz[j];
      }

    --num_blocks;
  }
  
  bool EquivalenceRelation::are_connected(nat_t p, nat_t q) const
  {
    return find(p) == find(q);
  }
  
  nat_t EquivalenceRelation::get_num_blocks() const
  {
    return num_blocks;
  }

  nat_t EquivalenceRelation::size() const
  {
    return id.size();
  }
  
} // end namespace Designar
