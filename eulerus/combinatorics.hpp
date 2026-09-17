#pragma once

#include <any>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <functional>
#include <iostream>
#include <typeindex>
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

    // Helper struct to store elements of different types in a set
    struct SetElement {
        std::any value;
        std::type_index type_idx;
        bool (*equality_func)(const std::any& a, const std::any& b);
        void (*output_func)(std::ostream& os, const std::any& element);
        std::size_t hash;

        // Construct a SetElement from a value of any type
        template <typename T>
        requires requires (T a, T b) { std::hash<T>{}(a); {a == b} -> std::same_as<bool>; }
        SetElement(const T& val) : value(val), type_idx(typeid(T)) {
            equality_func = [](const std::any& a, const std::any& b) {
                return std::any_cast<T>(a) == std::any_cast<T>(b);
            };

            output_func = [](std::ostream& os, const std::any& element) {
                os << std::any_cast<T>(element);
            };

            hash = std::hash<T>{}(val);
        }

        // Output the element to an io stream
        friend std::ostream& operator<<(std::ostream& os, const SetElement& element) {
            element.output_func(os, element.value);
            return os;
        }

        // Check if two elements are equal based on their type and value
        bool operator==(const SetElement& other) const {
            if (type_idx != other.type_idx) return false;
            return equality_func(value, other.value);
        }

        // Custom hash implementation for SetElement, so it can be used in unordered_set
        struct Hash
        {
            std::size_t operator()(const SetElement& element) const
            {
                std::size_t h1 = element.type_idx.hash_code();
                std::size_t h2 = element.hash;            
                return h1 ^ (h2 << 1);
            }
        };
    };

    // Mathematical set class that can hold elements of different types
    class Set {
        public:
            // Construct an empty set
            Set() = default;

            // Construct a set with elements of different types
            template <typename... Types>
            Set(Types... args) : _elements{SetElement(args)...} {}  

            // Output the set to an io stream
            friend std::ostream& operator<<(std::ostream& os, const Set& set) {
                os << "{";
                
                for (const auto& element : set._elements) {
                    if (element != *set._elements.begin()) os << ", ";
                    os << element;
                }
                
                os << "}";
                return os;
            }

            // Return the number of elements in the set
            std::size_t size() const {
                return _elements.size();
            }

            // Check if the set contains a specific element
            template <typename T>
            bool contains(T element) const {
                return _elements.contains(SetElement(element));
            }

            // Check if the set is equal to another set
            bool equal_to(const Set& other) const {
                return _elements == other._elements;
            }

            // Check if the set is a subset of another set
            bool subset_of(const Set& other) const {
                bool missing = false;

                for (const auto& element : _elements) {
                    if (!other.contains(element)) {
                        missing = true;
                        break;
                    }
                }

                return !missing;
            }

            // Check if the set is a superset of another set
            bool superset_of(const Set& other) const {
                return other.subset_of(*this);
            }

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

            // Return the difference of this set and another set
            Set difference(const Set& other) const {
                Set result;

                for (const auto& element : _elements) {
                    if (!other._elements.contains(element)) {
                        result._elements.insert(element);
                    }
                }

                return result;
            }

            // Return the symmetric difference of this set and another set
            Set symmetric_difference(const Set& other) const {
                Set result;

                for (const auto& element : _elements) {
                    if (!other._elements.contains(element)) {
                        result._elements.insert(element);
                    }
                }

                for (const auto& element : other._elements) {
                    if (!_elements.contains(element)) {
                        result._elements.insert(element);
                    }
                }

                return result;
            }

            // Union operator overload
            Set operator|(const Set& other) const { return merge(other); }

            // Intersection operator overload
            Set operator&(const Set& other) const { return intersect(other); }

            // Set difference operator overload
            Set operator/(const Set& other) const { return difference(other); }

            // Symmetric difference operator overload
            Set operator^(const Set& other) const { return symmetric_difference(other); }

            // Set equality operator overload
            bool operator==(const Set& other) const { return equal_to(other); }

            // Subset operator overload
            bool operator<(const Set& other) const { return subset_of(other); }

            // Superset operator overload
            bool operator>(const Set& other) const { return superset_of(other); }

        private:
            std::unordered_set<SetElement, SetElement::Hash> _elements;
    };
}