#pragma once

#include <cassert>
#include <cmath>
#include <concepts>
#include <cstddef>
#include <array>
#include <iostream>
#include <type_traits>
#include <utility>

namespace eulerus::linear_algebra {
    /* -------------------------------------------------------------------------- */
    /*                                  Matrices                                  */
    /* -------------------------------------------------------------------------- */

    /**
     * @brief Matrix class supporting different dimensions and data types
     * 
     * @tparam Rows Number of rows in the matrix
     * @tparam Columns Number of columns in the matrix
     * @tparam T Data type of the matrix's values
     */
    template <std::size_t Rows, std::size_t Columns, typename T = double>
    requires (Rows > 0 && Columns > 0)
    class Matrix {
        public:
            using MatrixDataType = T;
            static constexpr std::array<std::size_t, 2> shape = {Rows, Columns};

            // Construct a matrix with a default initialization of its values
            Matrix() = default;

            // Construct a matrix from any 2D collection
            template <typename Collection>
            requires requires(Collection a) {{a[0][0]} -> std::convertible_to<T>; requires (std::size(a) == Rows);}
            Matrix(const Collection& args) {
                for (std::size_t i = 0; i < Rows; i++) {
                    assert(std::size(args[i]) == Columns);

                    for (std::size_t j = 0; j < Columns; j++) {
                        values[i][j] = args[i][j];
                    }
                }
            }

            // Construct a matrix from multiple arrays of values
            template <std::convertible_to<T>... Values, std::size_t N>
            requires (sizeof...(Values) == Rows) && (N == Columns)
            Matrix(const Values (&...args)[N]) : Matrix(std::array<std::array<T, Columns>, Rows>{std::to_array(args)...}) { }

            // Construct an N x 1 matrix (vector) from any collection
            template <typename Collection>
            requires requires(Collection a) {{a[0]} -> std::convertible_to<T>; requires (std::size(a) == Rows);}
            Matrix(const Collection& args) requires(Columns == 1) {
                for (std::size_t i = 0; i < Rows; i++) {
                    values[i][0] = args[i];
                }
            }

            // Output the matrix to an io stream
            friend std::ostream& operator<<(std::ostream& os, const Matrix& matrix) {
                os << std::endl;
                
                for (std::size_t i = 0; i < Rows; i++) {
                    os << '[';
                    for (std::size_t j = 0; j < Columns; j++) {
                        os << matrix.values[i][j];
                        if (j + 1 < Columns) os << ", ";
                    }
                    os << ']' << std::endl;
                }

                return os;
            }

            // Cast the matrix's values to a compatible type
            template <typename T2>
            requires (std::is_convertible_v<T, T2>)
            Matrix<Rows, Columns, T2> cast() const {
                Matrix<Rows, Columns, T2> matrix;

                for (std::size_t i = 0; i < Rows; i++) {
                    for (std::size_t j = 0; j < Columns; j++) {
                        matrix[i][j] = static_cast<T2>((*this)[i][j]);
                    }
                }

                return matrix;
            }

            // Return the matrix in row echelon form
            Matrix<Rows, Columns, T> row_echelon_form() const {
                return _row_echelon_form().ref_matrix;
            }

            // Return the determinant of a square matrix
            const T determinant() const requires (Rows == Columns) {
                if constexpr (Rows == 1) {
                    return values[0][0];
                }
                else if constexpr (Rows == 2) {
                    return values[0][0] * values[1][1] - values[0][1] * values[1][0];
                }
                else { 
                    // size > 2: calculate the determinant from the matrix's row echelon form

                    REFData ref_data = _row_echelon_form();
                    const Matrix<Rows, Columns, T>& ref_matrix = ref_data.ref_matrix;

                    T result = ref_matrix[0][0];
                    
                    for (std::size_t i = 1; i < Rows; i++) {
                        if (ref_matrix[i][i] == T()) return T();
                        result *= ref_matrix[i][i];
                    }
                    
                    return result * ref_data.determinant_sign;
                }
            }

            // Return the matrix's row count
            static constexpr std::size_t size() { return Rows; }
            
            // Return a mutable reference to the row at `index`
            auto& operator[](std::size_t index) { return values[index]; }

            // Return an immutable reference to the row at `index`
            const auto& operator[](std::size_t index) const { return values[index]; }

            // Return a mutable reference to the value at row `i` and column `j`
            T& get_value(std::size_t i, std::size_t j) { return values[i][j]; }

            // Return an immutable reference to the value at row `i` and column `j`
            const T& get_value(std::size_t i, std::size_t j) const { return values[i][j]; }

            // Post-multiply the matrix in-place by a compatible square matrix of the same type
            Matrix& operator*=(const Matrix<Columns, Columns, T>& matrix) {
                *this = *this * matrix;
                return *this;
            }

        private:
            std::array<std::array<T, Columns>, Rows> values{};

            // Helper struct for row echelon form calculations
            struct REFData {
                Matrix ref_matrix;
                int determinant_sign = 1;
            };

            // Internal implementation of the row_echelon_form function, with additional information for determinant calculations
            REFData _row_echelon_form() const {
                Matrix<Rows, Columns, T> matrix = *this;
                int determinant_sign = 1;
 
                for (std::size_t row = 0, column = 0; row < Rows && column < Columns;) {
                    T pivot = matrix[row][column];

                    // If the pivot is zero, swap the row with the first row below it that has a non-zero pivot in the same column
                    if (matrix[row][column] == T()) {
                        bool swapped = false;
                        for (std::size_t i = row + 1; i < Rows; i++) {
                            if (matrix[i][column] != T()) {
                                std::swap(matrix[row], matrix[i]);
                                pivot = matrix[row][column];
                                determinant_sign *= -1;
                                swapped = true;
                                break;
                            }
                        }

                        // If no non-zero pivot was found, skip this pivot and move to the next column on the same row
                        if (!swapped) {
                            column++;
                            continue; 
                        }
                    }

                    for (std::size_t i = row + 1; i < Rows; i++) {
                        if (matrix[i][column] == T()) continue; // Skip the row if the factor would be zero
                        T factor = matrix[i][column] / pivot;

                        for (std::size_t j = column; j < Columns; j++) {
                            matrix[i][j] -= factor * matrix[row][j];
                        }
                    }

                    row++;
                    column++;
                }
                
                return REFData{matrix, determinant_sign};
            }
    };

    // TODO: inverses and matrix "division"

    // Template deduction guide to deduce the Matrix type from multiple arrays of values 
    template <typename... Values, typename T = std::common_type_t<Values...>, std::size_t Columns>
    Matrix(const Values (&...args)[Columns]) -> Matrix<sizeof...(Values), Columns, T>;

    /**
     * @brief Create a square identity matrix of a given data type and shape
     * 
     * @tparam Dimension Number of rows and columns in the matrix
     * @tparam T Data type of the matrix's values
     * @param identity "Identity" value to fill the matrix's main diagonal. Defaults to the equivalent of '1' for the type `T`
     * @param non_identity Value to fill the rest of the matrix. Defaults to the default value of type `T`
     */
    template <std::size_t Dimension, typename T = double>
    constexpr Matrix<Dimension, Dimension, T> Identity(T identity = T(1), T non_identity = T()) {
        Matrix<Dimension, Dimension, T> matrix;

        if (non_identity != T()) {
            for (std::size_t i = 0; i < Dimension; i++) {
                for (std::size_t j = 0; j < Dimension; j++) {
                    matrix[i][j] = non_identity;
                }
            }
        }

        for (std::size_t i = 0; i < Dimension; i++) {
            matrix[i][i] = identity;
        }

        return matrix;
    }

    // Return the transpose of `matrix` 
    template <std::size_t Rows, std::size_t Columns, typename T>
    constexpr Matrix<Columns, Rows, T> transpose(const Matrix<Rows, Columns, T>& matrix) {
        Matrix<Columns, Rows, T> transposed;

        for (std::size_t i = 0; i < matrix.shape[0]; i++) { 
            for (std::size_t j = 0; j < matrix.shape[1]; j++) { 
                transposed.get_value(j, i) = matrix.get_value(i, j);
            }
        }

        return transposed;
    }

    // Matrix-multiply the M x N matrix `left` by the N x P matrix `right` to return a new M x P matrix
    template <std::size_t Rows, std::size_t Columns, std::size_t Columns2, typename T, typename T2>
    requires requires (T a, T2 b) { a * b; }
    auto operator*(const Matrix<Rows, Columns, T>& left, const Matrix<Columns, Columns2, T2>& right) {
        using Result = decltype(std::declval<T>() * std::declval<T2>());
        Matrix<Rows, Columns2, Result> product;

        for (std::size_t i = 0; i < Rows; i++) {
            for (std::size_t j = 0; j < Columns2; j++) {
                product.get_value(i, j) = left.get_value(i, 0) * right.get_value(0, j);
                for (std::size_t k = 1; k < Columns; k++) {
                    product.get_value(i, j) += left.get_value(i, k) * right.get_value(k, j);
                }
            }
        }

        return product;
    }

    // Multiply the matrix in-place by a scalar
    template <typename M, typename Other>
    requires requires(typename M::MatrixDataType a, Other b) { a *= b; }
    M& operator*=(M& matrix, const Other& other) {
        for (std::size_t i = 0; i < M::shape[0]; i++) {
            for (std::size_t j = 0; j < M::shape[1]; j++) {
                matrix.get_value(i, j) *= other;
            }
        }
        return matrix;
    }

    // Divide the matrix in-place by a scalar
    template <typename M, typename Other>
    requires requires(typename M::MatrixDataType a, Other b) { a /= b; }
    M& operator/=(M& matrix, const Other& other) {
        for (std::size_t i = 0; i < M::shape[0]; i++) {
            for (std::size_t j = 0; j < M::shape[1]; j++) {
                matrix.get_value(i, j) /= other;
            }
        }

        return matrix;
    }

    // Add in-place another matrix of the same shape to the matrix
    template <typename M, typename M2>
    requires requires(typename M::MatrixDataType a, typename M2::MatrixDataType b) { a += b; } && (M::shape == M2::shape)
    M& operator+=(M& matrix, const M2& other) {
        for (std::size_t i = 0; i < M::shape[0]; i++) {
            for (std::size_t j = 0; j < M::shape[1]; j++) {
                matrix.get_value(i, j) += other.get_value(i, j);
            }
        }

        return matrix;
    }

    // Subtract in-place another matrix of the same shape from the matrix
    template <typename M, typename M2>
    requires requires(typename M::MatrixDataType a, typename M2::MatrixDataType b) { a -= b; } && (M::shape == M2::shape)
    M& operator-=(M& matrix, const M2& other) {
        for (std::size_t i = 0; i < M::shape[0]; i++) {
            for (std::size_t j = 0; j < M::shape[1]; j++) {
                matrix.get_value(i, j) -= other.get_value(i, j);
            }
        }

        return matrix;
    }

    // Post-multiply `matrix` by a scalar `other`
    template <typename M, typename Other>
    requires requires(typename M::MatrixDataType a, Other b) { a *= b; }
    M operator*(const M& matrix, const Other& other) { 
        M result = matrix;
        return result *= other; 
    }
    
    // Pre-multiply `matrix` by a scalar `other`
    template <typename M, typename Other>
    requires requires(typename M::MatrixDataType a, Other b) { a *= b; }
    M operator*(const Other& other, const M& matrix) { 
        M result = matrix;
        return result *= other; 
    }

    // Divide `matrix` by a scalar `other`
    template <typename M, typename Other>
    requires requires(typename M::MatrixDataType a, Other b) { a /= b; }
    M operator/(const M& matrix, const Other& other) {
        M result = matrix;
        return result /= other;
    }

    // Add two matrices of the same shape
    template <typename M, typename M2>
    requires requires(typename M::MatrixDataType a, typename M2::MatrixDataType b) { a += b; } && (M::shape == M2::shape)
    auto operator+(const M& left, const M2& right) {
        M result = left;
        return result += right;
    }

    // Subtract two matrices of the same shape
    template <typename M, typename M2>
    requires requires(typename M::MatrixDataType a, typename M2::MatrixDataType b) { a -= b; } && (M::shape == M2::shape)
    auto operator-(const M& left, const M2& right) {
        M result = left;
        return result -= right;
    }

    /* -------------------------------------------------------------------------- */
    /*                                   Vectors                                  */
    /* -------------------------------------------------------------------------- */

    /**
     * @brief Vector class supporting different dimensions and data types
     * 
     * @tparam Dimension Number of values held by the vector
     * @tparam T Data type of the vector's values
     */
    template <std::size_t Dimension, typename T = double>
    requires (Dimension > 0)
    class Vector : public Matrix<Dimension, 1, T> {
        public:
            // Construct a vector with a default initialization of its values
            Vector() = default;

            // Construct a vector from any collection
            template <typename Collection>
            requires requires(Collection a) {{a[0]} -> std::convertible_to<T>; requires (std::size(a) == Dimension);}
            Vector(const Collection& args) : Matrix<Dimension, 1, T>(args) { }

            // Construct a vector from multiple values
            template <std::convertible_to<T>... Values>
            requires (sizeof...(Values) == Dimension)
            Vector(const Values&... args) : Vector(std::array<T, Dimension>{args...}) { }

            // Construct a vector from an N x 1 matrix
            Vector(const Matrix<Dimension, 1, T>& matrix) : Matrix<Dimension, 1, T>(matrix) {}

            // Return a mutable reference to the value at `index`
            T& operator[](std::size_t index) { return Matrix<Dimension, 1, T>::operator[](index)[0]; }

            // Return an immutable reference to the value at `index`
            const T& operator[](std::size_t index) const { return Matrix<Dimension, 1, T>::operator[](index)[0]; }

            // Pre-multiply the vector in-place by a compatible square matrix of the same type
            Vector& operator*=(const Matrix<Dimension, Dimension, T>& matrix) {
                *this = matrix * (*this);
                return *this;
            }

            // Return the square of the vector's magnitude
            auto sqr_magnitude() const { return *this * (*this); }

            // Return the vector's magnitude
            auto magnitude() const { return std::sqrt(sqr_magnitude()); }
    };

    // Template deduction guide to deduce the Vector type from multiple values
    template <typename... Values, typename T = std::common_type_t<Values...>>
    Vector(const Values&... args) -> Vector<sizeof...(Values), T>;

    // Return the scalar/dot product between vectors `left` and `right`
    template <std::size_t Dimension, typename T, typename T2>
    requires requires(T a, T2 b) {a * b;} 
    auto operator*(const Vector<Dimension, T>& left, const Vector<Dimension, T2>& right) {
        auto sum = left[0] * right[0];
        
        for (std::size_t i = 1; i < Dimension; i++) {
            sum += left[i] * right[i];
        }

        return sum;
    }

    // Pre-multiply `vector` by `matrix` to return a new vector
    template <std::size_t Rows, std::size_t Columns, typename T, typename T2>
    requires requires (T a, T2 b) { a * b; }
    auto operator*(const Matrix<Rows, Columns, T>& matrix, const Vector<Columns, T2>& vector) {
        using Result = decltype(std::declval<T>() * std::declval<T2>());
        const auto& converted = static_cast<const Matrix<Rows, 1, T2>&>(vector);
        return Vector<Rows, Result>(matrix * converted);
    }
}