#pragma once

#include <cassert>
#include <concepts>
#include <utility>

namespace eulerus::number_theory {
    /* -------------------------------------------------------------------------- */
    /*                    Basic Integer Number Theory Functions                   */
    /* -------------------------------------------------------------------------- */
    
    // Return `x` modulo `m`, ensuring the result is between 0 and `m`
    inline auto mod(std::integral auto x, std::integral auto m) {
        assert(m != 0);

        auto r = x % m;
        if ((x < 0 != m < 0) && r != 0) r += m;

        return r;
    }

    // Check if `a` divides `b` (`b` is a multiple of `a`)
    inline bool divides(std::integral auto a, std::integral auto b) {
        if (a == 0 && b == 0) return true;
        if (b == 0) return true;
        if (a == 0) return false;
        return mod(a, b) == 0;
    }

    // Check if `x` is a prime number
    inline bool is_prime(std::integral auto x) {
        if (x <= 1) return false;
        if (x == 2 || x == 3) return true;
        if (x % 2 == 0 || x % 3 == 0) return false;

        for (decltype(x) i = 5; i * i < x; i += 6) {
            if (x % i == 0 || x % (i + 2) == 0) return false;
        }

        return true;
    }

    // Return the greatest common divisor of `a` and `b`
    inline auto gcd(std::integral auto a, std::integral auto b) {
        if (a == 0 && b == 0) return 0;
        if (a < 0) a *= -1;
        if (b < 0) b *= -1;
        if (a < b) std::swap(a, b);
        
        while (b != 0) a = std::exchange(b, mod(a, b));
        return a;
    }

    // Return the least common multiple of `a` and `b`
    inline auto lcm(std::integral auto a, std::integral auto b) {
        if (a == 0 && b == 0) return 0;
        if (a < 0) a *= -1;
        if (b < 0) b *= -1;
        return (a / gcd(a, b)) * b;
    }
}