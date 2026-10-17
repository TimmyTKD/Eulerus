#pragma once

#include <cassert>
#include <concepts>
#include <type_traits>
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
    
    // Return `x` modulo `m`, ensuring the result is between 0 and `m`, and store the associated quotient in `q`
    inline auto mod(std::integral auto x, std::integral auto m, std::integral auto& q) {
        assert(m != 0);

        auto r = x % m;
        if ((x < 0 != m < 0) && r != 0) r += m;

        q = (x - r) / m;

        return r;
    }

    // Check if `a` divides `b` (`b` is a multiple of `a`)
    inline bool divides(std::integral auto a, std::integral auto b) {
        if (a == 0 && b == 0) return true;
        if (b == 0) return true;
        if (a == 0) return false;
        return mod(b, a) == 0;
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

    // Check if `x` is a prime number, and store its lowest prime factor in `lpf`
    inline bool is_prime(std::integral auto x, std::integral auto& lpf) {
        if (x < 0) { is_prime(-x, lpf); return false; }
        if (x == 0 || x == 1) { lpf = x; return false; }
        if (x == 2 || x == 3) { lpf = x; return true; }
        if (x % 2 == 0) { lpf = 2; return false; }
        if (x % 3 == 0) { lpf = 3; return false; }

        for (decltype(x) i = 5; i * i <= x; i += 6) {
            if (x % i == 0) { lpf = i; return false; };
            if (x % (i + 2) == 0) { lpf = i + 2; return false; };
        }
        
        lpf = x;
        return true;
    }

    // Return the number of positive integers less than or equal to `x` that are coprime with `x`
    inline auto totient(std::integral auto x) {
        assert(x > 0);

        if (x == 1) return x;

        decltype(x) lpf;
        if (is_prime(x, lpf)) return x - 1;
        
        decltype(x) lpf_totient = lpf - 1;
        x /= lpf;

        while (x % lpf == 0) {
            lpf_totient *= lpf;
            x /= lpf;
        }

        return lpf_totient * totient(x);
    }

    // Return the greatest common divisor of `a` and `b`
    inline auto gcd(std::integral auto a, std::integral auto b) {
        using Common = std::common_type_t<decltype(a), decltype(b)>;

        if (a == 0 && b == 0) return Common(0);
        if (a < 0) a *= -1;
        if (b < 0) b *= -1;

        Common d = b > a ? b : a;
        Common modulus = b > a ? a : b;
        
        // Calculate the gcd using the Euclidean algorithm
        while (modulus != 0) d = std::exchange(modulus, mod(d, modulus));
        return d;
    }

    // Return the greatest common divisor of `a` and `b`, and store the Bézout coefficients for `a` and `b` in `x` and `y` respectively
    inline auto gcd(std::integral auto a, std::integral auto b, std::integral auto& x, std::integral auto& y) {
        using Common = std::common_type_t<decltype(a), decltype(b), decltype(x), decltype(y)>;

        if (a == 0 && b == 0) {
            x = Common(0);
            y = Common(0);
            return Common(0);
        }

        if (a < 0) a *= -1;
        if (b < 0) b *= -1;

        Common d = b > a ? b : a;
        Common modulus = b > a ? a : b;

        Common q;
        Common x_previous = Common(1);
        Common y_previous = Common(0);
        Common x_current = Common(0);
        Common y_current = Common(1);
        
        // Calculate the gcd and Bézout coefficients using the extended Euclidean algorithm
        while (modulus != 0) {
            d = std::exchange(modulus, mod(d, modulus, q));
            x_previous = std::exchange(x_current, x_previous - q * x_current);
            y_previous = std::exchange(y_current, y_previous - q * y_current);
        }

        x_current = x_previous;
        y_current = y_previous;

        if (b > a) std::swap(x_current, y_current);
        if (a < 0) x_current *= -1;
        if (b < 0) y_current *= -1;

        x = x_current;
        y = y_current;

        return d;
    }

    // Return the least common multiple of `a` and `b`
    inline auto lcm(std::integral auto a, std::integral auto b) {
        using Common = std::common_type_t<decltype(a), decltype(b)>;

        if (a == 0 && b == 0) return Common(0);
        if (a < 0) a *= -1;
        if (b < 0) b *= -1;

        return (Common(a) / gcd(a, b)) * Common(b);
    }
}