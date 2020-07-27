/*
  This file is part of Designar.
  
  Author: Alejandro Mujica (aledrums@gmail.com)
*/

# ifndef DSGRELATION_H
# define DSGRELATION_H

# include <map.H>

namespace Designar
{

  class EquivalenceRelation
  {
    FixedArray<nat_t> id;
    FixedArray<nat_t> sz;
    nat_t   num_blocks;

    nat_t find(nat_t) const;
    
  public:
    EquivalenceRelation(nat_t);

    void join(nat_t, nat_t);

    bool are_connected(nat_t, nat_t) const;

    nat_t get_num_blocks() const;

    nat_t size() const;
  };

  template <typename T, class Equal = std::equal_to<T>>
  class TRelation : public EquivalenceRelation
  {
    using Table = HashMap<T, nat_t, Equal>;

    mutable Table table;

    nat_t search_or_insert(const T & item) const
    {
      nat_t * result = table.search(item);

      if (result == nullptr)
	{
	  if (table.size() == EquivalenceRelation::size())
	    throw std::overflow_error("Cannot add a new item");
	  
	  result = table.insert(item, table.size());
	}

      return *result;
    }

    void init_from_list(const std::initializer_list<T> &);

  public:
    TRelation(nat_t n, Equal & eq)
      : EquivalenceRelation(n), table(n *1.25, eq)
    {
      // empty
    }

    TRelation(nat_t n, Equal && eq = Equal())
      : EquivalenceRelation(n), table(n *1.25, std::forward<Equal>(eq))
    {
      // empty
    }

    TRelation(const std::initializer_list<T> & list, Equal & eq)
      : EquivalenceRelation(list.size()), table(list.size() * 1.25, eq)
    {
      init_from_list(list);
    }

    TRelation(const std::initializer_list<T> & list, Equal && eq = Equal())
      : EquivalenceRelation(list.size()),
	table(list.size() * 1.25, std::forward<Equal>(eq))
    {
      init_from_list(list);
    }
    
    void join(const T & tp, const T & tq)
    {
      nat_t p = search_or_insert(tp);
      nat_t q = search_or_insert(tq);
      EquivalenceRelation::join(p, q);
    }

    
    bool are_connected(const T & tp, const T & tq) const
    {
      nat_t p, q;
      
      try
	{
	  p = search_or_insert(tp);
	  q = search_or_insert(tq);
	}
      catch (...)
	{
	  throw std::domain_error("Both elements must belong to relation");
	}

      return EquivalenceRelation::are_connected(p, q);
    }

    
  };

  template <typename T, class Equal>
  void TRelation<T, Equal>::init_from_list(const std::initializer_list<T> & l)
  {
    nat_t i = 0;
    
    for (const T & item : l)
      table.insert(item, i++);
  }
    
} // end namespace Designar

# endif // DSGRELATION_H
