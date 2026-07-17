/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <iostream>

using namespace std;

#include <minimax.hpp>

using namespace Designar;

namespace
{
    // Board: 9 cells, 0 = empty, 1 = X (maximizer), 2 = O (minimizer).
    using Board = DynArray<int_t>;

    int_t winner(const Board& b)
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
    }

    bool is_full(const Board& b)
    {
        for (nat_t i = 0; i < 9; ++i)
        {
            if (b[i] == 0)
            {
                return false;
            }
        }

        return true;
    }

    bool is_terminal(const Board& b)
    {
        return winner(b) != 0 || is_full(b);
    }

    real_t evaluate(const Board& b)
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
    }

    int_t player_to_move(const Board& b)
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
    }

    DynArray<Board> get_children(const Board& b)
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
    }

    void print_board(const Board& b)
    {
        for (nat_t r = 0; r < 3; ++r)
        {
            for (nat_t c = 0; c < 3; ++c)
            {
                int_t v = b[r * 3 + c];
                cout << (v == 0 ? '.' : (v == 1 ? 'X' : 'O'));
            }

            cout << endl;
        }
    }
} // end anonymous namespace

int main()
{
    // Two perfect players, driven entirely by find_best_move(): the
    // textbook guarantee is that optimal tic-tac-toe always ends in a
    // draw, regardless of who plays what.
    Board board(9, int_t(0));

    while (!is_terminal(board))
    {
        int_t player = player_to_move(board);
        bool maximizing = player == 1;

        MoveSearchResult<Board> result = find_best_move<Board>(
            board, 9, maximizing, get_children, is_terminal, evaluate);

        board = result.best_state;

        cout << "Player " << (player == 1 ? "X" : "O")
             << " moves (search score " << result.score << "):\n";
        print_board(board);
        cout << endl;
    }

    int_t w = winner(board);
    cout << (w == 0 ? "Draw." : (w == 1 ? "X wins." : "O wins.")) << endl;

    return 0;
}
