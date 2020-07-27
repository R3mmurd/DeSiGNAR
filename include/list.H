/*
  This file is part of Designar.
  
  Author: Alejandro Mujica (aledrums@gmail.com)
*/

# ifndef DSGLIST_H
# define DSGLIST_H

# include <nodesdef.H>
# include <containeralgorithms.H>

namespace Designar
{

  template <typename T>
  class NodeSLList
  {
  public:
    using Node = SLNode<T>;

  private:
    Node * head;
    Node * tail;

    void add_to_empty_list(Node * node)
    {
      assert(head == nullptr);
      assert(tail == nullptr);
      head = tail = node;
    }

  public:
    NodeSLList()
      : head(nullptr), tail(nullptr)
    {
      // empty
    }

    NodeSLList(const NodeSLList &) = delete;

    NodeSLList(NodeSLList && l)
      : NodeSLList()
    {
      swap(l);
    }

    NodeSLList & operator = (const NodeSLList &) = delete;

    NodeSLList & operator = (NodeSLList && l)
    {
      swap(l);
      return *this;
    }

    void swap(NodeSLList & l)
    {
      std::swap(head, l.head);
      std::swap(tail, l.tail);
    }

    bool is_empty() const
    {
      return head == nullptr;
    }

    bool is_unitarian_or_empty() const
    {
      return head == tail;
    }

    bool is_unitarian() const
    {
      return head == tail and head != nullptr;
    }
    
    void insert(Node * node)
    {
      if (head == nullptr)
	add_to_empty_list(node);
      else
	{
	  node->get_next() = head;
	  head = node;
	}
    }

    void append(Node * node)
    {
      if (head == nullptr)
	add_to_empty_list(node);
      else
	{
	  tail->insert_next(node);
	  tail = node;
	}
    }

    Node *& get_first()
    {
      return head;
    }

    const Node *& get_first() const
    {
      return (const Node *&) head;
    }

    Node *& get_last()
    {
      return tail;
    }

    const Node *& get_last() const
    {
      return (const Node *&) tail;
    }

    Node * remove_first()
    {
      if (head == nullptr)
	return nullptr;

      Node * ret_val = head;
      head = ret_val->get_next();
      ret_val->reset();

      if (head == nullptr)
	tail = nullptr;

      return ret_val;
    }

    void concat(NodeSLList * l)
    {
      if (l->is_empty())
	return;
      
      if (this->is_empty())
	{
	  this->head = l->head;
	  this->tail = l->tail;
	}
      else
	{
	  this->tail->get_next() = l->head;
	  this->tail = l->tail;
	}

      l->head = l->tail = nullptr;
    }

    void concat(NodeSLList & l)
    {
      concat(&l);
    }

    void split(NodeSLList &, NodeSLList &);
  };

  template <typename T>
  void NodeSLList<T>::split(NodeSLList & l, NodeSLList & r)
  {
    assert(l.is_empty());
    assert(r.is_empty());
    
    while (not this->is_empty())
      {
	l->append(this->remove_first());

	if (not l->is_empty())
	  r->append(this->remove_first());
      }
  }
  
  template <typename T>
  class SLList : public NodeSLList<T>,
		 public ContainerAlgorithms<SLList<T>, T>
  {
    using Node     = SLNode<T>;
    using BaseList = NodeSLList<T>;
    
    nat_t num_items;

    void copy_list(const SLList &);
    
    void empty_list();
    
  public:
    using ItemType  = T;
    using KeyType   = T;
    using DataType  = T;
    using ValueType = T;
    using SizeType  = nat_t;

    SLList()
      : BaseList(), num_items(0)
    {
      // empty
    }

    SLList(const SLList & l)
      : SLList()
    {
      copy_list(l);
    }

    SLList(SLList && l)
      : SLList()
    {
      this->swap(l);
    }

    SLList(const std::initializer_list<T> &);

    ~SLList()
    {
      clear();
    }

    nat_t size() const
    {
      return num_items;
    }

    void clear()
    {
      empty_list();
    }

    T & insert(const T & item)
    {
      Node * node = new Node(item);
      BaseList::insert(node);
      ++num_items;
      return node->get_item();
    }

    T & insert(T && item)
    {
      Node * node = new Node(std::forward<T>(item));
      BaseList::insert(node);
      ++num_items;
      return node->get_item();
    }

    T & append(const T & item)
    {
      Node * node = new Node(item);
      BaseList::append(node);
      ++num_items;
      return node->get_item();
    }

    T & append(T && item)
    {
      Node * node = new Node(std::forward<T>(item));
      BaseList::append(node);
      ++num_items;
      return node->get_item();
    }

    T & get_first()
    {
      if (num_items == 0)
	throw std::underflow_error("List is empty");
      
      return BaseList::get_first()->get_item();
    }

    const T & get_first() const
    {
      if (num_items == 0)
	throw std::underflow_error("List is empty");
      
      return BaseList::get_first()->get_item();
    }

    T & get_last()
    {
      if (num_items == 0)
	throw std::underflow_error("List is empty");
	  
      return BaseList::get_last()->get_item();
    }

    const T & get_last() const
    {
      if (num_items == 0)
	throw std::underflow_error("There is not element in list");
      
      return BaseList::get_last()->get_item();
    }

    T remove_first()
    {
      if (num_items == 0)
	throw std::underflow_error("There is not element in list");

      Node * p = BaseList::remove_first();
      T ret_val = std::move(p->get_item());
      delete p;
      --num_items;
      return ret_val;
    }

    void swap(SLList & l)
    {
      BaseList::swap(l);
      std::swap(num_items, l.num_items);
    }

    void concat(SLList & l)
    {
      num_items += l.num_items;
      BaseList::concat(l);
      l.num_items = 0;
    }

    SLList & operator = (const SLList & l)
    {
      if (this == &l)
	return *this;

      clear();
      copy_list(l);
      return *this;
    }

    SLList & operator = (SLList && l)
    {
      swap(l);
      return *this;
    }

    class Iterator : public ForwardIterator<Iterator, T>
    {
      friend class BasicIterator<Iterator, T>;
      
      SLList * list_ptr;
      Node   * curr;
      Node   * pred;

    protected:
      Node * get_location() const
      {
	return curr;
      }
      
    public:
      Iterator()
	: list_ptr(nullptr), curr(nullptr), pred(nullptr)
      {
	// empty
      }
      
      Iterator(const SLList<T> & l)
	: list_ptr(&const_cast<SLList<T> &>(l)),
	  curr(((BaseList &)l).get_first()), pred(nullptr)
      {
	// empty
      }

      Iterator(const SLList<T> & l, Node * c)
	: list_ptr(&const_cast<SLList<T> &>(l)), curr(c), pred(nullptr)
      {
	// empty
      }

      Iterator(const Iterator & it)
	: list_ptr(it.list_ptr), curr(it.curr), pred(it.pred)
      {
	// empty
      }

      Iterator(Iterator && it)
	: Iterator()
      {
	swap(it);
      }

      Iterator & operator = (const Iterator & it)
      {
	if (this == &it)
	  return *this;

	list_ptr = it.list_ptr;
	curr = it.curr;
	pred = it.pred;

	return *this;
      }

      Iterator & operator = (Iterator && it)
      {
	swap(it);
	return *this;
      }

      void swap(Iterator & it)
      {
	std::swap(list_ptr, it.list_ptr);
	std::swap(curr, it.curr);
	std::swap(pred, it.pred);
      }
      
      bool has_current() const
      {
	return curr != nullptr;
      }

      T & get_current()
      {
	if (not has_current())
	  throw std::overflow_error("There is not current element");
	
	return curr->get_item();
      }

      const T & get_current() const
      {
	if (not has_current())
	  throw std::overflow_error("There is not current element");
	
	return curr->get_item();
      }

      void next()
      {
	if (not has_current())
	  return;

	pred = curr;
	curr = curr->get_next();
      }

      void reset_first()
      {
	curr = ((BaseList *) list_ptr)->get_first();
      }

      T del()
      {
	if (not has_current())
	  throw std::logic_error("There is not current element");
	
	if (pred == nullptr)
	  {
	    T ret_val = list_ptr->remove_first();
	    curr = ((BaseList *) list_ptr)->get_first();
	    return ret_val;
	  }

	curr = curr->get_next();

	if (curr == nullptr)
	  ((BaseList *) list_ptr)->get_last() = pred;

	Node * p = pred->remove_next();
	T ret_val = std::move(p->get_item());
	delete p;
	--list_ptr->num_items;
	return ret_val;
      }
    };

    Iterator begin()
    {
      return Iterator(*this);
    }

    Iterator begin() const
    {
      return Iterator(*this);
    }

    Iterator end()
    {
      return Iterator(*this, nullptr);
    }

    Iterator end() const
    {
      return Iterator(*this, nullptr);
    }

    T & select(nat_t);

    const T & select(nat_t) const;

    T & operator [] (nat_t i)
    {
      return select(i);
    }

    const T & operator [] (nat_t i) const
    {
      return select(i);
    }
  };

  template <typename T>
  SLList<T>::SLList(const std::initializer_list<T> & l)
    : SLList()
  {
    for (const T & item : l)
      append(item);
  }

  template <typename T>
  void SLList<T>::copy_list(const SLList & l)
  {
    Node * n = ((BaseList &) l).get_first();
    
    while (n != nullptr)
      {
	append(n->get_item());
	n = n->get_next();
      }
  }
  
  template <typename T>
  void SLList<T>::empty_list()
  {
    while (not this->is_empty())
      remove_first();
  }

  template <typename T>
  T & SLList<T>::select(nat_t i)
  {
    for (T & item : *this)
      {
	if (i-- == 0)
	  return item;
      }
    throw std::out_of_range("Index out of range");
  }

  template <typename T>
  const T & SLList<T>::select(nat_t i) const
  {
    for (const T & item : *this)
      {
	if (i-- == 0)
	  return item;
      }
    throw std::out_of_range("Index out of range");
  }

  template <typename T>
  class DLList : public DL,
		 public ContainerAlgorithms<DLList<T>, T>
  {
    using Base = DL;
    using Node = DLNode<T>;
    
    nat_t num_items;
    
    void copy_list(const DLList &);
    
    void empty_list();

  public:
    using ItemType  = T;
    using KeyType   = T;
    using DataType  = T;
    using ValueType = T;
    using SizeType  = nat_t;
    
    DLList()
      : Base(), num_items(0)
    {
      // empty
    }
    
    DLList(const DLList & l)
      : DLList()
    {
      copy_list(l);
    }
    
    DLList(DLList && l)
      : DLList()
    {
      swap(l);
    }

    DLList(const std::initializer_list<T> &);

    ~DLList()
    {
      clear();
    }

    nat_t size() const
    {
      return num_items;
    }

    void clear()
    {
      empty_list();
    }

    void swap(DLList & l)
    {
      std::swap(num_items, l.num_items);
      Base::swap(&l);
    }

    T & insert(const T & item)
    {
      Node * node = new Node(item);
      Base::insert_next(node);
      ++num_items;
      return node->get_item();
    }

    T & insert(T && item)
    {
      Node * node = new Node(std::forward<T>(item));
      Base::insert_next(node);
      ++num_items;
      return node->get_item();
    }

    T & append(const T & item)
    {
      Node * node = new Node(item);
      Base::insert_prev(node);
      ++num_items;
      return node->get_item();
    }

    T & append(T && item)
    {
      Node * node = new Node(std::forward<T>(item));
      Base::insert_prev(node);
      ++num_items;
      return node->get_item();
    }

    T & get_first()
    {
      if (Base::is_empty())
	throw std::underflow_error("List is empty");

      return static_cast<Node *>(DL::get_next())->get_item();
    }
    
    const T & get_first() const
    {
      if (Base::is_empty())
	throw std::underflow_error("List is empty");

      return static_cast<const Node *>(DL::get_next())->get_item();
    }

    T & get_last()
    {
      if (Base::is_empty())
	throw std::underflow_error("List is empty");

      return static_cast<Node *>(DL::get_prev())->get_item();
    }
    
    const T & get_last() const
    {
      if (Base::is_empty())
	throw std::underflow_error("List is empty");

      return static_cast<const Node *>(DL::get_prev())->get_item();
    }

    T remove_first()
    {
      if (Base::is_empty())
	throw std::underflow_error("List is empty");
      
      Node * node = static_cast<Node *>(DL::remove_next());
      T ret_val = std::move(node->get_item());
      delete node;
      --num_items;
      return ret_val;
    }

    T remove_last()
    {
      if (Base::is_empty())
	throw std::underflow_error("List is empty");
      
      Node * node = static_cast<Node *>(DL::remove_prev());;
      T ret_val = std::move(node->get_item());
      delete node;
      --num_items;
      return ret_val;
    }

    void remove(T & item)
    {
      Node * zero = 0;
      nat_t off_set = nat_t(&zero->item);
      Node * item_node = (Node *) (nat_t(&item) - off_set);
      item_node->del();
      delete item_node;
      --num_items;
    }

    DLList & operator = (const DLList & l)
    {
      if (this == &l)
	return *this;

      clear();
      copy_list(l);
      return *this;
    }

    DLList & operator = (DLList && l)
    {
      swap(l);
      return *this;
    }

    class Iterator : public DL::Iterator,
		     public BidirectionalIterator<Iterator, T>
    {
      friend class BasicIterator<Iterator, T>;
      
      using Base = DL::Iterator;
      
      DLList * list_ptr;
    public:
      Iterator()
	: Base(), list_ptr(nullptr)
      {
	// empty
      }
      
      Iterator(const DLList & l)
	: Base(&const_cast<DLList &>(l)), list_ptr(&const_cast<DLList &>(l))
      {
	// empty
      }
      
      Iterator(const DLList & l, DL * c)
	: Base(&const_cast<DLList &>(l), c), list_ptr(&const_cast<DLList &>(l))
      {
	// empty
      }

      Iterator(const Iterator & it)
	: Base(it), list_ptr(it.list_ptr)
      {
	// empty
      }

      Iterator(Iterator && it)
	: Iterator()
      {
	swap(it);
      }

      Iterator & operator = (const Iterator & it)
      {
	if (this == &it)
	  return *this;

	(Base &) *this = it;
	list_ptr = it.list_ptr;
	return *this;
      }

      Iterator & operator = (Iterator && it)
      {
	swap(it);
	return *this;
      }

      void swap(Iterator & it)
      {
	Base::swap(it);
	std::swap(list_ptr, it.list_ptr);
      }

      T & get_current()
      {
	return static_cast<Node *>(Base::get_current())->get_item();
      }

      const T & get_current() const
      {
	return static_cast<Node *>(Base::get_current())->get_item();
      }

      T del()
      {
	Node * p = static_cast<Node *>(Base::del());
	T ret_val = std::move(p->get_item());
	delete p;
	--list_ptr->num_items;
	return ret_val;
      }
    };

    Iterator begin()
    {
      return Iterator(*this);
    }

    Iterator begin() const
    {
      return Iterator(*this);
    }

    Iterator end()
    {
      return Iterator(*this, this);
    }

    Iterator end() const
    {
      return Iterator(*this, const_cast<DLList *>(this));
    }

    T & select(nat_t);

    const T & select(nat_t) const;

    T & operator [] (nat_t i)
    {
      return select(i);
    }

    const T & operator [] (nat_t i) const
    {
      return select(i);
    }
  };

  template <typename T>
  DLList<T>::DLList(const std::initializer_list<T> & l)
    : DLList()
  {
    for (const T & item : l)
      append(item);
  }
  
  template <typename T>
  void DLList<T>::copy_list(const DLList & l)
  {
    Node * node = static_cast<Node *>(const_cast<DLList &>(l).get_next());
    
    while ((DL *)node != &l)
      {
	append(node->get_item());
	node = node->get_next();
      }
  }

  template <typename T>
  void DLList<T>::empty_list()
  {
    while (not this->is_empty())
      remove_first();
  }

  template <typename T>
  T & DLList<T>::select(nat_t i)
  {
    for (T & item : *this)
      {
	if (i-- == 0)
	  return item;
      }
    throw std::out_of_range("Index out of range");
  }

  template <typename T>
  const T & DLList<T>::select(nat_t i) const
  {
    for (const T & item : *this)
      {
	if (i-- == 0)
	  return item;
      }
    throw std::out_of_range("Index out of range");
  }
  
} // end namespace Designar

# endif // DSGLIST_H
