#pragma once

#include <cassert>
#include <concepts>
#include <cstddef>
#include <type_traits>
#include <utility>

#include "set_theory.hpp"

namespace eulerus::number_theory {
    /* -------------------------------------------------------------------------- */
    /*                    Basic Integer Number Theory Functions                   */
    /* -------------------------------------------------------------------------- */
    
    // Return `x` modulo `m`, ensuring the result is between 0 and `m`
    template <std::integral T>
    inline T mod(std::integral auto x, T m) {
        assert(m != 0);

        if (m < 0) {
            if (x < 0) {
                return x % m;
            } else {
                if (x % -m == 0) return 0;
                return (T)(x % -m) + m;
            }
        } else {
            if (x < 0) {
                if (-x % m == 0) return 0;
                return m - (-x % m);
            } else {
                return x % m;
            }
        }
    }
    
    // Return `x` modulo `m`, ensuring the result is between 0 and `m`, and store the associated quotient in `q`
    inline auto mod(std::integral auto x, std::integral auto m, std::integral auto& q) {
        assert(m != 0);
        
        auto r = mod(x, m);

        if ((x < 0) == (m < 0)) {
            q = x / m;
        } else {
            if (x < 0) {
                q = (-x) / m;
            } else {
                q = x / (-m);
            }

            q = -q;
            if (r != 0) q -= 1;
        }

        return r;
    }

    // Check if `a` divides `b` (`b` is a multiple of `a`)
    inline bool divides(std::integral auto a, std::integral auto b) {
        if (a == 0 && b == 0) return true;
        if (b == 0) return true;
        if (a == 0) return false;
        return b % a == 0;
    }

    /* -------------------------------------------------------------------------- */
    /*                           Prime Number Functions                           */
    /* -------------------------------------------------------------------------- */

    // Check if `x` is a prime number
    inline bool is_prime(std::integral auto x) {
        if (x <= 1) return false;
        if (x == 2 || x == 3) return true;
        if (x % 2 == 0 || x % 3 == 0) return false;

        // Check if numbers of the form 6n-1 or 6n+1, that are less than the square root of `x`, divide `x`
        for (decltype(x) i = 5; i * i < x; i += 6) {
            if (x % i == 0 || x % (i + 2) == 0) return false;
        }

        return true;
    }

    // Check if `x` is a prime number, and store its lowest prime factor in `lpf`
    inline bool is_prime(std::integral auto x, std::integral auto& lpf) {
        assert(x > 1);

        if (x == 2 || x == 3) { lpf = x; return true; }
        if (x % 2 == 0) { lpf = 2; return false; }
        if (x % 3 == 0) { lpf = 3; return false; }

        // Check if numbers of the form 6n-1 or 6n+1, that are less than the square root of `x`, divide `x`
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

        decltype(x) result = 1;
        decltype(x) lpf;

        while (x > 1) { 
            if (is_prime(x, lpf)) {
                result *= x - 1;
                break;
            } else {
                // Use the formula: phi(n) = p1^(k1-1) * (p1 - 1) * p2^(k2-1) * (p2 - 1) * ..., for the prime factorization n = p1^k1 * p2^k2 * ...
                result *= lpf - 1;
                x /= lpf;

                while (x % lpf == 0) {
                    result *= lpf;
                    x /= lpf;
                }
            }
        }

        return result;
    }

    // Return a set containing the prime numbers that are a factor of `x`
    inline auto prime_factors(std::integral auto x) {
        assert(x > 1);

        set_theory::TypedSet<decltype(x)> factors;
        decltype(x) lpf; 

        while (x > 1) {
            if (is_prime(x, lpf)) {
                factors.insert(x);
                break;
            } else {
                factors.insert(lpf);

                while (x % lpf == 0) {
                    x /= lpf;
                }
            }
        }

        return factors;
    }

    /* -------------------------------------------------------------------------- */
    /*                           Divisors and Multiples                           */
    /* -------------------------------------------------------------------------- */

    // Return a set containing the integers that divide `x`
    inline auto divisors(std::integral auto x) {
        assert(x != 0);
        if (x < 0) x *= -1;

        set_theory::TypedSet<decltype(x)> divisors;
        
        for (decltype(x) i = 1; i * i <= x; i++) {
            if (x % i == 0) {
                divisors.insert(i);
                divisors.insert(-i);
                divisors.insert(x / i);
                divisors.insert(-x / i);
            }
        }

        return divisors;
    }

    // Return a set containing the positive integers that divide `x`
    inline auto positive_divisors(std::integral auto x) {
        assert(x != 0);
        if (x < 0) x *= -1;

        set_theory::TypedSet<decltype(x)> divisors;
        
        for (decltype(x) i = 1; i * i <= x; i++) {
            if (x % i == 0) {
                divisors.insert(i);
                divisors.insert(x / i);
            }
        }

        return divisors;
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
        while (modulus != 0) {
            d = std::exchange(modulus, mod(d, modulus));
        }

        return d;
    }

    // Return the greatest common divisor of `a` and `b`, and store the Bézout coefficients for `a` and `b` in `x` and `y` respectively
    inline auto gcd(std::integral auto a, std::integral auto b, std::integral auto& x, std::integral auto& y) {
        using Common = std::common_type_t<decltype(a), decltype(b), decltype(x), decltype(y)>;

        if (a == 0 && b == 0) {
            x = 0;
            y = 0;
            return Common(0);
        }

        if (a < 0) a *= -1;
        if (b < 0) b *= -1;

        Common d = b > a ? b : a;
        Common modulus = b > a ? a : b;

        Common q;
        Common x_previous = 1;
        Common y_previous = 0;
        Common x_current = 0;
        Common y_current = 1;
        
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

        // Calculate the lcm using the relationship between gcd and lcm
        return (Common(a) / gcd(a, b)) * Common(b);
    }

    /* -------------------------------------------------------------------------- */
    /*                        Linear Diophantine Equations                        */
    /* -------------------------------------------------------------------------- */

    // Check if the linear Diophantine equation `ax + by = c` has integer solution pairs (x, y)
    inline bool is_lde_solvable(std::integral auto a, std::integral auto b, std::integral auto c) {
        if (a == 0 && b == 0) return c == 0;
        if (a == 0) return divides(b, c);
        if (b == 0) return divides(a, c);

        return c % gcd(a, b) == 0;
    }

    // Check if the linear Diophantine equation `ax + by = c` has integer solution pairs (x, y), and store one of the solution pairs in `x_0` and `y_0`
    inline bool is_lde_solvable(std::integral auto a, std::integral auto b, std::integral auto c, std::integral auto& x_0, std::integral auto& y_0) {
        using Common = std::common_type_t<decltype(a), decltype(b), decltype(c), decltype(x_0), decltype(y_0)>;
        
        if (a == 0 && b == 0) { x_0 = 0; y_0 = 0; return c == 0; }
        if (a == 0) { x_0 = 0; y_0 = c / b; return divides(b, c); }
        if (b == 0) { x_0 = c / a; y_0 = 0; return divides(a, c); }

        // Use Bézout's identity to find a solution to the equation `ax + by = gcd(a, b)`, then scale the solution to satisfy `ax + by = c`
        Common d = gcd(a, b, x_0, y_0);
        x_0 *= c / d;
        y_0 *= c / d;

        return c % gcd(a, b) == 0;
    }

    /* -------------------------------------------------------------------------- */
    /*                          Modular Arithmetic Class                          */
    /* -------------------------------------------------------------------------- */

    // Class representing a congruence class modulo `M`
    template <std::size_t M>
    class Congruence {
        public:
            std::size_t value;

            // Constructor a congruence class with a value of 0 by default
            Congruence() : value(0) {}

            // Construct a congruence class with a value between 0 and `M`
            Congruence(std::integral auto x) : value(mod(x, M)) {}

            // Return the modular inverse of the congruence class, if it exists
            Congruence inverse() const {
                std::size_t x, y;
                
                if (value != 0 && is_lde_solvable(value, M, 1, x, y)) {
                    return x;
                } else {
                    throw std::runtime_error("No modular inverse exists for " + std::to_string(value) + " modulo " + std::to_string(M));
                }
            }

            // Raise the congruence class to the power of `exp`
            Congruence pow(std::integral auto exp) const {
                if (exp < 0) {
                    return inverse().pow(-exp);
                }

                std::size_t result = 1;

                for (std::size_t i = 0; i < exp; i++) {
                    result = mod(result * value, M);
                }

                return result;
            }

            // Raise the congruence class to the power of another congruence class
            Congruence pow(const Congruence& other) const {
                return pow(other.value);
            }

            // Convert the congruence class to a numeric value
            operator std::size_t() const {
                return value;
            }

            // Check if two congruence classes are equal
            bool operator==(const Congruence& other) const {
                return value == other.value;
            }

            // Add two congruence classes together
            Congruence operator+(const Congruence& other) const {
                return value + other.value;
            }

            // Add an integer to a congruence class
            Congruence operator+(std::integral auto x) const {
                return value + mod(x, M);
            }

            // Increment the congruence class by another congruence class
            Congruence& operator+=(const Congruence& other) {
                value = mod(value + other.value, M);
                return *this;
            }

            // Increment the congruence class by an integer
            Congruence& operator+=(std::integral auto x) {
                value = mod(value + mod(x, M), M);
                return *this;
            }

            // Subtract one congruence class from another
            Congruence operator-(const Congruence& other) const {
                return value >= other.value ? value - other.value : M - (other.value - value);
            }

            // Subtract an integer from a congruence class
            Congruence operator-(std::integral auto x) const {
                std::size_t remainder = mod(x, M);
                return value >= remainder ? value - remainder : M - (remainder - value);
            }

            // Decrement the congruence class by another congruence class
            Congruence& operator-=(const Congruence& other) {
                value = mod(value >= other.value ? value - other.value : M - (other.value - value), M);
                return *this;
            }

            // Decrement the congruence class by an integer
            Congruence& operator-=(std::integral auto x) {
                std::size_t remainder = mod(x, M);
                value = mod(value >= remainder ? value - remainder : M - (remainder - value), M);
                return *this;
            }

            // Multiply two congruence classes together
            Congruence operator*(const Congruence& other) const {
                return value * other.value;
            }

            // Multiply a congruence class by an integer
            Congruence operator*(std::integral auto x) const {
                return value * mod(x, M);
            }

            // Multiply the congruence class in-place by another congruence class
            Congruence& operator*=(const Congruence& other) {
                value = mod(value * other.value, M);
                return *this;
            }

            // Multiply the congruence class in-place by an integer
            Congruence& operator*=(std::integral auto x) {
                value = mod(value * mod(x, M), M);
                return *this;
            }

            // Divide one congruence class by another, if the divisor has a modular inverse
            Congruence operator/(const Congruence& other) const {
                return value * other.inverse().value;
            }

            // Divide a congruence class by an integer, if the divisor has a modular inverse
            Congruence operator/(std::integral auto x) const {
                return value * Congruence(x).inverse().value;
            }

            // Divide the congruence class in-place by another congruence class, if the divisor has a modular inverse
            Congruence& operator/=(const Congruence& other) {
                value = mod(value * other.inverse().value, M);
                return *this;
            }

            // Divide the congruence class in-place by an integer, if the divisor has a modular inverse
            Congruence& operator/=(std::integral auto x) {
                value = mod(value * Congruence(x).inverse().value, M);
                return *this;
            }
    };
}