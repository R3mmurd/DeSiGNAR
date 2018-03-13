# include <iostream>

using namespace std;

# include <nodesdef.H>

using namespace Designar;

using Node = MTreeNode<char>;

int main()
{
  Node * root = new Node('A');

  assert(root != nullptr);
  assert(root->get_key() == 'A');
  assert(root->get_left_sibling() == nullptr);
  assert(root->get_right_sibling() == nullptr);
  assert(root->get_first_child() == nullptr);
  assert(root->get_last_child() == nullptr);

  Node * first_child = new Node('B');
  Node * second_child = new Node('C');
  Node * third_child = new Node('D');

  root->append_child(second_child);
  assert(root->get_first_child() == second_child);
  assert(root->get_last_child() == second_child);
  assert(root->get_first_child()->get_left_sibling() == nullptr);
  assert(root->get_first_child()->get_right_sibling() == nullptr);
  assert(root->get_last_child()->get_left_sibling() == nullptr);
  assert(root->get_last_child()->get_right_sibling() == nullptr);

  root->insert_child(first_child);
  assert(root->get_first_child() == first_child);
  assert(root->get_last_child() == second_child);
  assert(root->get_first_child()->get_left_sibling() == nullptr);
  assert(root->get_first_child()->get_right_sibling() == second_child);
  assert(root->get_last_child()->get_left_sibling() == first_child);
  assert(root->get_last_child()->get_right_sibling() == nullptr);

  root->append_child(third_child);
  assert(root->get_first_child() == first_child);
  assert(root->get_last_child() == third_child);
  assert(root->get_first_child()->get_left_sibling() == nullptr);
  assert(root->get_first_child()->get_right_sibling() == second_child);
  assert(root->get_first_child()->get_right_sibling()->get_right_sibling()
	 == third_child);
  assert(root->get_last_child()->get_left_sibling() == second_child);
  assert(root->get_last_child()->get_left_sibling()->get_left_sibling()
	 == first_child);
  assert(root->get_last_child()->get_right_sibling() == nullptr);

  Node * grand_child = new Node('E');
  root->get_first_child()->get_right_sibling()->append_child(grand_child);
  assert(root->get_first_child()->get_right_sibling()->get_first_child()
	 == grand_child);
  assert(root->get_first_child()->get_right_sibling()->get_last_child()
	 == grand_child);

  Node::destroy_tree(root);
  assert(root == nullptr);

  cout << "Everything ok!\n";
  return 0;
}
