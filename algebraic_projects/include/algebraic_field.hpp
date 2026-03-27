// ╔══════════════════════════════════════════════════════════════════════════╗
// ║  algebraic_field.hpp                                                      ║
// ║  Exact arithmetic via algebraic field extensions encoded in C++ types     ║
// ╚══════════════════════════════════════════════════════════════════════════╝

#pragma once
#include <cmath>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>
#include <numeric>
#include <algorithm>
#include <initializer_list>
#include <complex>

// Helper to convert __int128 to string
inline std::string i128_to_str(__int128 n) {
    if (n == 0) return "0";
    std::string s = "";
    bool neg = false;
    if (n < 0) { neg = true; n = -n; }
    while (n > 0) {
        s += (char)('0' + (n % 10));
        n /= 10;
    }
    if (neg) s += '-';
    std::reverse(s.begin(), s.end());
    return s;
}

// ═══════════════════════════════════════════════════════════════════
//  §1  Exact rational arithmetic — the base field ℚ
//      Using 128-bit integers for high dynamic range.
// ═══════════════════════════════════════════════════════════════════
class Q {
public:
    using i128 = __int128;
    i128 n = 0, d = 1;

    constexpr Q() = default;
    constexpr Q(i128 num, i128 den = 1) : n(num), d(den) { canon(); }

    constexpr Q  operator-()  const noexcept { return {-n, d}; }
    constexpr Q  operator+(const Q& r) const { return {n * r.d + r.n * d, d * r.d}; }
    constexpr Q  operator-(const Q& r) const { return {n * r.d - r.n * d, d * r.d}; }
    constexpr Q  operator*(const Q& r) const { return {n * r.n, d * r.d}; }
    constexpr Q  operator/(const Q& r) const {
        if (r.n == 0) throw std::domain_error("Q: division by zero");
        return {n * r.d, d * r.n};
    }
    constexpr bool operator==(const Q& r) const noexcept { return n==r.n && d==r.d; }
    constexpr bool operator!=(const Q& r) const noexcept { return !(*this==r); }
    constexpr bool operator< (const Q& r) const noexcept { return n*r.d < r.n*d; }

    [[nodiscard]] double approx() const noexcept { return (double)n/(double)d; }

    [[nodiscard]] std::string str() const {
        if (d == 1) return i128_to_str(n);
        return i128_to_str(n) + "/" + i128_to_str(d);
    }

private:
    static constexpr i128 gcd_128(i128 a, i128 b) noexcept {
        if (a < 0) a = -a; if (b < 0) b = -b;
        while (b) { a %= b; i128 t = a; a = b; b = t; } return a ? a : 1;
    }
    constexpr void canon() noexcept {
        if (d == 0) return;
        if (d < 0) { n=-n; d=-d; }
        i128 g = gcd_128(n, d);
        n /= g; d /= g;
    }
};

inline constexpr Q rat(long long p, long long q=1) noexcept { return Q((__int128)p, (__int128)q); }

// ═══════════════════════════════════════════════════════════════════════
//  §2  Quadratic field extension:  Base(√D)
// ═══════════════════════════════════════════════════════════════════════
template<int D, typename Base = Q>
class SqrtExt {
public:
    Base a, b;  // a + b√D
    constexpr SqrtExt() : a(0), b(0) {}
    constexpr SqrtExt(Base a, Base b) : a(a), b(b) {}
    constexpr explicit SqrtExt(long long n) : a(Base(n)), b(Base(0)) {}
    constexpr explicit SqrtExt(Base r) : a(r), b(Base(0)) {}

    constexpr SqrtExt operator-() const { return {-a, -b}; }
    constexpr SqrtExt operator+(const SqrtExt& r) const { return {a+r.a, b+r.b}; }
    constexpr SqrtExt operator-(const SqrtExt& r) const { return {a-r.a, b-r.b}; }
    constexpr SqrtExt operator*(const SqrtExt& r) const {
        Base Db((long long)D);
        return { a*r.a + Db*b*r.b, a*r.b + b*r.a };
    }

    [[nodiscard]] constexpr Base norm() const {
        return a*a - Base((long long)D)*b*b;
    }
    [[nodiscard]] constexpr Base trace() const { return Base(2LL)*a; }
    [[nodiscard]] constexpr std::pair<Base,Base> min_poly() const { return { -trace(), norm() }; }

    constexpr SqrtExt inv() const {
        Base n = norm();
        return { a/n, (-b)/n };
    }
    constexpr SqrtExt operator/(const SqrtExt& r) const { return (*this)*r.inv(); }
    constexpr bool operator==(const SqrtExt& r) const { return a==r.a && b==r.b; }
    constexpr bool operator!=(const SqrtExt& r) const { return !(*this==r); }
    constexpr bool operator< (const SqrtExt& r) const { return approx() < r.approx(); }

    [[nodiscard]] double approx() const noexcept {
        if constexpr (D >= 0) return a.approx() + b.approx() * std::sqrt((double)D);
        else return a.approx();
    }

    std::pair<double, double> approx_complex() const {
        if constexpr (D < 0) return {a.approx(), b.approx() * std::sqrt((double)-D)};
        else return {approx(), 0.0};
    }

    [[nodiscard]] std::string str() const {
        if (b == Base(0LL)) return a.str();
        std::string sign;
        if (D == -1) sign = "i";
        else if (D < 0) sign = "i√" + std::to_string(-D);
        else sign = "√" + std::to_string(D);
        return "(" + a.str() + ") + (" + b.str() + ")" + sign;
    }
};

using Q2 = SqrtExt<2>;
using Q3 = SqrtExt<3>;
using Q5 = SqrtExt<5>;
using Qi = SqrtExt<-1>;
using Q2_3 = SqrtExt<3, Q2>;

// ═══════════════════════════════════════════════════════════════════
//  §3  Eisenstein Rationals Q(ω) where ω = e^{2πi/3}
//      ω² + ω + 1 = 0 => ω² = -ω - 1
// ═══════════════════════════════════════════════════════════════════
class Qomega {
public:
    Q a, b; // a + bω
    Qomega(Q a={}, Q b={}) : a(a), b(b) {}
    Qomega operator+(const Qomega& r) const { return {a+r.a, b+r.b}; }
    Qomega operator-(const Qomega& r) const { return {a-r.a, b-r.b}; }
    Qomega operator-() const { return {-a, -b}; }
    Qomega operator*(const Qomega& r) const {
        // (a + bω)(c + dω) = ac + (ad + bc)ω + bdω²
        //                  = ac + (ad + bc)ω + bd(-ω - 1)
        //                  = (ac - bd) + (ad + bc - bd)ω
        return {a*r.a - b*r.b, a*r.b + b*r.a - b*r.b};
    }
    bool operator==(const Qomega& r) const { return a==r.a && b==r.b; }
    bool operator!=(const Qomega& r) const { return !(*this==r); }

    std::pair<double, double> approx() const {
        double re = a.approx() - 0.5 * b.approx();
        double im = 0.5 * std::sqrt(3.0) * b.approx();
        return {re, im};
    }
    std::string str() const { return "(" + a.str() + ") + (" + b.str() + ")ω"; }
};

// ═══════════════════════════════════════════════════════════════════
//  §4  Cubic Extension ℚ(∛D)
// ═══════════════════════════════════════════════════════════════════
template<int D>
class CubicExt {
public:
    Q a, b, c; // a + bρ + cρ² where ρ = ∛D
    CubicExt(Q a={}, Q b={}, Q c={}) : a(a), b(b), c(c) {}
    CubicExt operator+(const CubicExt& r) const { return {a+r.a, b+r.b, c+r.c}; }
    CubicExt operator-(const CubicExt& r) const { return {a-r.a, b-r.b, c-r.c}; }
    CubicExt operator-() const { return {-a, -b, -c}; }
    CubicExt operator*(const CubicExt& r) const {
        Q Dq((long long)D);
        return { a*r.a + Dq*(b*r.c + c*r.b),
                 a*r.b + b*r.a + Dq*(c*r.c),
                 a*r.c + b*r.b + c*r.a };
    }
    [[nodiscard]] Q norm() const {
        Q Dq((long long)D);
        return a*a*a + Dq*b*b*b + Dq*Dq*c*c*c - Q(3LL)*Dq*a*b*c;
    }

    // α⁻¹ = (1/N(α)) * [ (a² - Dbc) + (Dc² - ab)ρ + (b² - ac)ρ² ]
    CubicExt inv() const {
        Q n = norm();
        Q Dq((long long)D);
        return { (a*a - Dq*b*c)/n, (Dq*c*c - a*b)/n, (b*b - a*c)/n };
    }
    CubicExt operator/(const CubicExt& r) const { return (*this)*r.inv(); }

    bool operator==(const CubicExt& r) const { return a==r.a && b==r.b && c==r.c; }
    bool operator!=(const CubicExt& r) const { return !(*this==r); }

    std::string str() const {
        return "(" + a.str() + ") + (" + b.str() + ")·∛" + std::to_string(D) + " + (" + c.str() + ")·∛" + std::to_string(D) + "²";
    }
    double approx() const {
        double r = std::cbrt((double)D);
        return a.approx() + b.approx()*r + c.approx()*r*r;
    }
};

using Qcbrt2 = CubicExt<2>;

// ═══════════════════════════════════════════════════════════════════
//  §5  General Polynomial Extension: Base[x]/(f(x))
//      Minimal polynomial: f(x) = x^Degree + c_{Degree-1}x^{Degree-1} + ... + c_0
// ═══════════════════════════════════════════════════════════════════
template<int Degree, typename Base = Q>
class PolyExt {
    std::vector<Base> min_poly_coeffs;
public:
    std::vector<Base> coeffs;

    PolyExt(std::initializer_list<Base> c, std::initializer_list<Base> mp)
        : min_poly_coeffs(mp), coeffs(c) {
        coeffs.resize(Degree, Base(0));
    }

    PolyExt operator+(const PolyExt& r) const {
        PolyExt res = *this;
        for(int i=0; i<Degree; ++i) res.coeffs[i] = res.coeffs[i] + r.coeffs[i];
        return res;
    }

    PolyExt operator*(const PolyExt& r) const {
        std::vector<Base> res(2 * Degree - 1, Base(0));
        for(int i=0; i<Degree; ++i) {
            for(int j=0; j<Degree; ++j) {
                res[i+j] = res[i+j] + coeffs[i] * r.coeffs[j];
            }
        }
        for(int i = 2 * Degree - 2; i >= Degree; --i) {
            Base m = res[i];
            for(int j=0; j<Degree; ++j) {
                res[i - Degree + j] = res[i - Degree + j] - m * min_poly_coeffs[j];
            }
            res[i] = Base(0);
        }
        res.resize(Degree);
        return {res, min_poly_coeffs};
    }

    PolyExt(const std::vector<Base>& c, const std::vector<Base>& mp)
        : min_poly_coeffs(mp), coeffs(c) {}

    bool operator==(const PolyExt& r) const { return coeffs == r.coeffs; }

    std::string str() const {
        std::string s = "";
        for(int i=0; i<Degree; ++i) {
            if(coeffs[i] != Base(0)) {
                if(!s.empty()) s += " + ";
                s += "(" + coeffs[i].str() + ")";
                if(i > 0) s += "x^" + std::to_string(i);
            }
        }
        return s.empty() ? "0" : s;
    }
};
