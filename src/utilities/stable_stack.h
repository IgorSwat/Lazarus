#pragma once

#include "stable_vector.h"


// ---------------------------
// Stable stack implementation
// ---------------------------

template <typename InnerContainer, typename T>
class __stable_stack
{
public:
    // Constructor - only default construction enabled
    __stable_stack() = default;

    // Modifiers - adding / removing elements
    void push() { container.push_back(); }
    void push(const T& element) { container.push_back(element); }
    void pop() { container.pop_back(); }
    void shrink() { container.resize(1); }      // Reduces (logically) stack to only one (root) element
    void clear() { container.clear(); }         // Removes (logically) all the elements

    // Getters - accessing elements
    T& top() { return container.back(); }
    const T& top() const { return container.back(); }
    T& top_n(int n) { return container[container.size() - n - 1]; }                        // n-th element from the top (0 = top element)
    const T& top_n(int n) const { return container[container.size() - n - 1]; }            // n-th element from the top (0 = top element)
    int size() const { return container.size(); }
    bool empty() const { return container.empty(); }
    bool full() const { return container.full(); }

private:
    // Data container
    InnerContainer container;
};


// --------------------------------
// Stable stack - public definition
// --------------------------------

template <memory::Storage storage, typename T, int size>
using StableStack = std::conditional<
    storage == memory::Storage::STATIC,
    __stable_stack<__static_vector<T, size>, T>,
    __stable_stack<__dynamic_vector<T, size>, T>
>::type;