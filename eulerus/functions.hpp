#pragma once

#include <cmath>
#include <complex>
#include <numbers>
#include <type_traits>
#include <utility>

namespace eulerus::functions {
    /* -------------------------------------------------------------------------- */
    /*                           Function Wrapper Class                           */
    /* -------------------------------------------------------------------------- */

    /**
    * @brief Generic function class that acts as a wrapper of some invocable function
    * 
    * @tparam Func Function type
    */
    template <typename Func> // TODO: require that `Func` is an invocable type of generic arguments
    class Function {
        public:
            // Construct a function object using a base function
            Function(Func f) : _function(std::move(f)) {}

            // Call the internal function implementation
            template<typename T>
            auto operator()(T arg) {
                return _function(arg);
            }

            // Compose the function with another function
            template<typename F>
            auto operator()(const Function<F>& inner) {
                auto outer = this->_function;
                auto f = [outer, inner](auto x) { return outer(inner(x)); };
                return Function<decltype(f)>(f);
            }

        private:
            Func _function;
    };

    // Add two functions
    template<typename F, typename F2>
    auto operator+(const Function<F>& left, const Function<F2>& right) {
        return Function([left, right](auto x) { return left(x) + right(x); });
    }

    // Add a constant to a function
    template<typename F, typename T>
    auto operator+(const Function<F>& function, const T other) {
        return Function([function, other](auto x) { return function(x) + other; });
    }

    // Add a function to a constant
    template<typename F, typename T>
    auto operator+(const T other, const Function<F>& function) {
        return Function([function, other](auto x) { return other + function(x); });
    }

    // Subtract two functions
    template<typename F, typename F2>
    auto operator-(const Function<F>& left, const Function<F2>& right) {
        return Function([left, right](auto x) { return left(x) - right(x); });
    }

    // Subtract a constant from a function
    template<typename F, typename T>
    auto operator-(const Function<F>& function, const T other) {
        return Function([function, other](auto x) { return function(x) - other; });
    }

    // Subtract a function from a constant
    template<typename F, typename T>
    auto operator-(const T other, const Function<F>& function) {
        return Function([function, other](auto x) { return other - function(x); });
    }

    // Multiply two functions
    template<typename F, typename F2>
    auto operator*(const Function<F>& left, const Function<F2>& right) {
        return Function([left, right](auto x) { return left(x) * right(x); });
    }

    // Multiply a function by a constant
    template<typename F, typename T>
    auto operator*(const Function<F>& function, const T other) {
        return Function([function, other](auto x) { return function(x) * other; });
    }

    // Multiply a constant by a function
    template<typename F, typename T>
    auto operator*(const T other, const Function<F>& function) {
        return Function([function, other](auto x) { return other * function(x); });
    }

    // Divide two functions
    template<typename F, typename F2>
    auto operator/(const Function<F>& left, const Function<F2>& right) {
        return Function([left, right](auto x) { return left(x) / right(x); });
    }

    // Divide a function by a constant
    template<typename F, typename T>
    auto operator/(const Function<F>& function, const T other) {
        return Function([function, other](auto x) { return function(x) / other; });
    }

    // Divide a constant by a function
    template<typename F, typename T>
    auto operator/(const T other, const Function<F>& function) {
        return Function([function, other](auto x) { return other / function(x); });
    }

    /* -------------------------------------------------------------------------- */
    /*                        Base Function Implementations                       */
    /* -------------------------------------------------------------------------- */

    /* --------------------------------- Helpers -------------------------------- */

    // Helper type trait that defaults to `double` for non-floating-point types, otherwise returns the original type
    template <typename T>
    struct to_floating { using value = std::conditional_t<std::is_floating_point_v<T>, T, double>; };

    // Helper type trait that returns a suitable floating-point type for complex types
    template <typename T>
    struct to_floating<std::complex<T>> { using value = typename to_floating<T>::value; };

    // Helper type alias for `to_floating`
    template <typename T>
    using to_floating_t = typename to_floating<T>::value;

    // Helper function that casts a value to a suitable floating-point type
    template <typename T>
    auto floating_cast(auto x) { return static_cast<to_floating_t<T>>(x); }

    /* -------------------------------------------------------------------------- */

    // "Variable" function, which simply returns a function wrapper for the input
    inline Function var([](auto x) { return x; });

    inline Function sin([](auto x) { return std::sin(x); });
    inline Function cos([](auto x) { return std::cos(x); });
    inline Function tan([](auto x) { return std::tan(x); });
    
    inline Function csc([](auto x) { return floating_cast<decltype(x)>(1.0) / std::sin(x); });
    inline Function sec([](auto x) { return floating_cast<decltype(x)>(1.0) / std::cos(x); });
    inline Function cot([](auto x) { return floating_cast<decltype(x)>(1.0) / std::tan(x); });

    inline Function asin([](auto x) { return std::asin(x); });
    inline Function acos([](auto x) { return std::acos(x); });
    inline Function atan([](auto x) { return std::atan(x); });
    
    inline Function acsc([](auto x) { return std::asin(floating_cast<decltype(x)>(1.0) / x); });
    inline Function asec([](auto x) { return std::acos(floating_cast<decltype(x)>(1.0) / x); });
    inline Function acot([](auto x) { return std::numbers::pi_v<to_floating_t<decltype(x)>> / floating_cast<decltype(x)>(2.0) - std::atan(x); });
}