/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <iostream>

using namespace std;

#include <array.hpp>

using namespace Designar;

/** Not a new algorithm — a walkthrough of the three standard amortized-
    analysis techniques (aggregate, accounting/banker's, potential)
    against a mechanism this library already has: DynArray::append()'s
    geometric-growth reallocation (array.hpp). Every append is O(1)
    except the rare one that triggers a reallocation, which costs O(new
    capacity) to move every surviving element — the textbook setup every
    one of these three techniques exists to analyze. */
int main()
{
    const nat_t N = 1000;

    cout << "Amortized analysis of DynArray::append(), three ways\n"
         << "=====================================================\n\n";

    cout << "1) Aggregate method\n"
         << "-------------------\n"
         << "Sum the real cost of all N operations, then divide by N.\n\n";
    {
        DynArray<int_t> a;
        nat_t total_real_cost = 0;
        nat_t num_reallocations = 0;

        for (nat_t i = 0; i < N; ++i)
        {
            nat_t cap_before = a.get_capacity();
            a.append(int_t(i));
            nat_t cap_after = a.get_capacity();

            if (cap_after != cap_before)
            {
                // A reallocation moves every element that existed just
                // before it into the new buffer: O(old capacity) work,
                // not O(1).
                total_real_cost += cap_before + 1;
                ++num_reallocations;
            }
            else
            {
                total_real_cost += 1;
            }
        }

        cout << N << " appends triggered " << num_reallocations
             << " reallocations.\n"
             << "Total real cost (sum of every append's actual work): "
             << total_real_cost << "\n"
             << "Average cost per append: " << real_t(total_real_cost) / N
             << " -- O(1) amortized, even though individual reallocating\n"
             << "appends cost far more than O(1) on their own.\n\n";
    }

    cout << "2) Accounting (banker's) method\n"
         << "-------------------------------\n"
         << "Charge every append a fixed amortized price, comfortably\n"
         << "above its true O(1) unamortized cost; the surplus from every\n"
         << "cheap append accumulates as prepaid credit, which a\n"
         << "reallocation then spends to cover its real O(new capacity)\n"
         << "cost. The charge is only valid if the credit balance never\n"
         << "goes negative.\n\n";
    {
        DynArray<int_t> b;
        int_t credit_balance = 0;
        const int_t AMORTIZED_CHARGE = 5;
        bool balance_ever_negative = false;

        for (nat_t i = 0; i < N; ++i)
        {
            nat_t cap_before = b.get_capacity();
            b.append(int_t(i));
            nat_t cap_after = b.get_capacity();

            // A reallocation moves every element that existed just
            // before it (cap_before of them); every other append is O(1).
            int_t real_cost =
                (cap_after != cap_before) ? int_t(cap_before) + 1 : 1;
            credit_balance += AMORTIZED_CHARGE - real_cost;

            if (credit_balance < 0)
            {
                balance_ever_negative = true;
            }
        }

        cout << "Amortized charge per append: " << AMORTIZED_CHARGE << "\n"
             << "Final credit balance after " << N
             << " appends: " << credit_balance << "\n"
             << "Balance ever went negative: "
             << (balance_ever_negative ? "yes (charge too low!)" : "no")
             << "\n\n";
    }

    cout << "3) Potential method\n"
         << "-------------------\n"
         << "Define a potential Phi(i) after the i-th append -- credit\n"
         << "stored in the data structure's current shape, not paid to any\n"
         << "particular past operation -- such that a reallocation's real\n"
         << "cost is (mostly) cancelled by the resulting *drop* in Phi\n"
         << "(a freshly-grown array has more slack capacity relative to\n"
         << "its item count, so Phi goes down right when the real cost\n"
         << "spikes), while every cheap append's real cost is (mostly)\n"
         << "offset by a small *rise* in Phi (capacity holds steady while\n"
         << "num_items creeps up toward it again). The classic textbook\n"
         << "choice for a doubling array (CLRS 17.4) is\n"
         << "Phi = 2*num_items - capacity; DynArray's real growth (a\n"
         << "gentler 1.4x factor, an integer-rounded formula, and a\n"
         << "MIN_SIZE=32 floor -- see array.hpp) does not reduce to that\n"
         << "exact closed form, but the same idea -- a potential that\n"
         << "rises with slack and drops on reallocation -- is what makes\n"
         << "real_cost(i) + Phi(i) - Phi(i-1) come out to a constant\n"
         << "amortized cost either way. That constant is precisely what\n"
         << "the accounting method's charge above (5) already stands for:\n"
         << "an amortized cost the potential method arrives at via a\n"
         << "different bookkeeping device, not a different answer.\n";

    return 0;
}
