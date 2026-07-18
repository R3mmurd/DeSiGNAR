/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <cassert>
#include <iostream>

using namespace std;

#include <set.hpp>
#include <random.hpp>

using namespace Designar;

/** Exercises the common Set-like contract (insert/search/remove/min/
    max/copy/move/inorder-iteration) shared by every BST in this
    library: Tree, Treap, AVLTree, RbTree, RandomizedTree (ABBA), and
    SplayTree. The Ranked* trees add order-statistics on top of this and
    are tested separately below. */
template <template <typename, class> class TreeType>
void test_common_tree_contract(const char* name)
{
    TreeType<int_t, std::less<int_t>> t;
    assert(t.is_empty());

    constexpr int_t N = 1500;

    for (int_t i = 0; i < N; ++i)
        t.insert(i);

    assert(t.size() == (nat_t)N);
    assert(!t.is_empty());
    assert(t.min() == 0);
    assert(t.max() == N - 1);

    for (int_t i = 0; i < N; ++i)
        assert(t.search(i) != nullptr);
    assert(t.search(-1) == nullptr);
    assert(t.search(N) == nullptr);

    int_t prev = -1;
    nat_t count = 0;
    for (int_t x : t)
    {
        assert(x > prev);
        prev = x;
        ++count;
    }
    assert(count == (nat_t)N);

    // Remove every key in a pseudo-random order.
    rng_t rng(20260716);
    DynArray<int_t> keys;
    for (int_t i = 0; i < N; ++i)
        keys.append(i);

    for (nat_t i = keys.size() - 1; i > 0; --i)
    {
        nat_t j = random_uniform(rng, i + 1);
        std::swap(keys[i], keys[j]);
    }

    nat_t expected_size = (nat_t)N;
    for (int_t k : keys)
    {
        assert(t.remove(k));
        --expected_size;
        assert(t.size() == expected_size);
        assert(t.search(k) == nullptr);
    }
    assert(t.is_empty());

    // Copy/move semantics.
    TreeType<int_t, std::less<int_t>> t2;
    for (int_t i = 0; i < 200; ++i)
        t2.insert(i);

    TreeType<int_t, std::less<int_t>> t3 = t2;
    t3.insert(999999);
    assert(t3.search(999999) != nullptr);
    assert(t2.search(999999) == nullptr);

    TreeType<int_t, std::less<int_t>> t4 = std::move(t3);
    assert(t4.search(999999) != nullptr);

    TreeSet<int_t, std::less<int_t>, TreeType> ts = {5, 3, 8, 1, 9};
    assert(ts.equal({1, 3, 5, 8, 9}));

    cout << name << ": common contract ok!\n";
}

/** Exercises select()/position()/operator[] (order-statistics) shared
    by every order-statistics-capable tree in this library: RankedTree,
    RankedAVLTree, RankedRBTree, RankedTreap, and RandomizedTree. */
template <template <typename, class> class TreeType>
void test_order_statistics_contract(const char* name)
{
    TreeType<int_t, std::less<int_t>> t;

    constexpr nat_t N = 800;
    for (int_t i = 0; i < (int_t)N; ++i)
        t.insert(i);

    assert(t.size() == N);

    for (nat_t i = 0; i < N; ++i)
    {
        assert(t.select(i) == (int_t)i);
        assert(t.position(t.select(i)) == (int_t)i);
        assert(t[i] == (int_t)i);
    }

    cout << name << ": order-statistics contract ok!\n";
}

/** Exercises split_pos(): across many random (tree size, split position)
    combinations, the two resulting trees must partition the original
    keyset exactly at the given rank and each still satisfy its own
    verify() — the main defense against subtle join-algorithm bugs (see
    RankedAVLTree's AVL-height-based join, this session's
    hardest-to-get-right piece). Shared by every tree in this library
    that offers split_pos: RankedTree, RankedAVLTree, RankedRBTree,
    RankedTreap, and RandomizedTree. */
template <template <typename, class> class TreeType>
void test_split_pos_contract(const char* name)
{
    rng_t rng(20260718);

    for (int_t trial = 0; trial < 200; ++trial)
    {
        nat_t N = 1 + random_uniform(rng, 300);
        TreeType<int_t, std::less<int_t>> t;

        DynArray<int_t> keys;
        for (nat_t i = 0; i < N; ++i)
            keys.append((int_t)i);

        for (nat_t i = keys.size() - 1; i > 0; --i)
        {
            nat_t j = random_uniform(rng, i + 1);
            std::swap(keys[i], keys[j]);
        }

        for (int_t k : keys)
            t.insert(k);

        assert(t.verify());

        nat_t pos = random_uniform(rng, N);
        auto [left, right] = t.split_pos(pos);

        assert(left.verify());
        assert(right.verify());
        assert(left.size() == pos);
        assert(right.size() == N - pos);

        for (nat_t i = 0; i < pos; ++i)
            assert(left.select(i) == (int_t)i);
        for (nat_t i = 0; i < right.size(); ++i)
            assert(right.select(i) == (int_t)(pos + i));
    }

    cout << name << ": split_pos contract ok!\n";
}

/** Exercises remove_pos() (delete-by-rank): shared only by RankedTreap
    and RankedAVLTree — RankedRBTree deliberately doesn't have it (see
    its class doc comment in rbtree.hpp for why). */
template <template <typename, class> class TreeType>
void test_remove_pos_contract(const char* name)
{
    TreeType<int_t, std::less<int_t>> t;

    constexpr nat_t N = 800;
    for (int_t i = 0; i < (int_t)N; ++i)
        t.insert(i);

    auto removed = t.remove_pos(N / 2);
    assert(removed == (int_t)(N / 2));
    assert(t.size() == N - 1);
    assert(t.search((int_t)(N / 2)) == nullptr);

    cout << name << ": remove_pos contract ok!\n";
}

int main()
{
    test_common_tree_contract<Tree>("Tree");
    test_common_tree_contract<Treap>("Treap");
    test_common_tree_contract<AVLTree>("AVLTree");
    test_common_tree_contract<RbTree>("RbTree");
    test_common_tree_contract<RandomizedTree>("RandomizedTree (ABBA)");
    test_common_tree_contract<SplayTree>("SplayTree");

    test_order_statistics_contract<RankedTree>("RankedTree");
    test_order_statistics_contract<RankedAVLTree>("RankedAVLTree");
    test_order_statistics_contract<RankedRBTree>("RankedRBTree");
    test_order_statistics_contract<RankedTreap>("RankedTreap");
    test_order_statistics_contract<RandomizedTree>("RandomizedTree (ABBA)");

    test_split_pos_contract<RankedTree>("RankedTree");
    test_split_pos_contract<RankedAVLTree>("RankedAVLTree");
    test_split_pos_contract<RankedRBTree>("RankedRBTree");
    test_split_pos_contract<RankedTreap>("RankedTreap");
    test_split_pos_contract<RandomizedTree>("RandomizedTree (ABBA)");

    test_remove_pos_contract<RankedTreap>("RankedTreap");
    test_remove_pos_contract<RankedAVLTree>("RankedAVLTree");

    // Regression: the very first bug that motivated verify()-style checks
    // in this session — every one of these trees must satisfy its own
    // structural invariant after heavy random insert/remove churn, not
    // just "look sorted" (which a completely broken structure could still
    // pass by accident if iteration itself were buggy in the same way).
    {
        AVLTree<int_t> avl;
        RbTree<int_t> rb;
        RandomizedTree<int_t> abba;

        rng_t rng(7);
        DynArray<int_t> present;

        for (int_t i = 0; i < 4000; ++i)
        {
            if (present.is_empty() || random_uniform(rng, 100) < 65)
            {
                int_t k = random_uniform(rng, 100000);
                if (avl.search(k) == nullptr)
                {
                    avl.insert(k);
                    rb.insert(k);
                    abba.insert(k);
                    present.append(k);
                }
            }
            else
            {
                nat_t idx = random_uniform(rng, present.size());
                int_t k = present[idx];
                assert(avl.remove(k));
                assert(rb.remove(k));
                assert(abba.remove(k));
                present.remove_pos(idx);
            }

            assert(avl.verify());
            assert(rb.verify());
            assert(abba.verify());
            assert(avl.size() == present.size());
            assert(rb.size() == present.size());
            assert(abba.size() == present.size());
        }

        cout
            << "AVLTree/RbTree/RandomizedTree: interleaved stress verify ok!\n";
    }

    // Splay-tree-specific: repeated access to the same key must not
    // corrupt the structure, and the tree must still iterate in order
    // after heavy, skewed access patterns (splay trees are exactly the
    // structure where a bug tends to show up only after many accesses,
    // not the first one).
    {
        SplayTree<int_t> t;
        for (int_t i = 0; i < 500; ++i)
            t.insert(i);

        rng_t rng(31);
        for (int_t i = 0; i < 5000; ++i)
            t.search(random_uniform(rng, 500));

        int_t prev = -1;
        nat_t count = 0;
        for (int_t x : t)
        {
            assert(x > prev);
            prev = x;
            ++count;
        }
        assert(count == 500);

        cout << "SplayTree: repeated-access stress ok!\n";
    }

    // Every one of these trees shares BaseBinTreeNode (nodesdef.hpp) as
    // its node representation, whose internal sentinel/head bookkeeping
    // node used to unconditionally default-construct a `Key` — even
    // though that node's key is never read by any tree algorithm — so a
    // Key type with no default constructor at all could not be used with
    // *any* of these trees. `NoDefaultKey` below has an explicitly
    // deleted default constructor, only a value constructor and
    // comparison, exercising exactly that fixed path end to end (insert,
    // search, remove) for the two most commonly used trees.
    {
        struct NoDefaultKey
        {
            int_t value;

            NoDefaultKey() = delete;

            explicit NoDefaultKey(int_t v) : value(v)
            {
                // empty
            }

            bool operator<(const NoDefaultKey& o) const
            {
                return value < o.value;
            }
        };

        AVLTree<NoDefaultKey> avl;
        avl.insert(NoDefaultKey(5));
        avl.insert(NoDefaultKey(3));
        avl.insert(NoDefaultKey(8));
        avl.insert(NoDefaultKey(1));

        assert(avl.search(NoDefaultKey(5)) != nullptr);
        assert(avl.search(NoDefaultKey(9)) == nullptr);

        avl.remove(NoDefaultKey(3));
        assert(avl.search(NoDefaultKey(3)) == nullptr);
        assert(avl.search(NoDefaultKey(5)) != nullptr);

        RbTree<NoDefaultKey> rb;
        rb.insert(NoDefaultKey(5));
        rb.insert(NoDefaultKey(3));
        rb.insert(NoDefaultKey(8));

        assert(rb.search(NoDefaultKey(8)) != nullptr);
        rb.remove(NoDefaultKey(8));
        assert(rb.search(NoDefaultKey(8)) == nullptr);

        cout << "AVLTree/RbTree: non-default-constructible Key Everything "
                "ok!\n";
    }

    cout << "Everything ok!\n";
    return 0;
}
