/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

/** @file neuralnetwork.hpp
    @brief NeuralNetwork: a feedforward multi-layer perceptron trained by
    per-example backpropagation (plain gradient descent, mean-squared-
    error loss) — no momentum/Adam/batching, kept to the textbook
    algorithm so what it computes is easy to verify by hand. Built on
    `Matrix<real_t>`/`Vector<real_t>` (linearalgebra.hpp): one weight
    matrix and one bias vector per layer transition.
    @ingroup ArtificialIntelligence
*/

#pragma once

#include <cmath>
#include <utility>

#include <types.hpp>
#include <array.hpp>
#include <random.hpp>
#include <linearalgebra.hpp>

namespace Designar
{
    /** The default activation: `1 / (1 + e^-z)`. Its derivative is
        expressed in terms of the *activated* value `a = sigmoid(z)`
        (`a * (1 - a)`) rather than `z` itself — the standard trick that
        lets backpropagation reuse the forward pass's already-computed
        activations instead of caching raw pre-activation values
        separately. */
    struct Sigmoid
    {
        real_t operator()(real_t z) const
        {
            return 1 / (1 + std::exp(-z));
        }

        real_t derivative(real_t activated_value) const
        {
            return activated_value * (1 - activated_value);
        }
    };

    /** An alternative activation, `tanh(z)`, output range `(-1, 1)`
        instead of Sigmoid's `(0, 1)` — sometimes trains faster since its
        output is zero-centered. Derivative likewise expressed via the
        activated value: `1 - tanh(z)^2 == 1 - a^2`. */
    struct Tanh
    {
        real_t operator()(real_t z) const
        {
            return std::tanh(z);
        }

        real_t derivative(real_t activated_value) const
        {
            return 1 - activated_value * activated_value;
        }
    };

    /** `Activation` is a compile-time policy (like this codebase's
        `Cmp` comparator parameters elsewhere), not a runtime choice —
        `Sigmoid`/`Tanh` above, or any type providing the same
        `operator()(real_t) const` / `derivative(real_t) const` pair. */
    template <class Activation = Sigmoid>
    class NeuralNetwork
    {
        Activation activation;
        DynArray<nat_t> layer_sizes;

        /** `weights[l]` maps layer `l`'s activations to layer `l + 1`'s
            pre-activation values: shape `(layer_sizes[l + 1] x
            layer_sizes[l])`, so `z^{l+1} = weights[l] * a^l +
            biases[l]`. */
        DynArray<Matrix<real_t>> weights;
        DynArray<Vector<real_t>> biases;

        /** `activations[l]` — cached by every forward() call (`a^0` is
            just the input; `a^L`, the last entry, is the network's
            output) so train_step()'s backward pass can reuse them
            instead of recomputing. */
        DynArray<Vector<real_t>> activations;

        rng_t rng;

        void randomize_layer(nat_t l)
        {
            Matrix<real_t>& w = weights[l];

            for (nat_t i = 0; i < w.num_rows(); ++i)
            {
                for (nat_t j = 0; j < w.num_cols(); ++j)
                {
                    w(i, j) = random_uniform<real_t>(rng, -1.0, 1.0);
                }
            }

            Vector<real_t>& b = biases[l];

            for (nat_t i = 0; i < b.size(); ++i)
            {
                b[i] = random_uniform<real_t>(rng, -1.0, 1.0);
            }
        }

    public:
        /** `sizes` must have at least 2 entries (an input and an output
            layer); e.g. `{2, 4, 1}` is the classic XOR-solving shape
            (2 inputs, one 4-neuron hidden layer, 1 output). Weights and
            biases start as small random values in `[-1, 1]`. */
        NeuralNetwork(const DynArray<nat_t>& sizes,
                     rng_seed_t seed = get_random_seed())
            : layer_sizes(sizes), rng(seed)
        {
            if (layer_sizes.size() < 2)
            {
                throw std::domain_error(
                    "NeuralNetwork: needs at least an input and an output "
                    "layer");
            }

            for (nat_t l = 0; l + 1 < layer_sizes.size(); ++l)
            {
                weights.append(
                    Matrix<real_t>(layer_sizes[l + 1], layer_sizes[l]));
                biases.append(Vector<real_t>(layer_sizes[l + 1], real_t()));
                randomize_layer(l);
            }

            for (nat_t l = 0; l < layer_sizes.size(); ++l)
            {
                activations.append(Vector<real_t>(layer_sizes[l], real_t()));
            }
        }

        /** Feeds `input` through every layer, caching each layer's
            activation along the way, and returns the output layer's
            activation (a reference into this object's own cache — valid
            until the next forward()/train_step() call). */
        const Vector<real_t>& forward(const Vector<real_t>& input)
        {
            if (input.size() != layer_sizes[0])
            {
                throw std::domain_error(
                    "NeuralNetwork::forward: input size mismatch");
            }

            activations[0] = input;

            for (nat_t l = 0; l + 1 < layer_sizes.size(); ++l)
            {
                const Matrix<real_t>& w = weights[l];
                const Vector<real_t>& b = biases[l];
                Vector<real_t> a(layer_sizes[l + 1], real_t());

                for (nat_t i = 0; i < w.num_rows(); ++i)
                {
                    real_t z = b[i];

                    for (nat_t j = 0; j < w.num_cols(); ++j)
                    {
                        z += w(i, j) * activations[l][j];
                    }

                    a[i] = activation(z);
                }

                activations[l + 1] = std::move(a);
            }

            return activations[layer_sizes.size() - 1];
        }

        /** One step of backpropagation on a single `(input, target)`
            example: forward pass, output-layer error
            `output - target` scaled by the output activation's
            derivative, then propagated backward one layer at a time
            (`delta^l = (weights[l]^T * delta^{l+1}) .*
            activation'(a^l)`, the standard chain-rule recurrence),
            finally a plain gradient-descent update of every weight
            (`dL/dW[i][j] = delta_i * a_prev_j`) and bias
            (`dL/db[i] = delta_i`). Returns the mean-squared error for
            this example (so a training loop can print/plot it
            decreasing). */
        real_t train_step(const Vector<real_t>& input,
                          const Vector<real_t>& target, real_t learning_rate)
        {
            const Vector<real_t>& output = forward(input);

            if (target.size() != output.size())
            {
                throw std::domain_error(
                    "NeuralNetwork::train_step: target size mismatch");
            }

            nat_t L = layer_sizes.size() - 1;

            DynArray<Vector<real_t>> deltas;

            for (nat_t l = 0; l < layer_sizes.size(); ++l)
            {
                deltas.append(Vector<real_t>(layer_sizes[l], real_t()));
            }

            real_t loss = 0;

            for (nat_t i = 0; i < output.size(); ++i)
            {
                real_t error = output[i] - target[i];
                loss += 0.5 * error * error;
                deltas[L][i] = error * activation.derivative(output[i]);
            }

            for (int_t l = int_t(L) - 1; l >= 1; --l)
            {
                const Matrix<real_t>& w = weights[l];
                const Vector<real_t>& next_delta = deltas[l + 1];
                Vector<real_t>& delta = deltas[l];

                for (nat_t j = 0; j < w.num_cols(); ++j)
                {
                    real_t sum = 0;

                    for (nat_t i = 0; i < w.num_rows(); ++i)
                    {
                        sum += w(i, j) * next_delta[i];
                    }

                    delta[j] = sum * activation.derivative(activations[l][j]);
                }
            }

            for (nat_t l = 0; l < L; ++l)
            {
                Matrix<real_t>& w = weights[l];
                Vector<real_t>& b = biases[l];
                const Vector<real_t>& delta = deltas[l + 1];
                const Vector<real_t>& prev_activation = activations[l];

                for (nat_t i = 0; i < w.num_rows(); ++i)
                {
                    for (nat_t j = 0; j < w.num_cols(); ++j)
                    {
                        w(i, j) -= learning_rate * delta[i] *
                                   prev_activation[j];
                    }

                    b[i] -= learning_rate * delta[i];
                }
            }

            return loss;
        }

        /** Repeats train_step() over every `(input, target)` pair in
            `examples`, `epochs` times, visiting the examples in a fresh
            random order each epoch (plain SGD, not mini-batched). */
        void train(const DynArray<std::pair<Vector<real_t>, Vector<real_t>>>&
                       examples,
                   nat_t epochs, real_t learning_rate)
        {
            DynArray<nat_t> order;

            for (nat_t i = 0; i < examples.size(); ++i)
            {
                order.append(i);
            }

            for (nat_t e = 0; e < epochs; ++e)
            {
                for (nat_t i = order.size() - 1; i > 0; --i)
                {
                    nat_t j = random_uniform<nat_t>(rng, i + 1);
                    std::swap(order[i], order[j]);
                }

                for (nat_t idx : order)
                {
                    train_step(examples[idx].first, examples[idx].second,
                              learning_rate);
                }
            }
        }

        const DynArray<nat_t>& sizes() const
        {
            return layer_sizes;
        }
    };

} // end namespace Designar
