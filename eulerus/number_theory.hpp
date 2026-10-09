#pragma once

#include <concepts>

namespace eulerus::number_theory {
    /* -------------------------------------------------------------------------- */
    /*                    Basic Integer Number Theory Functions                   */
    /* -------------------------------------------------------------------------- */
    
    // Return `x` modulo `m`, ensuring a non-negative result
    template <std::integral T, std::integral T2>
    inline T2 mod(T x, T2 m) {
        T2 r = x % m;
        if ((x < 0 != m < 0) && r != 0) r += m;
        return r;
    }
}