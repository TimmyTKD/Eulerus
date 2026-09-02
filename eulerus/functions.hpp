#pragma once

#include <cmath>
#include <complex>
#include <memory>
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
            using FunctionType = Func;

            // Construct a null function wrapper by default
            Function() = default;

            // Construct a function object using a base function
            Function(Func f) : _function(std::make_shared<const Func>(std::move(f))) {}

            // Return an immutable shared pointer to the internal function implementation
            const std::shared_ptr<const Func> operator&() const { 
                return _function; 
            }

            // Call the internal function implementation
            template<typename... Args>
            requires ((requires { typename Args::FunctionType; } == false) && ...) // Ensure `Function` objects are not accepted as arguments
            auto operator()(Args... args) const {
                return (*_function)(args...);
            }

            // Compose the function with other functions
            template<typename... Funcs>
            requires (requires { typename Funcs::FunctionType; } || ...) // Ensure at least one argument is a `Function` object
            auto operator()(const Funcs&... inner_functions) const {
                auto outer_ptr = this->_function;
                
                auto capture_value = [](auto inner_function) {
                    if constexpr (requires { typename decltype(inner_function)::FunctionType; }) {
                        return &inner_function;
                    } else {
                        return inner_function;
                    }
                };

                auto f = [outer_ptr, ...inner_values = capture_value(inner_functions)](auto... args) { 
                    // Helper function to separate handling of constant arguments from function arguments 
                    auto argument_value = [&args...](auto inner_value) {
                        if constexpr (std::is_invocable_v<typename decltype(inner_value)::element_type, decltype(args)...>) {
                            return (*inner_value)(args...);
                        } else {
                            return inner_value;
                        }
                    };

                    return (*outer_ptr)(argument_value(inner_values)...); 
                };

                return Function<decltype(f)>(std::move(f));
            }

        private:
            std::shared_ptr<const Func> _function;
    };

    // Add two functions
    template<typename F, typename F2>
    auto operator+(const Function<F>& left, const Function<F2>& right) {
        return Function([left_ptr = &left, right_ptr = &right](auto... x) { return (*left_ptr)(x...) + (*right_ptr)(x...); });
    }

    // Add a constant to a function
    template<typename F, typename T>
    auto operator+(const Function<F>& function, const T other) {
        return Function([function_ptr = &function, other](auto... x) { return (*function_ptr)(x...) + other; });
    }

    // Add a function to a constant
    template<typename F, typename T>
    auto operator+(const T other, const Function<F>& function) {
        return Function([function_ptr = &function, other](auto... x) { return other + (*function_ptr)(x...); });
    }

    // Subtract two functions
    template<typename F, typename F2>
    auto operator-(const Function<F>& left, const Function<F2>& right) {
        return Function([left_ptr = &left, right_ptr = &right](auto... x) { return (*left_ptr)(x...) - (*right_ptr)(x...); });
    }

    // Subtract a constant from a function
    template<typename F, typename T>
    auto operator-(const Function<F>& function, const T other) {
        return Function([function_ptr = &function, other](auto... x) { return (*function_ptr)(x...) - other; });
    }

    // Subtract a function from a constant
    template<typename F, typename T>
    auto operator-(const T other, const Function<F>& function) {
        return Function([function_ptr = &function, other](auto... x) { return other - (*function_ptr)(x...); });
    }

    // Multiply two functions
    template<typename F, typename F2>
    auto operator*(const Function<F>& left, const Function<F2>& right) {
        return Function([left_ptr = &left, right_ptr = &right](auto... x) { return (*left_ptr)(x...) * (*right_ptr)(x...); });
    }

    // Multiply a function by a constant
    template<typename F, typename T>
    auto operator*(const Function<F>& function, const T other) {
        return Function([function_ptr = &function, other](auto... x) { return (*function_ptr)(x...) * other; });
    }

    // Multiply a constant by a function
    template<typename F, typename T>
    auto operator*(const T other, const Function<F>& function) {
        return Function([function_ptr = &function, other](auto... x) { return other * (*function_ptr)(x...); });
    }

    // Divide two functions
    template<typename F, typename F2>
    auto operator/(const Function<F>& left, const Function<F2>& right) {
        return Function([left_ptr = &left, right_ptr = &right](auto... x) { return (*left_ptr)(x...) / (*right_ptr)(x...); });
    }

    // Divide a function by a constant
    template<typename F, typename T>
    auto operator/(const Function<F>& function, const T other) {
        return Function([function_ptr = &function, other](auto... x) { return (*function_ptr)(x...) / other; });
    }

    // Divide a constant by a function
    template<typename F, typename T>
    auto operator/(const T other, const Function<F>& function) {
        return Function([function_ptr = &function, other](auto... x) { return other / (*function_ptr)(x...); });
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

    // "Variable" function, which simply returns a function wrapper for the input. Useful for input transformations like f(2x) or polynomials like x^2+x+1
    inline Function var([](auto x) { return x; });

    inline Function sqr([](auto x) { return std::pow(x, floating_cast<decltype(x)>(2.0)); });
    inline Function pow([](auto x, auto n) { return std::pow(x, n); });
    inline Function sqrt([](auto x) { return std::sqrt(x); });
    inline Function cbrt([](auto x) { return std::cbrt(x); });
    inline Function nthroot([](auto x, auto n) { return std::pow(x, floating_cast<decltype(x)>(1.0) / n); });

    inline Function exp([](auto x) { return std::exp(x); });
    inline Function ln([](auto x) { return std::log(x); }); 
    inline Function log10([](auto x) { return std::log10(x); });
    inline Function log2([](auto x) { return std::log2(x); });
    inline Function log([]<typename T, typename T2 = T>(T x, T2 n = std::numbers::e_v<to_floating_t<T>>) { return std::log(x) / std::log(n); });

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
    inline auto& arcsin = asin;
    inline auto& arccos = acos;
    inline auto& arctan = atan;
    inline auto& arccsc = acsc;
    inline auto& arcsec = asec;
    inline auto& arccot = acot;

    inline Function sinh([](auto x) { return std::sinh(x); });
    inline Function cosh([](auto x) { return std::cosh(x); });
    inline Function tanh([](auto x) { return std::tanh(x); });
    inline Function csch([](auto x) { return floating_cast<decltype(x)>(1.0) / std::sinh(x); });
    inline Function sech([](auto x) { return floating_cast<decltype(x)>(1.0) / std::cosh(x); });
    inline Function coth([](auto x) { return floating_cast<decltype(x)>(1.0) / std::tanh(x); });

    inline Function asinh([](auto x) { return std::asinh(x); });
    inline Function acosh([](auto x) { return std::acosh(x); });
    inline Function atanh([](auto x) { return std::atanh(x); });
    inline Function acsch([](auto x) { return std::asinh(floating_cast<decltype(x)>(1.0) / x); });
    inline Function asech([](auto x) { return std::acosh(floating_cast<decltype(x)>(1.0) / x); });
    inline Function acoth([](auto x) { return std::atanh(floating_cast<decltype(x)>(1.0) / x); });
    inline auto& arcsinh = asinh;
    inline auto& arccosh = acosh;
    inline auto& arctanh = atanh;
    inline auto& arccsch = acsch;
    inline auto& arcsech = asech;
    inline auto& arccoth = acoth;
}