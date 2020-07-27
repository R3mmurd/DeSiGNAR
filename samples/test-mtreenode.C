/*
  This file is part of Designar.
  
  Author: Alejandro Mujica (aledrums@gmail.com)
*/

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
  assert(not root->has_parent());
  assert(not root->has_siblings());
  assert(not root->has_children());

  Node * first_child = new Node('B');
  assert(not first_child->has_parent());
  assert(not first_child->has_siblings());
  assert(not first_child->has_children());
  Node * second_child = new Node('C');
  assert(not second_child->has_parent());
  assert(not second_child->has_siblings());
  assert(not second_child->has_children());
  Node * third_child = new Node('D');
  assert(not third_child->has_parent());
  assert(not third_child->has_siblings());
  assert(not third_child->has_children());

  root->append_child(second_child);
  assert(root->has_children());
  assert(second_child->has_parent());
  assert(not second_child->has_siblings());
  assert(not second_child->has_children());
  assert(root->get_first_child() == second_child);
  assert(root->get_last_child() == second_child);
  assert(root->get_first_child()->get_left_sibling() == nullptr);
  assert(root->get_first_child()->get_right_sibling() == nullptr);
  assert(root->get_last_child()->get_left_sibling() == nullptr);
  assert(root->get_last_child()->get_right_sibling() == nullptr);

  root->insert_child(first_child);
  assert(root->has_children());
  assert(first_child->has_parent());
  assert(first_child->has_siblings());
  assert(second_child->has_siblings());
  assert(not first_child->has_children());
  assert(root->get_first_child() == first_child);
  assert(root->get_last_child() == second_child);
  assert(root->get_first_child()->get_left_sibling() == nullptr);
  assert(root->get_first_child()->get_right_sibling() == second_child);
  assert(root->get_last_child()->get_left_sibling() == first_child);
  assert(root->get_last_child()->get_right_sibling() == nullptr);

  root->append_child(third_child);
  assert(root->has_children());
  assert(third_child->has_parent());
  assert(first_child->has_siblings());
  assert(second_child->has_siblings());
  assert(third_child->has_siblings());
  assert(not third_child->has_children());
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
  assert(root->has_children());
  assert(grand_child->has_parent());
  assert(not grand_child->has_siblings());
  assert(second_child->has_children());
  assert(not grand_child->has_children());
  assert(root->get_first_child()->get_right_sibling()->get_first_child()
	 == grand_child);
  assert(root->get_first_child()->get_right_sibling()->get_last_child()
	 == grand_child);

  Node::destroy_tree(root);
  assert(root == nullptr);

  cout << "Everything ok!\n";
  return 0;
}
