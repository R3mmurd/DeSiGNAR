/*
  This file is part of Designar.
  
  Author: Alejandro Mujica (aledrums@gmail.com)
*/

# include <bitset.H>
# include <list.H>
# include <set.H>
# include <polygon.H>

using namespace std;
using namespace Designar;

ostream & operator << (ostream & out, const Polygon::PointType & p)
{
  out << p.to_string();
  return out;
}

ostream & operator << (ostream & out, const Polygon::SegmentType & s)
{
  out << '(' << s.get_src_point().to_string() << ','
      << s.get_tgt_point().to_string();
  return out;
}

string to_str(bool v)
{
  return v ? "true" : "false";
}

template <typename It>
string to_str(const It & it)
{
  stringstream s;
  if (it.has_current())
    s << "Iterator in value " << it.get_current();
  else
    s << "Iterator at end";
  return s.str();
}

template <class Container, typename T = int>
void demo_forward_iterator(const initializer_list<T> & xs)
{
  Container c{xs};
  
  cout << "Declaring iterator explicitly\n";
  for (typename Container::Iterator it(c); it.has_current(); it.next())
    cout << it.get_current() << ' ';
  cout << endl;
  
  cout << "Iterating like stl\n";
  for (auto it = c.begin(); it != c.end(); ++it)
    cout << *it << ' ';
  cout << endl;
 
  cout << "Iterating by using range for\n";
  for (auto item : c)
    cout << item << ' ';
  cout << endl;
}
 
template <class Container, typename T = int>
void demo_bidirectional_iterator(const initializer_list<T> & xs)
{
  demo_forward_iterator<Container, T>(xs);
  
  Container c{xs};

  auto it1 = c.begin();
  auto it2 = c.end();

  cout << "--(" << to_str(it2) << ") = ";
  --it2;
  cout << to_str(it2) << endl;
  
  while (it2 != it1)
    {
      cout << *it2 << ' ';
      --it2;
    }

  cout << *it2 << endl;
}

template <class Container, typename T = int>
void demo_random_access_iterator(const initializer_list<T> & xs)
{
  demo_bidirectional_iterator<Container, T>(xs);
  
  Container c{xs}; 
  
  auto it1 = c.begin();
  auto it2 = it1;
  ++it2;
  
  cout << to_str(it1) << " < " << to_str(it2)
       << " = " << to_str(it1 < it2) << endl;
  
  cout << to_str(it1) << " <= " << to_str(it2) << " = "
       << to_str(it1 <= it2) << endl;
  
  cout << to_str(it1) << " > " << to_str(it2) << " = "
       << to_str(it1 > it2) << endl;
  
  cout << to_str(it1) << " >= " << to_str(it2) << " = "
       << to_str(it1 >= it2) << endl;

  cout << "(" << to_str(it2) << ") += int(8) => ";
  it2 += 8;
  cout << to_str(it2) << endl;

  cout << "(" << to_str(it2) << ") -= int(5) => ";
  it2 -= 5;
  cout << to_str(it2) << endl;

  cout << to_str(it2) << " + 3 => " << to_str(it2 + 3) << endl;

  cout << to_str(it2) << " - 2 => " << to_str(it2 - 2) << endl;

  cout << "(" << to_str(it2) << ")[10] => "
       << to_str(it2) << endl;

}

void demo_polygon_iterators(const Polygon & p)
{
  cout << "Declaring vertex iterator explicitly\n";
  for (Polygon::VertexIterator it(p); it.has_current(); it.next())
    cout << it.get_current() << ' ';
  cout << endl;

  cout << "Iterating like stl\n";
  for (auto it = p.vertices_begin(); it != p.vertices_end(); ++it)
    cout << *it << ' ';
  cout << endl;

  auto vit1 = p.vertices_begin();
  auto vit2 = p.vertices_end();

  cout << "--(" << to_str(vit2) << ") = ";
  --vit2;
  cout << to_str(vit2) << endl;
  
  while (vit2 != vit1)
    {
      cout << *vit2 << ' ';
      --vit2;
    }

  cout << *vit2 << endl;

  cout << "Declaring segment iterator explicitly\n";
  for (Polygon::SegmentIterator it(p); it.has_current(); it.next())
    cout << it.get_current() << ' ';
  cout << endl;

  cout << "Iterating like stl\n";
  for (auto it = p.segments_begin(); it != p.segments_end(); ++it)
    cout << *it << ' ';
  cout << endl;

  auto sit1 = p.segments_begin();
  auto sit2 = p.segments_end();

  cout << "--(" << to_str(sit2) << ") = ";
  --sit2;
  cout << to_str(sit2) << endl;
  
  while (sit2 != sit1)
    {
      cout << *sit2 << ' ';
      --sit2;
    }

  cout << *sit2 << endl;
}

int main()
{
  initializer_list<int> xs{1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20};

  cout << "Forward Iterator\n"
       << "================\n";

  cout << "\nIterator for SLList<int>\n";
  demo_forward_iterator<SLList<int>>(xs);

  cout << "\nIterator for TreeSet<int, less<int>>\n";
  demo_forward_iterator<TreeSet<int>>(xs);

  cout << "Bidirectional Iterators\n"
       << "=======================\n";

  cout << "\nIterator for DLList<int>\n";
  demo_bidirectional_iterator<DLList<int>>(xs);

  cout << "\nIterator for HashSet<int, equal<int>>\n";
  demo_bidirectional_iterator<HashSet<int>>(xs);

  cout << "\nIterators for Polygon\n";
  demo_polygon_iterators({{0,0},{1,0},{0,1}});
  
  cout << "Random Access Iterators\n"
       << "=======================\n";

  cout << "\nIterators for FixedArray<int>\n";
  demo_random_access_iterator<FixedArray<int>>(xs);

  cout << "\nIterators for DynArray<int>\n";
  demo_random_access_iterator<DynArray<int>>(xs);

  cout << "\nIterators for ArraySet<int>\n";
  demo_random_access_iterator<ArraySet<int>>(xs);

  cout << "\nIterators for SortedArraySet<int>\n";
  demo_random_access_iterator<SortedArraySet<int>>(xs);

  cout << "\nIterators for DynBitSet\n";
  demo_random_access_iterator<DynBitSet, bool>(
    {1,1,1,0,0,0,1,0,1,0,0,1,0,0,0,1,1,1});
  
  return 0;
}
 
