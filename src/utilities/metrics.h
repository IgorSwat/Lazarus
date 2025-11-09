#pragma once

#include <bit>
#include <concepts>
#include <numeric>
#include <span>

namespace utilities::metrics {

    // -----------------
    // Metrics - Hamming
    // -----------------

    // Hamming metric calculation function
    template<std::integral T>
    int hamming(T a, T b)
    {
        return std::popcount(static_cast<std::make_unsigned_t<T>>(a ^ b));
    }

}