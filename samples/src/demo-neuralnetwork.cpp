/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <iostream>

using namespace std;

#include <neuralnetwork.hpp>

using namespace Designar;

int main()
{
    // The canonical XOR problem: not linearly separable, so a network
    // with no hidden layer provably cannot learn it — a real test that
    // backpropagation is training the hidden layer correctly.
    NeuralNetwork<Sigmoid> nn({2, 4, 1}, 20260718);

    DynArray<std::pair<Vector<real_t>, Vector<real_t>>> examples;
    examples.append({Vector<real_t>{0.0, 0.0}, Vector<real_t>{0.0}});
    examples.append({Vector<real_t>{0.0, 1.0}, Vector<real_t>{1.0}});
    examples.append({Vector<real_t>{1.0, 0.0}, Vector<real_t>{1.0}});
    examples.append({Vector<real_t>{1.0, 1.0}, Vector<real_t>{0.0}});

    nn.train(examples, 5000, 0.5);

    cout << "XOR truth table after training:\n";

    for (const auto& example : examples)
    {
        const Vector<real_t>& output = nn.forward(example.first);
        cout << example.first[0] << " XOR " << example.first[1]
             << " = " << output[0] << " (expected " << example.second[0]
             << ")\n";
    }

    return 0;
}
