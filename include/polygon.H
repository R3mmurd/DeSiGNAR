/*
  This file is part of Designar Library.
  Copyright (C) 2017 by Alejandro J. Mujica

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.

  Any user request of this software, write to 

  Alejandro Mujica

  aledrums@gmail.com
*/

# ifndef DSGPOLYGON_H
# define DSGPOLYGON_H

# include <segment.H>
# include <nodesdef.H>

namespace Designar
{

  template <typename PointT>
  class GenPolygon
  {
    using Vertex = DLNode<PointT>;

    static Vertex * dl_to_vertex(DL *& v)
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

    bool is_empty() const
    {
      return num_vertices == 0;
    }

    nat_t size() const
    {
      return num_vertices;
    }

    class VertexIterator : public DL::Iterator
    {
      using Base = DL::Iterator;

      GenPolygon * p_ptr;

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
	return dl_to_node(Base::get_current())->get_item();
      }

      const PointT & get_current() const
      {
	return dl_to_node(Base::get_current())->get_item();
      }

      void del()
      {
	if (not Base::has_current())
	  throw std::overflow_error("There is not current element");

	Vertex * v = dl_to_node(Base::get_current());
	Base::next();
	--p_ptr->num_vertices;
	v->del();
	delete v;
      }

      PointT & operator * ()
      {
	return get_current();
      }

      const PointT & operator * () const
      {
	return get_current();
      }
    };

    class SegmentIterator
    {
      GenPolygon * p_ptr;
      Vertex     * head;
      Vertex     * curr;

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
	  throw std::overflow_error("There is not next element");

	curr = curr->get_next();
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

      SegmentType operator * () const
      {
	return get_current();
      }

      void operator ++ ()
      {
	next();
      }

      bool operator == (const SegmentIterator & it)
      {
	return head == it.head and curr == it.curr;
      }

      bool operator != (const SegmentIterator & it)
      {
	return not (*this == it);
      }
    };

    VertexIterator begin_vertices()
    {
      return VertexIterator(*this);      
    }

    VertexIterator end_vertices()
    {
      VertexIterator it(*this);

      if (num_vertices == 0)
	return it;
      
      it.reset_last();
      it.next();
      return it;
    }

    SegmentIterator begin_segments()
    {
      return SegmentIterator(*this);
    }

    SegmentIterator end_segments()
    {
      VertexIterator it(*this);

      it.reset_last();
      it.next();
      return it;
    }
  };

  template <class PointT>
  void GenPolygon<PointT>::copy(const GenPolygon & p)
  {
    Vertex * v = dl_to_vertex(p.vertex_list.get_next());

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
