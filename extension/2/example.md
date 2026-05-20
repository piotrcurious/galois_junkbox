This demo implements the exact algebraic framework to solve a problem IEEE 754 floating-point cannot. By encoding the field extension directly into the type system, we can divide by irrationals, multiply them back, and compute exact norms without losing a single bit of precision.
The code is self-contained C++17. It uses std::gcd to normalize rationals and strictly enforces field boundaries at compile time.
### The C++ Implementation
```cpp
#include <iostream>
#include <numeric>
#include <cstdint>
#include <cassert>

// 1. The Base Field: Exact Rationals
template <typename IntType = int64_t>
struct Rational {
    IntType num, den;

    Rational(IntType n = 0, IntType d = 1) : num(n), den(d) { 
        if (den == 0) throw std::invalid_argument("Zero denominator");
        normalize(); 
    }

    void normalize() {
        IntType g = std::gcd(num, den);
        num /= g; den /= g;
        if (den < 0) { num = -num; den = -den; }
    }

    Rational operator+(const Rational& r) const { return Rational(num * r.den + r.num * den, den * r.den); }
    Rational operator-(const Rational& r) const { return Rational(num * r.den - r.num * den, den * r.den); }
    Rational operator*(const Rational& r) const { return Rational(num * r.num, den * r.den); }
    Rational operator/(const Rational& r) const { return Rational(num * r.den, den * r.num); }
    bool operator==(const Rational& r) const { return num == r.num && den == r.den; }

    friend std::ostream& operator<<(std::ostream& os, const Rational& r) {
        if (r.den == 1) return os << r.num;
        return os << r.num << "/" << r.den;
    }
};

// 2. The Algebraic Extension: Q(sqrt(D))
template <int64_t D, typename Base = Rational<int64_t>>
struct SqrtExt {
    Base a, b; // Represents: a + b*sqrt(D)

    SqrtExt(Base a = Base(0), Base b = Base(0)) : a(a), b(b) {}

    Base norm() const {
        return a * a - Base(D) * b * b;
    }

    SqrtExt operator+(const SqrtExt& rhs) const { return SqrtExt(a + rhs.a, b + rhs.b); }
    SqrtExt operator-(const SqrtExt& rhs) const { return SqrtExt(a - rhs.a, b - rhs.b); }
    
    // Polynomial multiplication mod x^2 - D
    SqrtExt operator*(const SqrtExt& rhs) const {
        return SqrtExt(
            a * rhs.a + Base(D) * b * rhs.b, 
            a * rhs.b + b * rhs.a
        );
    }

    // Exact division via Galois conjugate
    SqrtExt operator/(const SqrtExt& rhs) const {
        Base n = rhs.norm();
        if (n == Base(0)) throw std::invalid_argument("Division by zero");
        return SqrtExt(
            (a * rhs.a - Base(D) * b * rhs.b) / n,
            (b * rhs.a - a * rhs.b) / n
        );
    }

    bool operator==(const SqrtExt& rhs) const { return a == rhs.a && b == rhs.b; }

    friend std::ostream& operator<<(std::ostream& os, const SqrtExt& ext) {
        os << ext.a;
        if (!(ext.b == Base(0))) {
            os << (ext.b.num >= 0 ? " + " : " - ");
            Rational<int64_t> abs_b(std::abs(ext.b.num), ext.b.den);
            if (!(abs_b == Base(1))) os << abs_b << "*";
            os << "sqrt(" << D << ")";
        }
        return os;
    }
};

int main() {
    using Q = Rational<int64_t>;
    using Q_sqrt2 = SqrtExt<2, Q>;

    std::cout << "--- Exact Algebraic Arithmetic Demo ---\n\n";

    // 1. Prove the algebraic identity: (sqrt(2))^2 == 2
    Q_sqrt2 sqrt2(Q(0), Q(1)); // 0 + 1*sqrt(2)
    Q_sqrt2 two(Q(2), Q(0));   // 2 + 0*sqrt(2)
    
    Q_sqrt2 result = sqrt2 * sqrt2;
    std::cout << "1. Evaluating (sqrt(2))^2:\n";
    std::cout << "   " << sqrt2 << " * " << sqrt2 << " = " << result << "\n";
    assert(result == two);
    std::cout << "   [Assertion Passed: Result is EXACTLY 2]\n\n";

    // 2. Exact Division without approximation
    Q_sqrt2 alpha(Q(3), Q(1)); // 3 + sqrt(2)
    Q_sqrt2 one(Q(1), Q(0));   // 1
    
    Q_sqrt2 inverse = one / alpha;
    std::cout << "2. Exact Division (1 / alpha):\n";
    std::cout << "   alpha   = " << alpha << "\n";
    std::cout << "   1/alpha = " << inverse << "\n";
    
    // Prove it reverses exactly
    Q_sqrt2 recovered = alpha * inverse;
    std::cout << "   alpha * (1/alpha) = " << recovered << "\n";
    assert(recovered == one);
    std::cout << "   [Assertion Passed: Recovered EXACTLY 1]\n\n";

    // 3. Solving Pell's Equation Norm (p^2 - 2q^2)
    // The 3rd continued fraction convergent of sqrt(2) is 7/5
    Q_sqrt2 convergent(Q(7), Q(5)); // Represents 7 - 5*sqrt(2)
    std::cout << "3. Pell Equation via Norm:\n";
    std::cout << "   Convergent element : 7 - 5*sqrt(2)\n";
    std::cout << "   Norm (7^2 - 2*5^2) : " << Q_sqrt2(Q(7), Q(-5)).norm() << "\n";

    return 0;
}

```
### Execution Output
If you compile and run this program (g++ -std=c++17 main.cpp), the terminal output will be:
```text
--- Exact Algebraic Arithmetic Demo ---

1. Evaluating (sqrt(2))^2:
   0 + sqrt(2) * 0 + sqrt(2) = 2
   [Assertion Passed: Result is EXACTLY 2]

2. Exact Division (1 / alpha):
   alpha   = 3 + sqrt(2)
   1/alpha = 3/7 - 1/7*sqrt(2)
   alpha * (1/alpha) = 1
   [Assertion Passed: Recovered EXACTLY 1]

3. Pell Equation via Norm:
   Convergent element : 7 - 5*sqrt(2)
   Norm (7^2 - 2*5^2) : -1

```
### Why this architecture succeeds
 * **Closure under division:** When we compute 1 / (3 + sqrt(2)), the system applies the Galois conjugate rule internally. It computes the norm 3^2 - 2(1)^2 = 7, and returns exactly \frac{3}{7} - \frac{1}{7}\sqrt{2}. It does not rely on Taylor series or Newton-Raphson approximation.
 * **Zero float propagation:** Because a and b are exact fraction structs rather than IEEE 754 floats, the rounding errors that usually compound during matrix inversion or geometric intersections are mathematically eliminated.
 * **Compile-time domain walls:** If you attempted to create SqrtExt<3> sqrt3(0, 1); and do sqrt2 + sqrt3, the C++ compiler would throw a fatal error. Floating-point silently erases the distinction between \sqrt{2} and \sqrt{3}; the type system physically prevents mixing fields without explicitly lifting them into a shared degree-4 composite field.
