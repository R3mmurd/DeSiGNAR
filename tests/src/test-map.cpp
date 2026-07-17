/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <array.hpp>
#include <map.hpp>

using namespace std;
using namespace Designar;

int main()
{
    ArrayMap<string, int_t> array_map = {
        {"One", 1}, {"Two", 2}, {"Three", 3}, {"Four", 4}};

    assert(array_map.size() == 4);
    assert(array_map["One"] == 1);
    assert(array_map["Two"] == 2);
    assert(array_map["Three"] == 3);
    assert(array_map["Four"] == 4);

    array_map.append("Five", 5);

    assert(array_map.size() == 5);

    assert(array_map["Five"] == 5);

    array_map["Six"] = 6;

    assert(array_map.size() == 6);

    assert(array_map["Six"] == 6);

    assert(array_map.equal({{"One", 1},
                            {"Two", 2},
                            {"Three", 3},
                            {"Four", 4},
                            {"Five", 5},
                            {"Six", 6}}));

    auto sum = array_map.fold(
        map_item(string(""), 0), [](const auto& p, const auto& acc)
        { return map_item(key(acc) + key(p) + "+", value(acc) + value(p)); });

    key(sum).pop_back();
    assert(key(sum) == "One+Two+Three+Four+Five+Six" && value(sum) == 21);

    auto prod = array_map.fold(
        map_item(string(""), 1), [](const auto& p, const auto& acc)
        { return map_item(key(acc) + key(p) + "*", value(acc) * value(p)); });

    key(prod).pop_back();
    assert(key(prod) == "One*Two*Three*Four*Five*Six" && value(prod) == 720);

    TreeMap<string, int_t> tree_map = {
        {"One", 1}, {"Two", 2}, {"Three", 3}, {"Four", 4}};

    assert(tree_map.size() == 4);
    assert(tree_map["One"] == 1);
    assert(tree_map["Two"] == 2);
    assert(tree_map["Three"] == 3);
    assert(tree_map["Four"] == 4);

    tree_map.append("Five", 5);

    assert(tree_map.size() == 5);

    assert(tree_map["Five"] == 5);

    tree_map["Six"] = 6;

    assert(tree_map.size() == 6);

    assert(tree_map["Six"] == 6);

    assert(tree_map.equal({{"One", 1},
                           {"Two", 2},
                           {"Three", 3},
                           {"Four", 4},
                           {"Five", 5},
                           {"Six", 6}}));

    key(sum).clear();
    value(sum) = 0;

    sum = tree_map.fold(
        map_item(string(""), 0), [](const auto& p, const auto& acc)
        { return map_item(key(acc) + key(p) + "+", value(acc) + value(p)); });

    key(sum).pop_back();
    assert(key(sum) == "Five+Four+One+Six+Three+Two" && value(sum) == 21);

    key(prod).clear();
    value(prod) = 0;

    prod = tree_map.fold(
        map_item(string(""), 1), [](const auto& p, const auto& acc)
        { return map_item(key(acc) + key(p) + "*", value(acc) * value(p)); });

    key(prod).pop_back();
    assert(key(prod) == "Five*Four*One*Six*Three*Two" && value(prod) == 720);

    HashMap<string, int_t> hash_map = {
        {"One", 1}, {"Two", 2}, {"Three", 3}, {"Four", 4}};

    assert(hash_map.size() == 4);
    assert(hash_map["One"] == 1);
    assert(hash_map["Two"] == 2);
    assert(hash_map["Three"] == 3);
    assert(hash_map["Four"] == 4);

    hash_map.append("Five", 5);

    assert(hash_map.size() == 5);

    assert(hash_map["Five"] == 5);

    hash_map["Six"] = 6;

    assert(hash_map.size() == 6);

    assert(hash_map["Six"] == 6);

    key(sum).clear();
    value(sum) = 0;

    sum = hash_map.fold(
        map_item(string(""), 0), [](const auto& p, const auto& acc)
        { return map_item(key(acc) + key(p) + "+", value(acc) + value(p)); });

    key(sum).pop_back();
    assert(value(sum) == 21);

    key(prod).clear();
    value(prod) = 0;

    prod = hash_map.fold(
        map_item(string(""), 1), [](const auto& p, const auto& acc)
        { return map_item(key(acc) + key(p) + "*", value(acc) * value(p)); });

    key(prod).pop_back();
    assert(value(prod) == 720);

    // search()/find()/has()/remove() used to construct a full MapKey<Key,
    // Value> probe pair with a default-constructed Value purely to look a
    // key up — requiring Value to be default-constructible for no
    // algorithmic reason at all — now routed through a heterogeneous
    // search_by() (nodesdef.hpp's generic_bst_search_by for
    // ArrayMap/TreeMap, hash.hpp/openhash.hpp's own search_by() for
    // HashMap) that never materializes a probe pair at all. Exercised
    // end to end for all three map flavors with a Value type that has no
    // default constructor.
    {
        struct NoDefaultValue
        {
            int_t payload;

            NoDefaultValue() = delete;

            explicit NoDefaultValue(int_t p) : payload(p)
            {
                // empty
            }
        };

        TreeMap<int_t, NoDefaultValue> tm;
        tm.insert(1, NoDefaultValue(10));
        tm.insert(2, NoDefaultValue(20));
        tm.insert(3, NoDefaultValue(30));

        assert(tm.search(2) != nullptr && tm.search(2)->payload == 20);
        assert(tm.search(99) == nullptr);
        assert(tm.has(1) && !tm.has(99));
        assert(tm.remove(2));
        assert(tm.search(2) == nullptr);
        assert(tm.find(1).payload == 10);

        ArrayMap<int_t, NoDefaultValue> am;
        am.insert(1, NoDefaultValue(100));
        am.insert(2, NoDefaultValue(200));

        assert(am.search(1) != nullptr && am.search(1)->payload == 100);
        assert(am.remove(1));
        assert(am.search(1) == nullptr);

        HashMap<int_t, NoDefaultValue> hm;

        for (int_t i = 0; i < 200; ++i)
        {
            hm.insert(i, NoDefaultValue(i * 10));
        }

        assert(hm.search(50) != nullptr && hm.search(50)->payload == 500);
        assert(hm.search(99999) == nullptr);
        assert(hm.remove(50));
        assert(hm.search(50) == nullptr);
        assert(hm.find(1).payload == 10);

        cout << "TreeMap/ArrayMap/HashMap: non-default-constructible Value "
                "Everything ok!\n";
    }

    cout << "Everything ok!\n";

    return 0;
}
