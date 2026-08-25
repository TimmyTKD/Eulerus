#pragma once

#include <cassert>

namespace eulerus::calculus {
    /* -------------------------------------------------------------------------- */
    /*                     Single-Variable Numerical Calculus                     */
    /* -------------------------------------------------------------------------- */

    /**
     * @brief Numerically integrate a single-variable function `f` over the interval `[a, b]`
     * 
     * @tparam T Data type of the function's arguments
     * @tparam Func Function type
     * @param f Function to integrate
     * @param a Lower bound of the integration interval
     * @param b Upper bound of the integration interval
     * @param iterations Number of subintervals used for the numerical integration
     */
    template <typename T, typename Func>
    requires requires (Func f, T a, T b, double c) {(f(a) - f(b)) / c;}
    auto integrate(Func f, T a, T b, int iterations = 2000) {
        assert(iterations > 0);
        
        // Ensure an even number of subintervals for Simpson's 1/3 rule
        if (iterations % 2 != 0) iterations++; 

        auto dx = (b - a) / (double)iterations;
        auto dx_mid = dx / 2.0;

        // Approximate the integral using the composite Simpson's 1/3 rule 
        auto result = f(a) - f(b);
        for (int i = 0; i < iterations; i++) {
            auto x = a + i * dx;
            result += 4 * f(x + dx_mid);
            result += 2 * f(x + dx);
        }

        return result * dx_mid / 3.0;
    }

    /**
     * @brief Numerically differentiate a single-variable function `f` at the value `x`
     * 
     * @tparam T Data type of the function's arguments
     * @tparam Func Function type
     * @param f Function to differentiate
     * @param x Value to differentiate the function at
     * @param increment Increment between values for the numerical differentiation. Should be kept small to improve accuracy, but not too small to avoid floating-point errors
     */
    template <typename T, typename Func>
    requires requires (Func f, T a, T b, double c) {(f(a) - f(b)) / c;}
    auto differentiate(Func f, T x, double increment = 1e-6) {
        assert(increment > 0);

        // Approximate the derivative using the central-difference method
        return (f(x + increment) - f(x - increment)) / (2.0 * increment);
    }
}