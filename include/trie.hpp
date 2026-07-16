/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

/** @file trie.hpp
    @brief Trie (prefix tree): stores a set of symbol sequences (words)
    letting shared prefixes share storage, and answers both exact
    membership and "does any stored word start with this prefix"
    queries in time proportional to the query length rather than the
    number of stored words.
    @ingroup DataStructures
*/

#pragma once

#include <map.hpp>

namespace Designar
{
  /** A Trie over sequences of `Symbol` (a `std::string` of `char` being
      the classic case, but any equality-comparable, hashable symbol
      type works — this is deliberately not hardcoded to `char`/
      `std::string` the way most textbook tries are). Each node maps the
      *next* symbol of every word passing through it to the child
      holding the rest of that word, via this library's own HashMap, so
      the branching factor is exactly the number of distinct symbols
      actually used at that point — not, say, a fixed 26/128/256-entry
      array sized for an alphabet that may not apply to `Symbol` at all. */
  template <typename Symbol>
  class Trie
  {
    struct Node
    {
      HashMap<Symbol, Node*> children;
      bool is_word = false;
    };

    Node* root;
    nat_t num_words;

    static void destroy(Node* n)
    {
      if (n == nullptr)
      {
        return;
      }

      for (auto& child : n->children)
      {
        destroy(child.second);
      }

      delete n;
    }

    static Node* copy(Node* n)
    {
      if (n == nullptr)
      {
        return nullptr;
      }

      Node* c = new Node;
      c->is_word = n->is_word;

      for (auto& child : n->children)
      {
        c->children.insert(child.first, copy(child.second));
      }

      return c;
    }

  public:
    Trie()
        : root(new Node), num_words(0)
    {
      // empty
    }

    Trie(const Trie& t)
        : root(copy(t.root)), num_words(t.num_words)
    {
      // empty
    }

    Trie(Trie&& t)
        : root(new Node), num_words(0)
    {
      swap(t);
    }

    ~Trie()
    {
      destroy(root);
    }

    Trie& operator=(const Trie& t)
    {
      if (this == &t)
      {
        return *this;
      }

      destroy(root);
      root = copy(t.root);
      num_words = t.num_words;
      return *this;
    }

    Trie& operator=(Trie&& t)
    {
      swap(t);
      return *this;
    }

    void swap(Trie& t)
    {
      std::swap(root, t.root);
      std::swap(num_words, t.num_words);
    }

    bool is_empty() const
    {
      return num_words == 0;
    }

    nat_t size() const
    {
      return num_words;
    }

    /** Inserts `seq` (anything iterable yielding `Symbol`s — a
        `std::string`, a `DynArray<Symbol>`, ...). Returns false (and
        leaves the trie unchanged) if `seq` was already stored. */
    template <class Sequence>
    bool insert(const Sequence& seq)
    {
      Node* curr = root;

      for (const Symbol& sym : seq)
      {
        Node** next = curr->children.search(sym);

        if (next == nullptr)
        {
          Node* child = new Node;
          curr->children.insert(sym, child);
          curr = child;
        }
        else
        {
          curr = *next;
        }
      }

      if (curr->is_word)
      {
        return false;
      }

      curr->is_word = true;
      ++num_words;
      return true;
    }

  private:
    template <class Sequence>
    Node* find_node(const Sequence& seq) const
    {
      Node* curr = root;

      for (const Symbol& sym : seq)
      {
        Node* const* next = curr->children.search(sym);

        if (next == nullptr)
        {
          return nullptr;
        }

        curr = *next;
      }

      return curr;
    }

  public:
    template <class Sequence>
    bool search(const Sequence& seq) const
    {
      Node* n = find_node(seq);
      return n != nullptr && n->is_word;
    }

    /** Whether any stored word begins with `seq` (`seq` itself need not
        be a stored word). */
    template <class Sequence>
    bool has_prefix(const Sequence& seq) const
    {
      return find_node(seq) != nullptr;
    }

    /** Removes `seq` if it was stored as a complete word (a shared
        prefix of other words is unmarked, not deleted — its nodes are
        still needed by whichever other words pass through them; nodes
        are only ever freed by the destructor/copy-assignment, not by
        remove(), to keep this operation simple). Returns whether `seq`
        was actually present. */
    template <class Sequence>
    bool remove(const Sequence& seq)
    {
      Node* n = find_node(seq);

      if (n == nullptr || !n->is_word)
      {
        return false;
      }

      n->is_word = false;
      --num_words;
      return true;
    }
  };

} // end namespace Designar
