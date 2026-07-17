/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <iostream>
#include <cassert>
#include <functional>
#include <minimax.hpp>

using namespace std;
using namespace Designar;

namespace
{
  bool close(real_t a, real_t b)
  {
    return std::abs(a - b) < 1e-9;
  }

  /** A small, explicit game tree (not an actual game) used purely to
      verify alpha-beta pruning against known-by-hand values and a
      known-by-hand pruning count. A textbook example: root is a MAX
      node with 3 MIN children, each of which has 3 leaf children:

                          MAX
                 /         |         \
              MIN         MIN         MIN
             / | \        / | \       / | \
            3 12  8      2  4  6    14  5  2

      Hand-computed minimax:
        MIN #1 = min(3,12,8)  = 3
        MIN #2 = min(2,4,6)   = 2
        MIN #3 = min(14,5,2)  = 2
        MAX    = max(3,2,2)   = 3

      Hand-traced alpha-beta (left to right):
        MIN #1: alpha=-inf,beta=+inf; evaluates all 3 leaves
                (no cutoff possible with beta=+inf), returns 3.
                Root: alpha = max(-inf,3) = 3.
        MIN #2: alpha=3,beta=+inf; first leaf=2 -> beta=2;
                beta(2) <= alpha(3) -> cutoff. Only 1 leaf evaluated
                (4 and 6 are pruned). Returns 2. Root alpha stays 3
                (2 does not beat 3).
        MIN #3: alpha=3,beta=+inf; leaves 14,5,2 all evaluated (beta
                never drops to <= 3 until the very last leaf, by
                which point there is nothing left to prune). Returns
                2. Root alpha stays 3.
        Root MAX value = 3.
      Total leaves evaluated: 3 + 1 + 3 = 7 out of 9 — 2 fewer than a
      pruning-free minimax would evaluate. */
  struct TreeNode
  {
    DynArray<nat_t> children;
    real_t value;
    bool is_leaf;
  };

  DynArray<TreeNode> build_textbook_tree()
  {
    DynArray<TreeNode> tree;

    // Leaves: indices 0..8, values 3,12,8,2,4,6,14,5,2
    real_t leaf_values[9] = {3, 12, 8, 2, 4, 6, 14, 5, 2};

    for (nat_t i = 0; i < 9; ++i)
    {
      tree.append(TreeNode{DynArray<nat_t>(), leaf_values[i], true});
    }

    // MIN nodes: indices 9,10,11
    tree.append(TreeNode{DynArray<nat_t>({0, 1, 2}), 0, false});
    tree.append(TreeNode{DynArray<nat_t>({3, 4, 5}), 0, false});
    tree.append(TreeNode{DynArray<nat_t>({6, 7, 8}), 0, false});

    // Root MAX node: index 12
    tree.append(TreeNode{DynArray<nat_t>({9, 10, 11}), 0, false});

    return tree;
  }
} // end anonymous namespace

int main()
{
  // The hand-traced textbook tree above.
  {
    DynArray<TreeNode> tree = build_textbook_tree();
    nat_t leaf_evals = 0;

    auto get_children = [&](const nat_t& idx)
    {
      DynArray<nat_t> result;

      for (nat_t i = 0; i < tree[idx].children.size(); ++i)
      {
        result.append(tree[idx].children[i]);
      }

      return result;
    };

    auto is_terminal = [&](const nat_t& idx)
    { return tree[idx].is_leaf; };

    auto evaluate = [&](const nat_t& idx) -> real_t
    {
      ++leaf_evals;
      return tree[idx].value;
    };

    real_t value = alpha_beta_search<nat_t>(nat_t(12), 3, -std::numeric_limits<real_t>::infinity(),
                                            std::numeric_limits<real_t>::infinity(), true,
                                            get_children, is_terminal, evaluate);

    assert(close(value, 3));
    assert(leaf_evals == 7);

    cout << "alpha_beta_search: hand-traced textbook tree, value=3, 7/9 leaves pruned-correctly Everything ok!\n";
  }

  // Without pruning (very wide alpha/beta window collapsed via depth
  // limit of 0 evaluating immediately) every leaf gets visited exactly
  // once when there is nothing above to cut against — sanity check
  // that alpha=-inf/beta=+inf alone (root call) is what enables
  // pruning, not some hidden default.
  {
    DynArray<TreeNode> tree = build_textbook_tree();
    nat_t leaf_evals = 0;

    auto get_children = [&](const nat_t& idx)
    {
      DynArray<nat_t> result;

      for (nat_t i = 0; i < tree[idx].children.size(); ++i)
      {
        result.append(tree[idx].children[i]);
      }

      return result;
    };

    auto is_terminal = [&](const nat_t& idx)
    { return tree[idx].is_leaf; };

    auto evaluate = [&](const nat_t& idx) -> real_t
    {
      ++leaf_evals;
      return tree[idx].value;
    };

    // A single MIN subtree evaluated on its own (no sibling context
    // to prune against) must visit all 3 of its leaves.
    real_t min1 = alpha_beta_search<nat_t>(nat_t(9), 2, -std::numeric_limits<real_t>::infinity(),
                                           std::numeric_limits<real_t>::infinity(), false,
                                           get_children, is_terminal, evaluate);

    assert(close(min1, 3));
    assert(leaf_evals == 3);

    cout << "alpha_beta_search: isolated subtree visits every leaf (no false pruning) Everything ok!\n";
  }

  // find_best_move on the same textbook tree: the best move from the
  // root must be the first MIN subtree (index 9), since it is the one
  // that achieves the overall value of 3.
  {
    DynArray<TreeNode> tree = build_textbook_tree();

    auto get_children = [&](const nat_t& idx)
    {
      DynArray<nat_t> result;

      for (nat_t i = 0; i < tree[idx].children.size(); ++i)
      {
        result.append(tree[idx].children[i]);
      }

      return result;
    };

    auto is_terminal = [&](const nat_t& idx)
    { return tree[idx].is_leaf; };

    auto evaluate = [&](const nat_t& idx) -> real_t
    { return tree[idx].value; };

    MoveSearchResult<nat_t> result =
        find_best_move<nat_t>(nat_t(12), 3, true, get_children, is_terminal, evaluate);

    assert(result.has_move);
    assert(close(result.score, 3));
    assert(result.best_state == 9);

    cout << "find_best_move: correct best child on the textbook tree Everything ok!\n";
  }

  // A single terminal state (no children at all) reports has_move ==
  // false and simply its own evaluation as the score.
  {
    DynArray<TreeNode> tree;
    tree.append(TreeNode{DynArray<nat_t>(), 42, true});

    auto get_children = [&](const nat_t&)
    { return DynArray<nat_t>(); };

    auto is_terminal = [&](const nat_t& idx)
    { return tree[idx].is_leaf; };

    auto evaluate = [&](const nat_t& idx) -> real_t
    { return tree[idx].value; };

    MoveSearchResult<nat_t> result =
        find_best_move<nat_t>(nat_t(0), 5, true, get_children, is_terminal, evaluate);

    assert(!result.has_move);
    assert(close(result.score, 42));

    cout << "find_best_move: terminal root reports no move Everything ok!\n";
  }

  // Tic-Tac-Toe with two perfect players (both call find_best_move
  // with an unlimited search depth) must always end in a draw — the
  // best-known, exactly verifiable property of the game.
  {
    // Board: 9 cells, 0 = empty, 1 = X (maximizer), 2 = O (minimizer).
    using Board = DynArray<int_t>;

    auto winner = [](const Board& b) -> int_t
    {
      const nat_t lines[8][3] = {{0, 1, 2}, {3, 4, 5}, {6, 7, 8}, {0, 3, 6},
                                 {1, 4, 7}, {2, 5, 8}, {0, 4, 8}, {2, 4, 6}};

      for (nat_t l = 0; l < 8; ++l)
      {
        int_t a = b[lines[l][0]];
        int_t c = b[lines[l][1]];
        int_t d = b[lines[l][2]];

        if (a != 0 && a == c && c == d)
        {
          return a;
        }
      }

      return 0;
    };

    auto is_full = [](const Board& b)
    {
      for (nat_t i = 0; i < 9; ++i)
      {
        if (b[i] == 0)
        {
          return false;
        }
      }

      return true;
    };

    auto is_terminal = [&](const Board& b)
    { return winner(b) != 0 || is_full(b); };

    auto evaluate = [&](const Board& b) -> real_t
    {
      int_t w = winner(b);

      if (w == 1)
      {
        return 1.0;
      }

      if (w == 2)
      {
        return -1.0;
      }

      return 0.0;
    };

    // Whose turn it is at any board is derived from the board itself
    // (X moves first, so an even count of filled cells means X is to
    // move) rather than captured from an outer loop variable — the
    // search recurses through many simulated plies, alternating
    // players at every one of them, so get_children must determine
    // the mover from the *state it is currently expanding*, not from
    // whichever real-world turn happened to be in progress when the
    // top-level search began.
    auto player_to_move = [](const Board& b) -> int_t
    {
      nat_t filled = 0;

      for (nat_t i = 0; i < 9; ++i)
      {
        if (b[i] != 0)
        {
          ++filled;
        }
      }

      return (filled % 2 == 0) ? 1 : 2;
    };

    auto get_children = [&](const Board& b) -> DynArray<Board>
    {
      DynArray<Board> result;

      if (is_terminal(b))
      {
        return result;
      }

      int_t player = player_to_move(b);

      for (nat_t i = 0; i < 9; ++i)
      {
        if (b[i] == 0)
        {
          Board next = b;
          next[i] = player;
          result.append(next);
        }
      }

      return result;
    };

    Board board(9, int_t(0));
    int_t current_player = 1; // X moves first

    nat_t moves_played = 0;

    while (!is_terminal(board) && moves_played < 9)
    {
      bool maximizing = current_player == 1;

      MoveSearchResult<Board> result =
          find_best_move<Board>(board, 9, maximizing, get_children, is_terminal, evaluate);

      assert(result.has_move);
      board = result.best_state;
      current_player = (current_player == 1) ? 2 : 1;
      ++moves_played;
    }

    assert(is_terminal(board));
    assert(winner(board) == 0); // perfect play on both sides -> draw

    cout << "Tic-Tac-Toe: perfect play by both sides ends in a draw Everything ok!\n";
  }

  cout << "Everything ok!\n";
  return 0;
}
