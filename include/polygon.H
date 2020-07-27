/*
  This file is part of Designar.
  
  Author: Alejandro Mujica (aledrums@gmail.com)
*/

# ifndef DSGPOLYGON_H
# define DSGPOLYGON_H

# include <segment.H>
# include <nodesdef.H>
# include <italgorithms.H>

namespace Designar
{

  template <typename PointT>
  class GenPolygon
  {
    using Vertex = DLNode<PointT>;

    static Vertex * dl_to_vertex(DL * v)
    {
      return static_cast<Vertex *>(v);
    }

    static Vertex * to_vertex(PointT & p)
    {
      Vertex * zero          = 0;
      nat_t    off_set       = (nat_t) &zero->get_item();
      nat_t    point_address = (nat_t) &p;
      return (Vertex *) (point_address - off_set); 
    }

    void copy(const GenPolygon &);

    nat_t num_vertices;
    DL    vertex_list;

  public:
    using PointType   = PointT;
    using SegmentType = GenSegment<PointT>;

    GenPolygon()
      : num_vertices(0), vertex_list()
    {
      // empty
    }

    GenPolygon(const std::initializer_list<PointT> &);

    GenPolygon(const GenPolygon & p)
      : GenPolygon()
    {
      copy(p);
    }

    GenPolygon(GenPolygon && p)
      : GenPolygon()
    {
      swap(p);
    }

    ~GenPolygon()
    {
      clear();
    }

    GenPolygon & operator = (const GenPolygon & p)
    {
      if (this == &p)
	return *this;

      clear();
      copy(p);

      return *this;
    }

    GenPolygon & operator = (GenPolygon && p)
    {
      swap(p);
      return *this;
    }
    
    void swap(GenPolygon & p)
    {
      std::swap(num_vertices, p.num_vertices);
      vertex_list.swap(p.vertex_list);
    }

    void clear();

    void add_vertex(const PointT & p)
    {
      vertex_list.insert_prev(new Vertex(p));
      ++num_vertices;
    }

    void add_vertex(PointT && p)
    {
      vertex_list.insert_prev(new Vertex(std::forward<PointT>(p)));
      ++num_vertices;
    }

    PointT & get_first_vertex()
    {
      if (num_vertices == 0)
	throw std::length_error("Polygon has not vertices");
      
      return dl_to_vertex(vertex_list.get_next())->get_item();
    }

    const PointT & get_first_vertex() const
    {
      if (num_vertices == 0)
	throw std::length_error("Polygon has not vertices");
      
      return dl_to_vertex(const_cast<DL *>(vertex_list.get_next()))->get_item();
    }

    PointT & get_last_vertex()
    {
      if (num_vertices == 0)
	throw std::length_error("Polygon has not vertices");
      
      return dl_to_vertex(vertex_list.get_prev())->get_item();
    }

    const PointT & get_last_vertex() const
    {
      if (num_vertices == 0)
	throw std::length_error("Polygon has not vertices");
      
      return dl_to_vertex(const_cast<DL *>(vertex_list.get_prev()))->get_item();
    }

    SegmentType get_first_segment() const
    {
      if (num_vertices < 2)
	throw std::length_error("Polygon has not segments");

      const PointT & s =
	dl_to_vertex(const_cast<DL *>(vertex_list.get_next()))->get_item();
      const PointT & t =
	dl_to_vertex(const_cast<DL *>(vertex_list.get_next()->get_next()))
	->get_item();
      return SegmentType(s, t);
    }

    SegmentType get_last_segment() const
    {
      if (num_vertices < 2)
	throw std::length_error("Polygon has not segments");

      const PointT & s =
	dl_to_vertex(const_cast<DL *>(vertex_list.get_prev())->get_prev())
	->get_item();
      const PointT & t =
	dl_to_vertex(const_cast<DL *>(vertex_list.get_prev()))->get_item();
      
      return SegmentType(s, t);
    }

    bool is_empty() const
    {
      return num_vertices == 0;
    }

    nat_t size() const
    {
      return num_vertices;
    }

    class VertexIterator : public DL::Iterator,
			   public BidirectionalIterator<VertexIterator, PointT>
    {
      friend class BasicIterator<VertexIterator, PointT>;
      
      using Base = DL::Iterator;

      GenPolygon * p_ptr;

    public:
      VertexIterator()
	: Base(), p_ptr(nullptr)
      {
	// empty
      }

      VertexIterator(const GenPolygon & p)
	: Base(const_cast<DL *>(&p.vertex_list)),
	  p_ptr(const_cast<GenPolygon *>(&p))
      {
	// empty
      }

      VertexIterator(const GenPolygon & p, DL * curr)
	: Base(const_cast<DL *>(&p.vertex_list), curr),
	  p_ptr(const_cast<GenPolygon *>(&p))
      {
	// empty
      }

      VertexIterator(const VertexIterator & it)
	: Base(it), p_ptr(it.p_ptr)
      {
	// empty
      }

      VertexIterator(VertexIterator && it)
	: VertexIterator()
      {
	swap(it);
      }

      VertexIterator & operator = (const VertexIterator & it)
      {
	if (this == &it)
	  return *this;

	(Base &) *this = it;
	p_ptr = it.p_ptr;
	
	return *this;
      }

      VertexIterator & operator = (VertexIterator && it)
      {
	swap(it);
      }

      void swap(VertexIterator & it)
      {
	Base::swap(it);
	std::swap(p_ptr, it.p_ptr);
      }

      PointT & get_current()
      {
	return dl_to_vertex(Base::get_current())->get_item();
      }

      const PointT & get_current() const
      {
	return dl_to_vertex(Base::get_current())->get_item();
      }

      void del()
      {
	if (not Base::has_current())
	  throw std::overflow_error("There is not current element");

	Vertex * v = dl_to_vertex(Base::get_current());
	Base::next();
	--p_ptr->num_vertices;
	v->del();
	delete v;
      }
    };

    class SegmentIterator
      : public BidirectionalIterator<SegmentIterator, SegmentType, true>
    {
      friend class BasicIterator<SegmentIterator, SegmentType, true>;
      
      GenPolygon * p_ptr;
      Vertex     * head;
      Vertex     * curr;

    protected:
      Vertex * get_location() const
      {
	return curr;
      }
      
    public:
      SegmentIterator()
	: head(nullptr), curr(nullptr)
      {
	// empty
      }

      SegmentIterator(const GenPolygon & p)
	: head(dl_to_vertex(const_cast<DL *>(&p.vertex_list))),
	  curr(head->get_next())
      {
	if (p.size() < 2)
	  throw std::length_error("There are no segments in polygon");
      }

      SegmentIterator(const GenPolygon & p, DL * c)
	: head(dl_to_vertex(const_cast<DL *>(&p.vertex_list))),
	  curr(dl_to_vertex(c))
      {
	if (p.size() < 2)
	  throw std::length_error("There are no segments in polygon");
      }

      SegmentIterator(const SegmentIterator & it)
	: head(it.head), curr(it.curr)
      {
	// empty
      }

      SegmentIterator(SegmentIterator && it)
	: SegmentIterator()
      {
	swap(it);
      }

      void swap(SegmentIterator & it)
      {
	std::swap(head, it.head);
	std::swap(curr, it.curr);
      }

      bool has_current() const
      {
	return head != curr;
      }

      SegmentType get_current()
      {
	if (not has_current())
	  throw std::overflow_error("There is not current element");

	Vertex * next = curr->get_next() == head ?
	  head->get_next() : curr->get_next();

	return SegmentType(curr->get_item(), next->get_item());
      }

      SegmentType get_current() const
      {
	if (not has_current())
	  throw std::overflow_error("There is not current element");

	Vertex * next = curr->get_next() == head ?
	  head->get_next() : curr->get_next();

	return SegmentType(curr->get_item(), next->get_item());
      }

      void next()
      {
	if (not has_current())
	  return;

	curr = curr->get_next();
      }

      void prev()
      {
	if (curr == head->get_next())
	  return;

	curr = curr->get_prev();
      }

      void del()
      {
	if (not has_current())
	  throw std::overflow_error("There is not current element");

	Vertex * c = curr;
	Vertex * n = nullptr;

	if (c->get_next() == head)
	  {
	    n = head->get_next();
	    curr = head;
	  }
	else
	  {
	    next();
	    n = curr;
	    next();
	  }

	p_ptr->num_vertices -= 2;

	c->del();
	n->del();

	delete c;
	delete n;
      }

      void reset_first()
      {
	curr = head->get_next();
      }

      void reset_last()
      {
	curr = head->get_prev();
      }
    };

    VertexIterator vertices_begin()
    {
      return VertexIterator(*this);
    }

    VertexIterator vertices_begin() const
    {
      return VertexIterator(*this);
    }

    VertexIterator vertices_end()
    {
      return VertexIterator(*this, &vertex_list);
    }

    VertexIterator vertices_end() const
    {
      return VertexIterator(*this, const_cast<DL *>(&vertex_list));
   } 

    SegmentIterator segments_begin()
    {
      return SegmentIterator(*this);
    }

    SegmentIterator segments_begin() const
    {
      return SegmentIterator(*this);
    }

    SegmentIterator segments_end()
    {
      return SegmentIterator(*this, &vertex_list);
    }

    SegmentIterator segments_end() const
    {
      return SegmentIterator(*this, const_cast<DL *>(&vertex_list));
    }

    template <class Op>
    void for_each_vertex(Op & op) const
    {
      for_each_it(vertices_begin(), vertices_end(), op);
    }

    template <class Op>
    void for_each_vertex(Op && op = Op()) const
    {
      for_each_it(vertices_begin(), vertices_end(), std::forward<Op>(op));
    }

    template <class Pred>
    bool all_vertex(Pred & pred) const
    {
      return all_it(vertices_begin(), vertices_end(), pred);
    }

    template <class Pred>
    bool all_vertex(Pred && pred = Pred()) const
    {
      return all_it(vertices_begin(), vertices_end(), std::forward<Pred>(pred));
    }

    template <class Pred>
    bool exists_vertex(Pred & pred) const
    {
      return exists_it(vertices_begin(), vertices_end(), pred);
    }

    template <class Pred>
    bool exists_vertex(Pred && pred = Pred()) const
    {
      return exists_it(vertices_begin(), vertices_end(),
		       std::forward<Pred>(pred));
    }

    template <class Pred>
    bool none_vertex(Pred & pred) const
    {
      return none_it(vertices_begin(), vertices_end(), pred);
    }

    template <class Pred>
    bool none_vertex(Pred && pred = Pred()) const
    {
      return none_it(vertices_begin(), vertices_end(),
		     std::forward<Pred>(pred));
    }
    
    template <class Op>
    void for_each_segment(Op & op) const
    {
      for_each_it(segments_begin(), segments_end(), op);
    }

    template <class Op>
    void for_each_segment(Op && op = Op()) const
    {
      for_each_it(segments_begin(), segments_end(), std::forward<Op>(op));
    }

    template <class Pred>
    bool all_segment(Pred & pred) const
    {
      return all_it(segments_begin(), segments_end(), pred);
    }

    template <class Pred>
    bool all_segment(Pred && pred = Pred()) const
    {
      return all_it(segments_begin(), segments_end(), std::forward<Pred>(pred));
    }

    template <class Pred>
    bool exists_segment(Pred & pred) const
    {
      return exists_it(segments_begin(), segments_end(), pred);
    }

    template <class Pred>
    bool exists_segment(Pred && pred = Pred()) const
    {
      return exists_it(segments_begin(), segments_end(),
		       std::forward<Pred>(pred));
    }

    template <class Pred>
    bool none_segment(Pred & pred) const
    {
      return none_it(segments_begin(), segments_end(), pred);
    }

    template <class Pred>
    bool none_segment(Pred && pred = Pred()) const
    {
      return none_it(segments_begin(), segments_end(),
		     std::forward<Pred>(pred));
    }
  };

  template <class PointT>
  void GenPolygon<PointT>::copy(const GenPolygon & p)
  {
    Vertex * v = dl_to_vertex(const_cast<DL *>(p.vertex_list.get_next()));

    while (v != &p.vertex_list)
      {
	vertex_list.insert_prev(new Vertex(v->get_item()));
	v = v->get_next();
	++num_vertices;
      }
  }

  template <class PointT>
  GenPolygon<PointT>::GenPolygon(const std::initializer_list<PointT> & l)
    : GenPolygon()
  {
    for (const PointT & p : l)
      add_vertex(p);
  }

  template <class PointT>
  void GenPolygon<PointT>::clear()
  {
    while (not vertex_list.is_empty())
      delete vertex_list.remove_next();

    num_vertices = 0;
  }

  class PolygonInt : public GenPolygon<PointInt2D>
  {
    using Base = GenPolygon<PointInt2D>;
    using Base::Base;
  };

  class Polygon : public GenPolygon<Point2D>
  {
    using Base = GenPolygon<Point2D>;
    using Base::Base;
  };
  
} // end namespace Designar

# endif // DSGPOLYGON_H
