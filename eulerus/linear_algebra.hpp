#pragma once

#include <cassert>
#include <concepts>
#include <cstddef>
#include <initializer_list>
#include <array>
#include <iostream>
#include <type_traits>
#include <algorithm>

namespace eulerus::linear_algebra {
    /* -------------------------------------------------------------------------- */
    /*                                  Matrices                                  */
    /* -------------------------------------------------------------------------- */

    /**
    * @brief Matrix class supporting different dimensions and data types
    * 
    * @tparam rows Number of rows in the matrix
    * @tparam columns Number of columns in the matrix
    * @tparam T Data type of the matrix's values
    */
    template <std::size_t rows, std::size_t columns, typename T = double>
    requires (rows > 0 && columns > 0)
    class Matrix {
        public:
            using MatrixDataType = T;
            static constexpr std::array<std::size_t, 2> shape = {rows, columns};

            // Construct a matrix with a default initialization of its values
            Matrix() = default;

            // Construct a matrix from a 2D array of values
            Matrix(std::initializer_list<std::initializer_list<T>> args) {
                assert(args.size() == rows);

                auto row = args.begin();
                for (std::size_t i = 0; i < rows; i++) {
                    assert(row->size() == columns);
                    std::copy(row->begin(), row->end(), values[i].begin());
                    row++;
                }
            }

            // Construct a matrix from any 2D collection
            template <typename Collection>
            requires requires(Collection a) {{a[0][0]} -> std::convertible_to<T>; requires (std::size(a) == rows);}
            Matrix(const Collection& args) {
                for (std::size_t i = 0; i < rows; i++) {
                    assert(std::size(args[i]) == columns);

                    for (std::size_t j = 0; j < columns; j++) {
                        values[i][j] = args[i][j];
                    }
                }
            }

            // Construct an N x 1 matrix (vector) from an array of values
            Matrix(std::initializer_list<T> args) requires(columns == 1) {
                assert(args.size() == rows);

                auto row = args.begin();
                for (std::size_t i = 0; i < rows; i++) {
                    values[i][0] = *row;
                    row++;
                }
            }

            // Output the matrix to an io stream
            friend std::ostream& operator<<(std::ostream& os, const Matrix& matrix) {
                os << std::endl;
                
                for (std::size_t i = 0; i < rows; i++) {
                    os << '[';
                    for (std::size_t j = 0; j < columns; j++) {
                        os << matrix.values[i][j];
                        if (j + 1 < columns) os << ", ";
                    }
                    os << ']' << std::endl;
                }

                return os;
            }

            // Cast the matrix's values to a compatible type
            template <typename T2>
            requires (std::is_convertible_v<T, T2>)
            Matrix<rows, columns, T2> cast() const {
                Matrix<rows, columns, T2> matrix;

                for (std::size_t i = 0; i < rows; i++) {
                    for (std::size_t j = 0; j < columns; j++) {
                        matrix[i][j] = static_cast<T2>((*this)[i][j]);
                    }
                }

                return matrix;
            }

            // Return the determinant of a square matrix
            const T determinant() requires (rows == columns) {
                if constexpr (rows == 1) {
                    return values[0][0];
                }
                else if constexpr (rows == 2) {
                    return values[0][0] * values[1][1] - values[0][1] * values[1][0];
                }
                else { // size > 2: use Laplace expansion along the first row
                    T result = T();
                    for (std::size_t i = 0; i < columns; i++) {
                        T cofactor = ((i % 2 == 0) ? 1 : -1) * values[0][i];
                        Matrix<rows - 1, columns - 1, T> submatrix;

                        for (std::size_t j = 1; j < rows; j++) {
                            std::size_t subColumn = 0;
                            for (std::size_t k = 0; k < columns; k++) {
                                if (k == i) continue;
                                submatrix[j - 1][subColumn] = values[j][k];
                                subColumn++;
                            }
                        }
                        
                        result += cofactor * submatrix.determinant();
                    }
                    return result;
                }
            }

            // Return the matrix's row count
            static constexpr std::size_t size() { return rows; }
            
            // Return a mutable reference to the row at `index`
            auto& operator[](std::size_t index) { return values[index]; }

            // Return an immutable reference to the row at `index`
            const auto& operator[](std::size_t index) const { return values[index]; }

            // Return a mutable reference to the value at row `i` and column `j`
            T& getValue(std::size_t i, std::size_t j) { return values[i][j]; }

            // Return an immutable reference to the value at row `i` and column `j`
            const T& getValue(std::size_t i, std::size_t j) const { return values[i][j]; }

            // Post-multiply the matrix in-place by a compatible square matrix of the same type
            Matrix& operator*=(const Matrix<columns, columns, T>& matrix) {
                *this = *this * matrix;
                return *this;
            }

        private:
            std::array<std::array<T, columns>, rows> values{};
    };

    // TODO: inverses and matrix "division"

    /**
     * @brief Create a square identity matrix of a given data type and shape
     * 
     * @tparam dimension Number of rows and columns in the matrix
     * @tparam T Data type of the matrix's values
     * @param identity "Identity" value to fill the matrix's main diagonal. Defaults to the equivalent of '1' for the type `T`
     * @param nonIdentity Value to fill the rest of the matrix. Defaults to the default value of type `T`
     * @return constexpr Matrix<dimension, dimension, T> 
     */
    template <std::size_t dimension, typename T = double>
    constexpr Matrix<dimension, dimension, T> Identity(T identity = T(1), T nonIdentity = T()) {
        Matrix<dimension, dimension, T> matrix;

        if (nonIdentity != T()) {
            for (std::size_t i = 0; i < dimension; i++) {
                for (std::size_t j = 0; j < dimension; j++) {
                    matrix[i][j] = nonIdentity;
                }
            }
        }

        for (std::size_t i = 0; i < dimension; i++) {
            matrix[i][i] = identity;
        }

        return matrix;
    }

    // Return the transpose of `matrix` 
    template <std::size_t rows, std::size_t columns, typename T>
    constexpr Matrix<columns, rows, T> transpose(const Matrix<rows, columns, T>& matrix) {
        Matrix<columns, rows, T> transposed;

        for (std::size_t i = 0; i < matrix.shape[0]; i++) { 
            for (std::size_t j = 0; j < matrix.shape[1]; j++) { 
                transposed.getValue(j, i) = matrix.getValue(i, j);
            }
        }

        return transposed;
    }

    // Matrix-multiply the M x N matrix `left` by the N x P matrix `right` to return a new M x P matrix
    template <std::size_t rows, std::size_t columns, std::size_t columns2, typename T, typename T2>
    requires requires (T a, T2 b) { a * b; }
    auto operator*(const Matrix<rows, columns, T>& left, const Matrix<columns, columns2, T2>& right) {
        using Result = decltype(std::declval<T>() * std::declval<T2>());
        Matrix<rows, columns2, Result> product;

        for (std::size_t i = 0; i < rows; i++) {
            for (std::size_t j = 0; j < columns2; j++) {
                product.getValue(i, j) = left.getValue(i, 0) * right.getValue(0, j);
                for (std::size_t k = 1; k < columns; k++) {
                    product.getValue(i, j) += left.getValue(i, k) * right.getValue(k, j); 
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
                matrix.getValue(i, j) *= other;
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
                matrix.getValue(i, j) /= other;
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
                matrix.getValue(i, j) += other.getValue(i, j);
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
                matrix.getValue(i, j) -= other.getValue(i, j);
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
        using Common = std::common_type_t<typename M::MatrixDataType, typename M2::MatrixDataType>;
        auto result = left.template cast<Common>();
        return result += right;
    }

    // Subtract two matrices of the same shape
    template <typename M, typename M2>
    requires requires(typename M::MatrixDataType a, typename M2::MatrixDataType b) { a -= b; } && (M::shape == M2::shape)
    auto operator-(const M& left, const M2& right) {
        using Common = std::common_type_t<typename M::MatrixDataType, typename M2::MatrixDataType>;
        auto result = left.template cast<Common>();
        return result -= right;
    }

    /* -------------------------------------------------------------------------- */
    /*                                   Vectors                                  */
    /* -------------------------------------------------------------------------- */

    /**
    * @brief Vector class supporting different dimensions and data types
    * 
    * @tparam dimension Number of values held by the vector
    * @tparam T Data type of the vector's values
    */
    template <std::size_t dimension, typename T = double>
    requires (dimension > 0)
    class Vector : public Matrix<dimension, 1, T> {
        public:
            // Construct a vector with a default initialization of its values
            Vector() = default;

            // Construct a vector from an array of values
            Vector(std::initializer_list<T> args) : Matrix<dimension, 1, T>(args) { assert(args.size() == dimension); }

            // Construct a vector from an N x 1 matrix
            Vector(const Matrix<dimension, 1, T>& matrix) : Matrix<dimension, 1, T>(matrix) {}

            // Return a mutable reference to the value at `index`
            T& operator[](std::size_t index) { return Matrix<dimension, 1, T>::operator[](index)[0]; }

            // Return an immutable reference to the value at `index`
            const T& operator[](std::size_t index) const { return Matrix<dimension, 1, T>::operator[](index)[0]; }

            // Pre-multiply the vector in-place by a compatible square matrix of the same type
            Vector& operator*=(const Matrix<dimension, dimension, T>& matrix) {
                *this = matrix * *this;
                return *this;
            }
    };

    // Return the scalar/dot product between vectors `left` and `right`
    template <std::size_t dimension, typename T, typename T2>
    requires requires(T a, T2 b) {a * b;} 
    auto operator*(const Vector<dimension, T>& left, const Vector<dimension, T2>& right) {
        auto sum = left[0] * right[0];
        
        for (std::size_t i = 1; i < dimension; i++) {
            sum += left[i] * right[i];
        }

        return sum;
    }

    // Pre-multiply `vector` by `matrix` to return a new vector
    template <std::size_t rows, std::size_t columns, typename T, typename T2>
    requires requires (T a, T2 b) { a * b; }
    auto operator*(const Matrix<rows, columns, T>& matrix, const Vector<columns, T2>& vector) {
        using Result = decltype(std::declval<T>() * std::declval<T2>());
        const auto& converted = static_cast<const Matrix<rows, 1, T2>&>(vector);
        return Vector<rows, Result>(matrix * converted);
    }
}