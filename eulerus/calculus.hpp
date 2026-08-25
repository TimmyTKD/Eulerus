#pragma once

#include <cassert>
#include <cmath>
#include <complex>
#include <numbers>

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
            result += 4.0 * f(x + dx_mid);
            result += 2.0 * f(x + dx);
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

    /**
     * @brief Numerically calculate the `n`th derivative of a single-variable complex-valued function `f` at the value `x`, using Cauchy's integral formula
     * 
     * @tparam T Data type of the function's arguments
     * @tparam Func Function type
     * @param f Complex-valued function to differentiate
     * @param x Value to differentiate the function at
     * @param n Order of the derivative
     * @param r Radius of the integration contour. The optimal radius depends on the function and derivative order, and can return incorrect results if the contour contains singularities of the function
     * @param iterations Number of subintervals for the numerical contour integration
     */
    template <typename T, typename Func>
    requires requires (Func f, T a, std::complex<double> z) {f(z + a) / z;}
    auto differentiate_cauchy(Func f, T x, int n = 1, double r = 1.0, int iterations = 20) {
        assert(n >= 0);
        assert(r > 0.0);
        assert(iterations > 0);

        double two_pi = 2.0 * std::numbers::pi;

        long long n_factorial = 1;
        for (int i = 2; i <= n; i++) {
            n_factorial *= i;
        }

        // Integrand of the Cauchy integral formula for the `n`th derivative of `f` at `x`
        auto F = [&f, &x, &n, &r](auto t) { 
            auto z = r * std::exp(std::complex(0.0, t));
            auto z_n = std::pow(r, n) * std::exp(std::complex(0.0, n * t));
            return f(z + x) / z_n; 
        };

        auto result = integrate(F, 0.0, two_pi, iterations) * (double)n_factorial / two_pi;
        return std::real(result); // TODO: return complex result if the complex derivative is requested
    }
}