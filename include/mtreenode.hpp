/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#pragma once

#include <stdexcept>
#include <utility>

#include <types.hpp>
#include <iterator.hpp>
#include <nodesdef.hpp>

namespace Designar
{

    template <typename Key>
    class MTreeNode : private DLNode<Key>
    {
        struct SiblingInfo
        {
            unsigned int is_leftmost : 4;
            unsigned int is_rightmost : 4;

            SiblingInfo() : is_leftmost(true), is_rightmost(true)
            {
                // empty
            }
        };

        MTreeNode* parent = nullptr;
        MTreeNode* first_child = nullptr;
        SiblingInfo sibling_info;

        using Base = DLNode<Key>;

        static MTreeNode* to_treenode(Base* p)
        {
            return static_cast<MTreeNode*>(p);
        }

        void insert_first_child(MTreeNode* c)
        {
            assert(first_child == nullptr);
            c->parent = this;
            first_child = c;
        }

    public:
        using KeyType = Key;
        using ValueType = Key;
        using ItemType = Key;

        MTreeNode() = default;

        MTreeNode(const Key& k) : Base(k)
        {
            // empty
        }

        MTreeNode(Key&& k) : Base(std::forward<Key>(k))
        {
            // empty
        }

        Key& get_key()
        {
            return Base::get_item();
        }

        const Key& get_key() const
        {
            return Base::get_item();
        }

        MTreeNode* get_parent() const
        {
            return parent;
        }

        MTreeNode* get_first_child() const
        {
            return first_child;
        }

        MTreeNode* get_last_child() const
        {
            if (first_child == nullptr)
            {
                return nullptr;
            }

            return to_treenode(first_child->get_prev());
        }

        MTreeNode* get_right_sibling() const
        {
            if (sibling_info.is_rightmost)
            {
                return nullptr;
            }

            return to_treenode(const_cast<Base*>(Base::get_next()));
        }

        MTreeNode* get_left_sibling() const
        {
            if (sibling_info.is_leftmost)
            {
                return nullptr;
            }

            return to_treenode(const_cast<Base*>(Base::get_prev()));
        }

        bool is_leaf() const
        {
            return first_child == nullptr;
        }

        bool has_siblings() const
        {
            return !Base::is_empty();
        }

        bool has_parent() const
        {
            return parent != nullptr;
        }

        bool has_children() const
        {
            return first_child != nullptr;
        }

        void reset_sibling_info()
        {
            sibling_info.is_leftmost = sibling_info.is_rightmost = true;
        }

        void reset()
        {
            Base::reset();
            parent = first_child = nullptr;
            reset_sibling_info();
        }

        void add_right_sibling(MTreeNode* s)
        {
            assert(s != nullptr);
            assert(!s->has_siblings());
            assert(!s->has_parent());
            Base::insert_next(s);
            s->sibling_info.is_rightmost = sibling_info.is_rightmost;
            s->sibling_info.is_leftmost = false;
            sibling_info.is_rightmost = false;
            s->parent = parent;
        }

        void add_left_sibling(MTreeNode* s)
        {
            assert(s != nullptr);
            assert(!s->has_siblings());
            assert(!s->has_parent());
            Base::insert_prev(s);
            s->sibling_info.is_leftmost = sibling_info.is_leftmost;
            s->sibling_info.is_rightmost = false;
            sibling_info.is_leftmost = false;
            s->parent = parent;

            if (s->sibling_info.is_leftmost && parent)
            {
                parent->first_child = s;
            }
        }

        void insert_child(MTreeNode* c)
        {
            assert(c != nullptr);
            assert(!c->has_siblings());
            assert(!c->has_parent());

            if (first_child == nullptr)
            {
                insert_first_child(c);
            }
            else
            {
                first_child->add_left_sibling(c);
            }
        }

        void append_child(MTreeNode* c)
        {
            assert(c != nullptr);
            assert(!c->has_siblings());
            assert(!c->has_parent());

            if (first_child == nullptr)
            {
                insert_first_child(c);
            }
            else
            {
                to_treenode(first_child->get_prev())->add_right_sibling(c);
            }
        }

        MTreeNode* remove_first_child()
        {
            if (first_child == nullptr)
            {
                return nullptr;
            }

            MTreeNode* ret_val = first_child;

            if (ret_val->sibling_info.is_rightmost)
            {
                first_child = nullptr;
            }
            else
            {
                first_child = to_treenode(ret_val->get_next());
                first_child->sibling_info.is_leftmost = true;
                ret_val->del();
            }

            ret_val->parent = nullptr;
            ret_val->reset_sibling_info();
            return ret_val;
        }

        MTreeNode* remove_last_child()
        {
            if (first_child == nullptr)
            {
                return nullptr;
            }

            MTreeNode* ret_val = to_treenode(first_child->get_prev());

            if (ret_val->sibling_info.is_leftmost)
            {
                first_child = nullptr;
            }
            else
            {
                ret_val->get_left_sibling()->sibling_info.is_rightmost = true;
                ret_val->del();
            }

            ret_val->parent = nullptr;
            ret_val->reset_sibling_info();
            return ret_val;
        }

        class ChildrenIterator
            : public BidirectionalIterator<ChildrenIterator, MTreeNode*, true>
        {
            friend class BasicIterator<ChildrenIterator, MTreeNode*, true>;

            MTreeNode* first;
            MTreeNode* curr;

        protected:
            MTreeNode* get_location() const
            {
                return curr;
            }

        public:
            ChildrenIterator() : first(nullptr), curr(nullptr)
            {
                // empty
            }

            ChildrenIterator(const MTreeNode& node)
                : first(const_cast<MTreeNode*>(&node)->get_first_child()),
                  curr(first)
            {
                // empty
            }

            ChildrenIterator(const ChildrenIterator& it)
                : first(it.first), curr(it.curr)
            {
                // empty
            }

            ChildrenIterator(ChildrenIterator&& it) : ChildrenIterator()
            {
                swap(it);
            }

            ChildrenIterator& operator=(const ChildrenIterator& it)
            {
                if (this == &it)
                {
                    return *this;
                }

                first = it.first;
                curr = it.curr;
                return *this;
            }

            ChildrenIterator& operator=(ChildrenIterator&& it)
            {
                swap(it);
                return *this;
            }

            void swap(ChildrenIterator& it)
            {
                std::swap(first, it.first);
                std::swap(curr, it.curr);
            }

            bool has_current() const
            {
                return curr != nullptr;
            }

            MTreeNode* get_current()
            {
                if (!has_current())
                {
                    throw std::overflow_error("There is not current element");
                }

                return curr;
            }

            MTreeNode* get_current() const
            {
                if (!has_current())
                {
                    throw std::overflow_error("There is not current element");
                }

                return curr;
            }

            void next()
            {
                if (!has_current())
                {
                    throw std::out_of_range("There is not next element");
                }

                curr = curr->get_right_sibling();
            }

            void prev()
            {
                if (curr == first)
                {
                    throw std::out_of_range("There is not previous element");
                }

                if (curr == nullptr)
                {
                    curr = to_treenode(first->get_prev());
                }
                else
                {
                    curr = curr->get_left_sibling();
                }
            }
        };

        template <class Op>
        void for_each_child(Op&) const;

        template <class Op>
        void for_each_child(Op&& op = Op()) const
        {
            for_each_child<Op>(op);
        }

        static void destroy_tree(MTreeNode*&);
    };

    template <typename Key>
    template <class Op>
    void MTreeNode<Key>::for_each_child(Op& op) const
    {
        MTreeNode* ptr = first_child;

        while (ptr != nullptr)
        {
            op(ptr);
            ptr = ptr->get_right_sibling();
        }
    }

    template <typename Key>
    void MTreeNode<Key>::destroy_tree(MTreeNode*& r)
    {
        if (r == nullptr)
        {
            return;
        }

        MTreeNode* fc = nullptr;

        while ((fc = r->remove_first_child()))
        {
            destroy_tree(fc);
        }

        delete r;
        r = nullptr;
    }

} // end namespace Designar
