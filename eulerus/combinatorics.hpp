#pragma once

#include <any>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <functional>
#include <iostream>
#include <iterator>
#include <type_traits>
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
        void (*output_func)(std::ostream& os, const std::any& element_value);
        std::size_t hash;

        // Construct a SetElement from a value of any type
        template <typename T>
        requires requires (T a, T b) { std::hash<T>{}(a); {a == b} -> std::same_as<bool>; }
        SetElement(const T& val) : value(val), type_idx(typeid(T)) {
            equality_func = [](const std::any& a, const std::any& b) {
                return std::any_cast<T>(a) == std::any_cast<T>(b);
            };

            output_func = [](std::ostream& os, const std::any& element_value) {
                os << std::any_cast<T>(element_value);
            };

            hash = std::hash<T>{}(val);
        }

        // Construct a SetElement from a value of any type, using a provided hash
        template <typename T>
        requires requires (T a, T b) { {a == b} -> std::same_as<bool>; }
        SetElement(const T& val, std::size_t custom_hash) : value(val), type_idx(typeid(T)), hash(custom_hash) {
            equality_func = [](const std::any& a, const std::any& b) {
                return std::any_cast<T>(a) == std::any_cast<T>(b);
            };

            output_func = [](std::ostream& os, const std::any& element_value) {
                os << std::any_cast<T>(element_value);
            };
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

    // Helper struct to store an ordered tuple of SetElements
    template <std::size_t N>
    struct SetElementTuple {
        SetElement items[N];
        std::size_t hash;

        // Construct a SetElementTuple from several objects
        template <typename... Args>
        requires(sizeof...(Args) == N)
        SetElementTuple(Args... args) : items(args...) {
            hash = typeid(SetElementTuple).hash_code();
            auto hasher = eulerus::combinatorics::SetElement::Hash();

            // Hash each item in the tuple, using the same method as the boost library's hash_combine function
            for (std::size_t i = 0; i < N; i++) {
                hash ^= hasher(items[i]) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
            }
        }

        // Output the tuple to an io stream
        friend std::ostream& operator<<(std::ostream& os, const SetElementTuple& tuple) {
            os << "(";
            
            for (std::size_t i = 0; i < N; i++) {
                if (i != 0) os << ", ";
                os << tuple.items[i];
            }

            os << ")";
            return os;
        }

        // Check if two tuples are equal based on the equality of all items
        bool operator==(SetElementTuple other) const {
            bool equal = true;

            for (std::size_t i = 0; i < N; i++) {
                if (items[i] != other.items[i]) {
                    equal = false;
                    break;
                }
            }

            return equal;
        }
    };

    // Mathematical set class that can hold elements of different types
    class Set {
        public:
            // Construct an empty set
            Set() = default;

            // Construct a set with elements of different types
            template <typename... Types>
            Set(Types... args) : _elements{SetElement(args)...} {} 
            
            // Construct a set from an iterable collection
            template <typename Iterable>
            requires requires (Iterable collection) { std::size(collection); std::begin(collection); std::end(collection); } 
            static Set from_iterable(Iterable collection) {
                Set set;
                set._elements.insert(std::begin(collection), std::end(collection));
                return set;
            }

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

            // Return an iterator pointing to the first element in the set
            auto begin() const { return _elements.begin(); }

            // Return an iterator pointing to the last element in the set
            auto end() const { return _elements.end(); }

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

            // Return the complement of this set with respect to some universal set
            Set complement(const Set& universal_set) const {
                Set result = universal_set;

                for (const auto& element : _elements) {
                    result._elements.erase(element);
                }

                return result;
            }

            // Return the cartesian product of this set and another set
            Set cartesian_product(const Set& other) const {
                std::unordered_set<SetElement, SetElement::Hash> elements;

                // Return the empty set if any of the sets are empty
                if (this->size() == 0 || other.size() == 0) return Set{};

                for (const auto& a : _elements) {
                    for (const auto& b : other._elements) {
                        SetElementTuple<2> pair = SetElementTuple<2>(a, b);
                        elements.insert(SetElement(pair, pair.hash));
                    }
                }

                return Set::from_iterable(elements);
            }

            // Union operator overload
            Set operator|(const Set& other) const { return merge(other); }

            // Intersection operator overload
            Set operator&(const Set& other) const { return intersect(other); }

            // Set difference operator overload
            Set operator-(const Set& other) const { return difference(other); }

            // Symmetric difference operator overload
            Set operator^(const Set& other) const { return symmetric_difference(other); }

            // Cartesian product operator overload
            Set operator*(const Set& other) const { return this->cartesian_product(other); }

            // Set equality operator overload
            bool operator==(const Set& other) const { return equal_to(other); }

            // Subset operator overload
            bool operator<=(const Set& other) const { return subset_of(other); }

            // Superset operator overload
            bool operator>=(const Set& other) const { return superset_of(other); }

            // Strict subset operator overload
            bool operator<(const Set& other) const { return subset_of(other) && !equal_to(other); }

            // Strict superset operator overload
            bool operator>(const Set& other) const { return superset_of(other) && !equal_to(other); }

        private:
            std::unordered_set<SetElement, SetElement::Hash> _elements;
    };

    // Return the union of several sets
    template <std::same_as<Set>... Sets>
    requires(sizeof...(Sets) >= 2)
    inline Set merge_sets(const Sets&... sets) {
        std::unordered_set<SetElement, SetElement::Hash> elements;
        (elements.insert(sets.begin(), sets.end()), ...);
        return Set::from_iterable(elements);
    }

    // Return the intersection of several sets
    template <std::same_as<Set>... Sets>
    requires(sizeof...(Sets) >= 2)
    inline Set intersect_sets(const Sets&... sets) {
        std::unordered_set<SetElement, SetElement::Hash> elements;
        const Set* set_array[sizeof...(Sets)] = {(&sets)...};

        elements.insert(set_array[0]->begin(), set_array[0]->end());

        for (const Set* set : set_array) {
            // Return the empty set if any of the sets are empty, or if `elements` becomes empty
            if (set->size() == 0 || elements.size() == 0) return Set{};

            // Remove elements from `elements` that are not in `set`
            auto iterator = elements.begin();
            while (iterator != elements.end()) {
                if (!set->contains(*iterator)) {
                    iterator = elements.erase(iterator); 
                } else {
                    iterator++;
                }
            }
        }

        return Set::from_iterable(elements);
    }

    // Return the cartesian product of several sets
    template <std::same_as<Set>... Sets>
    requires(sizeof...(Sets) >= 2)
    inline Set cartesian_product(const Sets&... sets) {
        std::unordered_set<SetElement, SetElement::Hash> elements;
        const Set* set_array[sizeof...(Sets)] = {(&sets)...};

        // Return the empty set if any of the sets are empty
        for (const Set* set : set_array) {
            if (set->size() == 0) return Set{};
        }

        // Recursively loop through the sets to construct ordered tuples using their elements
        auto recursive_loop = [&set_array, &elements](auto&& recursive_loop, auto n, const auto&... items) {
            for (const auto& element : *set_array[n.value]) {
                if constexpr (n.value == sizeof...(Sets) - 1) {
                    SetElementTuple<sizeof...(Sets)> tuple = SetElementTuple<sizeof...(Sets)>(items..., element);
                    elements.insert(SetElement(tuple, tuple.hash));
                }
                else {
                    recursive_loop(recursive_loop, std::integral_constant<std::size_t, n + 1>{}, items..., element);
                }
            }
        };

        recursive_loop(recursive_loop, std::integral_constant<std::size_t, 0>{});

        return Set::from_iterable(elements);
    }
}

// Custom specialization of std::hash for Set class
template<>
struct std::hash<eulerus::combinatorics::Set>
{
    std::size_t operator()(const eulerus::combinatorics::Set& set) const
    {
        std::size_t hash = typeid(eulerus::combinatorics::Set).hash_code();

        // Hash each element in the set, using the same method as the boost library's hash_combine function
        for (const auto& element : set) {
            auto hasher = eulerus::combinatorics::SetElement::Hash();
            hash ^= hasher(element) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
        }

        return hash;
    }
};