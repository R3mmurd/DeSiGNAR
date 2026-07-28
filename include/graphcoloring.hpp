/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

/** @file graphcoloring.hpp
    @brief Greedy graph coloring (Welsh-Powell): visiting nodes in
    decreasing-degree order and assigning each the lowest color not
    already used by an already-colored neighbor — a fast heuristic for
    an NP-hard problem (minimum graph coloring), not an exact algorithm;
    it never uses more colors than `max_degree + 1`, but does not
    guarantee the true chromatic number.
    @ingroup Algorithms
*/

#pragma once

#include <algorithm>

#include <array.hpp>
#include <map.hpp>
#include <set.hpp>
#include <graphutilities.hpp>

namespace Designar
{
    /** Assigns every node of `g` a color (a small non-negative integer)
        such that no two adjacent nodes share one — visiting nodes in
        decreasing order of degree first (Welsh-Powell's own heuristic:
        the highest-degree, hardest-to-color nodes get first pick of the
        low color numbers, before their many neighbors have already
        claimed most of them), assigning each the smallest color number
        none of its already-colored neighbors currently holds. */
    template <class GT>
    HashMap<Node<GT>*, nat_t> greedy_graph_coloring(const GT& g)
    {
        DynArray<Node<GT>*> nodes;

        g.for_each_node([&](Node<GT>* p) { nodes.append(p); });

        std::sort(nodes.begin(), nodes.end(),
                 [](Node<GT>* a, Node<GT>* b)
                 { return a->get_num_arcs() > b->get_num_arcs(); });

        HashMap<Node<GT>*, nat_t> color;

        for (Node<GT>* p : nodes)
        {
            HashSet<nat_t> used_by_neighbors;

            for (AdArcIt<GT> it(g, p); it.has_current(); it.next())
            {
                Arc<GT>* a = *it;
                Node<GT>* q = a->get_connected_node(p);
                nat_t* neighbor_color = color.search(q);

                if (neighbor_color != nullptr)
                {
                    used_by_neighbors.insert(*neighbor_color);
                }
            }

            nat_t chosen = 0;

            while (used_by_neighbors.search(chosen) != nullptr)
            {
                ++chosen;
            }

            color.insert(p, chosen);
        }

        return color;
    }

    /** The number of distinct colors greedy_graph_coloring() actually
        used — one more than the largest color number assigned, since
        colors are numbered from 0. Templated directly over the key
        type `K` (rather than over `GT`, deducing the key as `Node<GT>*`
        — a template alias like `Node<GT>` is not a deducible context in
        C++, so a parameter declared that way could never actually have
        `GT` inferred from an argument). */
    template <typename K>
    nat_t count_colors_used(const HashMap<K, nat_t>& coloring)
    {
        if (coloring.is_empty())
        {
            return 0;
        }

        nat_t max_color = 0;

        for (const auto& entry : coloring)
        {
            if (entry.second > max_color)
            {
                max_color = entry.second;
            }
        }

        return max_color + 1;
    }

} // end namespace Designar
