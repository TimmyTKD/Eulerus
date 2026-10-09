#pragma once

#include <concepts>
#include <utility>

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

    // Return the greatest common divisor of `a` and `b`
    template <std::integral T, std::integral T2>
    inline auto gcd(T a, T2 b) {
        if (a == 0 && b == 0) return 0;
        if (a < 0) a *= -1;
        if (b < 0) b *= -1;
        if (a < b) std::swap(a, b);

        while (b != 0) a = std::exchange(b, mod(a, b));
        return a;
    }

    // Return the least common multiple of `a` and `b`
    template <std::integral T, std::integral T2>
    inline auto lcm(T a, T2 b) {
        if (a == 0 && b == 0) return 0;
        if (a < 0) a *= -1;
        if (b < 0) b *= -1;
        return (a / gcd(a, b)) * b;
    }
}