/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <cassert>
#include <cmath>
#include <neuralnetwork.hpp>

using namespace Designar;

int main()
{
    DynArray<std::pair<Vector<real_t>, Vector<real_t>>> examples;
    examples.append({Vector<real_t>{0.0, 0.0}, Vector<real_t>{0.0}});
    examples.append({Vector<real_t>{0.0, 1.0}, Vector<real_t>{1.0}});
    examples.append({Vector<real_t>{1.0, 0.0}, Vector<real_t>{1.0}});
    examples.append({Vector<real_t>{1.0, 1.0}, Vector<real_t>{0.0}});

    // A single train_step() must decrease the loss on a fixed example —
    // catches a backprop sign/indexing error even in a run that would
    // otherwise stumble into a correct-looking final answer by luck.
    {
        NeuralNetwork<Sigmoid> nn({2, 4, 1}, 20260718);
        real_t loss1 = nn.train_step(examples[0].first, examples[0].second,
                                     0.5);
        real_t loss2 = nn.train_step(examples[0].first, examples[0].second,
                                     0.5);
        assert(loss2 < loss1);
    }

    // The canonical XOR problem: not linearly separable, so correctly
    // solving it end to end after training is a genuine test of
    // backpropagation through the hidden layer, not a trivially
    // separable set a broken-but-lucky implementation could still fit.
    {
        NeuralNetwork<Sigmoid> nn({2, 4, 1}, 20260718);
        nn.train(examples, 5000, 0.5);

        for (const auto& example : examples)
        {
            const Vector<real_t>& output = nn.forward(example.first);
            assert(std::abs(output[0] - example.second[0]) < 0.1);
        }
    }

    // Tanh activation policy: same architecture/problem, different
    // activation — exercises that Activation really is swappable.
    {
        NeuralNetwork<Tanh> nn({2, 4, 1}, 20260718);
        DynArray<std::pair<Vector<real_t>, Vector<real_t>>> tanh_examples;
        tanh_examples.append({Vector<real_t>{0.0, 0.0}, Vector<real_t>{-1.0}});
        tanh_examples.append({Vector<real_t>{0.0, 1.0}, Vector<real_t>{1.0}});
        tanh_examples.append({Vector<real_t>{1.0, 0.0}, Vector<real_t>{1.0}});
        tanh_examples.append({Vector<real_t>{1.0, 1.0}, Vector<real_t>{-1.0}});

        nn.train(tanh_examples, 5000, 0.1);

        for (const auto& example : tanh_examples)
        {
            const Vector<real_t>& output = nn.forward(example.first);
            assert(std::abs(output[0] - example.second[0]) < 0.2);
        }
    }

    // Mismatched input/target sizes must throw, not silently misbehave.
    {
        NeuralNetwork<Sigmoid> nn({2, 3, 1});

        try
        {
            nn.forward(Vector<real_t>{0.0, 0.0, 0.0});
            assert(false);
        }
        catch (const std::domain_error&)
        {
            assert(true);
        }

        try
        {
            nn.train_step(Vector<real_t>{0.0, 0.0}, Vector<real_t>{0.0, 0.0},
                         0.1);
            assert(false);
        }
        catch (const std::domain_error&)
        {
            assert(true);
        }
    }

    return 0;
}
