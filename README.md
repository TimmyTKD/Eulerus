# Eulerus
A simple, general-purpose mathematics library for C++.

Requires C++20 or later.

## Overview
Below is a brief overview of each major component of Eulerus.

### Linear Algebra
This component focuses on providing generic `Vector` and `Matrix` classes, which can be of any dimension and store any data type that can be held in a regular array. 

These classes are provided alongside various operations and methods, such as the dot product between vectors, matrix-vector multiplication, and more.

### Set Theory
This component provides the generic `Set` class, which acts like a mathematical set and can store any collection of different data types that are hashable. 

Various operations are provided that can be performed on and between sets, such as taking the union, intersection, Cartesian product, and more.

### Number Theory
This component provides various functions that deal with integers and their properties, including divisibility and prime numbers. 

Some examples include finding the greatest common divisor of two integers alongside their Bézout coefficients, determining whether an integer is prime, and finding all the divisors of an integer.

### Calculus

This component mainly focuses on numerical differentiation and integration, such as providing functions that can approximate the derivative of a function at a given point, as well as the definite integral of a function over a given interval. 