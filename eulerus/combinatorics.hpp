#pragma once

#include <any>
#include <cassert>
#include <unordered_set>

namespace eulerus::combinatorics {
    /* -------------------------------------------------------------------------- */
    /*                       Integer Combinatorics Functions                       */
    /* -------------------------------------------------------------------------- */

    // Calculate the factorial of `n` (or `n!`)
    inline unsigned long long factorial(int n) {
        assert(n >= 0);

        if (n == 0 || n == 1) return 1;

        // Note: this value will likely overflow for large `n`
        unsigned long long result = 1;
        for (int i = 2; i <= n; i++) {
            result *= i;
        }

        return result;
    }

    // Calculate the number of `r`-combinations of `n` (or `n` choose `r`)
    inline unsigned long long nCr(int n, int r) {
        assert(n >= 0);
        assert(r >= 0);

        if (r > n) return 0;
        if (r == 0 || r == n) return 1;
        if (r > n / 2) r = n - r;
        
        // Note: this value will likely overflow for large `n`
        unsigned long long result = 1;
        for (int i = 1; i <= r; i++) {
            result *= n + 1 - i;
            result /= i;
        }

        return result;
    }

    // Calculate the number of `r`-permutations of `n` (or `n` permute `r`)
    inline unsigned long long nPr(int n, int r) {
        assert(n >= 0);
        assert(r >= 0);
        
        if (r > n) return 0;
        if (r == 0) return 1;
        if (r == n) return factorial(n);
        
        // Note: this value will likely overflow for large `n`
        unsigned long long result = 1;
        for (int i = n - r + 1; i <= n; i++) {
            result *= i;
        }

        return result;
    }

    /* -------------------------------------------------------------------------- */
    /*                                    Sets                                    */
    /* -------------------------------------------------------------------------- */

    /**
     * @brief Mathematical set class that can hold elements of different types
     * 
     */
    class Set {
        public:
            // Construct an empty set
            Set() = default;

            // Construct a set with elements of different types
            template <typename... Types>
            Set(Types... args) : _elements{args...} {}  

            // Return the union of this set and another set
            Set merge(const Set& other) const {
                Set result = *this;
                result._elements.insert(other._elements.begin(), other._elements.end());
                return result;
            }

            // Return the intersection of this set and another set
            Set intersect(const Set& other) const {
                Set result;

                for (const auto& element : _elements) {
                    if (other._elements.contains(element)) {
                        result._elements.insert(element);
                    }
                }

                return result;
            }

        private:
            std::unordered_set<std::any> _elements;
    };

    
}