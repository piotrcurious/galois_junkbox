// ╔══════════════════════════════════════════════════════════════════════════╗
// ║  algebraic_field.hpp                                                      ║
// ║  Exact arithmetic via algebraic field extensions encoded in C++ types     ║
// ║                                                                            ║
// ║  Compile:  g++ -std=c++17 -O2 demo.cpp -o demo                            ║
// ║                                                                            ║
// ║  Core claim:                                                               ║
// ║    An irrational algebraic number α is not an approximation target.        ║
// ║    It is an exact coset in the quotient ring Q[x]/(f), where f is its      ║
// ║    minimal polynomial.  Elements are tuples of rationals; operations are   ║
// ║    polynomial arithmetic mod f.  No float enters until you call .approx(). ║
// ║                                                                            ║
// ║  Mathematical background                                                   ║
// ║  ─────────────────────────────────────────────────────────────────         ║
// ║  Positional notation is polynomial evaluation:                             ║
// ║    1011₂  = x³+x+1 evaluated at x=2 ∈ ℤ[x]                               ║
// ║    0xAB   = polynomial over 𝔽₂ in GF(2⁸) = 𝔽₂[x]/(x⁸+x⁴+x³+x+1)        ║
// ║    √2     = coset x̄  in  ℚ[x]/(x²-2) ≅ ℚ(√2)                            ║
// ║                                                                            ║
// ║  All three are the same construction: quotient ring arithmetic.            ║
// ║  AES already does exact GF(2⁸) arithmetic in hardware.  We extend         ║
// ║  the same idea to algebraic numbers over ℚ.                               ║
// ╚══════════════════════════════════════════════════════════════════════════╝

#pragma once
#include <cmath>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

// ═══════════════════════════════════════════════════════════════════
//  §1  Exact rational arithmetic — the base field ℚ
//
//  Invariant: n/d is always fully reduced with d > 0.
//  This is the coefficient domain for all higher extensions.
// ═══════════════════════════════════════════════════════════════════
class Q {
public:
    long long n = 0, d = 1;

    constexpr Q() = default;
    constexpr Q(long long num, long long den = 1) : n(num), d(den) { canon(); }

    constexpr Q  operator-()  const noexcept { return {-n, d}; }
    constexpr Q  operator+(Q r) const { return {n*r.d + r.n*d, d*r.d}; }
    constexpr Q  operator-(Q r) const { return {n*r.d - r.n*d, d*r.d}; }
    constexpr Q  operator*(Q r) const { return {n*r.n, d*r.d}; }
    constexpr Q  operator/(Q r) const {
        if (r.n == 0) throw std::domain_error("Q: division by zero");
        return {n*r.d, d*r.n};
    }
    constexpr bool operator==(Q r) const noexcept { return n==r.n && d==r.d; }
    constexpr bool operator!=(Q r) const noexcept { return !(*this==r); }
    constexpr bool operator< (Q r) const noexcept { return n*r.d < r.n*d; }

    // ── Explicit lossy escape hatch ───────────────────────────────
    // This is the ONLY place a float appears.  Name it to signal intent.
    [[nodiscard]] double approx() const noexcept { return double(n)/double(d); }

    [[nodiscard]] std::string str() const {
        return (d==1) ? std::to_string(n)
                      : std::to_string(n)+"/"+std::to_string(d);
    }

private:
    constexpr void canon() noexcept {
        if (d < 0) { n=-n; d=-d; }
        long long g = gcd(n<0?-n:n, d);
        n /= g; d /= g;
    }
    static constexpr long long gcd(long long a, long long b) noexcept {
        while (b) { a%=b; long long t=a; a=b; b=t; } return a ? a : 1;
    }
};

// ═══════════════════════════════════════════════════════════════════════
//  §2  Degree-2 field extension:  Base(√D)
//
//  Template parameters:
//    D    — positive squarefree integer radicand
//    Base — coefficient field (defaults to ℚ; can be a prior SqrtExt)
//
//  An element is a pair (a, b) representing   a + b·√D,   a,b ∈ Base.
//  The single defining relation  (√D)² = D  is encoded in operator*.
//
//  This is exactly ℚ[x]/(x²−D) lifted to work over any field, with
//  the product rule:
//    (a + b·√D)(c + d·√D)  =  (ac + D·bd)  +  (ad + bc)·√D
//
//  Galois structure:
//    Gal(Base(√D)/Base) ≅ ℤ/2ℤ,  generator σ: √D ↦ −√D
//    Norm  N(α) = α·σ(α) = a²−Db²  ∈ Base  (multiplicative)
//    Trace Tr(α) = α+σ(α) = 2a      ∈ Base  (additive)
//    Minpoly: x² − Tr(α)·x + N(α)  ∈ Base[x]
//    Inverse: α⁻¹ = σ(α)/N(α)
//
//  Nesting: SqrtExt<3, SqrtExt<2>>  gives ℚ(√2,√3), a degree-4
//  extension of ℚ with basis {1, √2, √3, √6}.  Each level of nesting
//  is exact; no float propagates upward.
// ═══════════════════════════════════════════════════════════════════════
template<int D, typename Base = Q>
class SqrtExt {
    static_assert(D > 1, "radicand D must be a squarefree integer > 1");
public:
    Base a, b;  // element  =  a + b·√D

    // ── Constructors ────────────────────────────────────────────
    constexpr SqrtExt()                         : a(0LL), b(0LL)          {}
    constexpr SqrtExt(Base a, Base b)           : a(std::move(a)),
                                                  b(std::move(b))         {}
    // Embed integer: n  ↦  n + 0·√D
    constexpr explicit SqrtExt(long long n)     : a(n), b(0LL)            {}
    // Embed Base element: r  ↦  r + 0·√D
    constexpr explicit SqrtExt(Base r)          : a(std::move(r)), b(0LL) {}

    // ── Field arithmetic ─────────────────────────────────────────
    constexpr SqrtExt operator-()             const { return {-a, -b}; }
    constexpr SqrtExt operator+(SqrtExt r)   const { return {a+r.a, b+r.b}; }
    constexpr SqrtExt operator-(SqrtExt r)   const { return {a-r.a, b-r.b}; }

    //  Product rule: (√D)·(√D) ≡ D   ←  polynomial mod x²−D
    constexpr SqrtExt operator*(SqrtExt r)   const {
        Base Db(static_cast<long long>(D));   // embed D into Base
        return { a*r.a + Db*b*r.b,
                 a*r.b + b*r.a };
    }

    // ── Galois theory ─────────────────────────────────────────────
    //  Galois conjugate  σ(a + b√D) = a − b√D
    [[nodiscard]] constexpr SqrtExt conj()  const { return {a, -b}; }

    //  Field norm   N(α) = α·σ(α) = a²−Db²  ∈ Base
    //  Key property: N(αβ) = N(α)·N(β)  — multiplicativity is EXACT
    [[nodiscard]] constexpr Base norm()  const {
        return a*a - Base(static_cast<long long>(D))*b*b;
    }

    //  Field trace   Tr(α) = α+σ(α) = 2a   ∈ Base
    [[nodiscard]] constexpr Base trace() const { return Base(2LL)*a; }

    //  Minimal polynomial coefficients  {p₁, p₀}  of  x² + p₁x + p₀
    [[nodiscard]] constexpr std::pair<Base,Base> min_poly() const {
        return { -trace(), norm() };  // i.e. x² − Tr·x + N
    }

    //  Inverse: α⁻¹ = σ(α)/N(α)   — reduces field division to Base division
    //  Works recursively: if Base = SqrtExt<2>, Base::operator/ is used
    [[nodiscard]] constexpr SqrtExt inv() const {
        Base n = norm();
        // n==0 iff α==0; leave undefined-behaviour detection to Base::operator/
        return { a/n, (-b)/n };
    }
    constexpr SqrtExt operator/(SqrtExt r)  const { return (*this)*r.inv(); }

    // ── Equality ─────────────────────────────────────────────────
    constexpr bool operator==(SqrtExt r) const { return a==r.a && b==r.b; }
    constexpr bool operator!=(SqrtExt r) const { return !(*this==r); }

    // ── Explicit lossy escape hatch ───────────────────────────────
    //  Converts to double by evaluating the real embedding.
    //  Calling this discards the algebraic identity stored in the type.
    //  For nested extensions, calls Base::approx() recursively — each
    //  level introduces its own floating-point rounding.
    [[nodiscard]] double approx() const noexcept {
        return a.approx() + b.approx() * std::sqrt(double(D));
    }

    [[nodiscard]] std::string str() const {
        if (b == Base(0LL)) return a.str();
        return a.str() + " + (" + b.str() + ")·√" + std::to_string(D);
    }
};

// ═══════════════════════════════════════════════════════════════════
//  §3  Degree-3 field extension:  ℚ(∛D)
//
//  Elements: a + b·ρ + c·ρ²   where ρ = ∛D,  a,b,c ∈ ℚ
//  Defining relation: ρ³ = D
//  Minimal polynomial: x³ − D  (irreducible over ℚ for squarefree D>1)
//
//  Important structural note:
//    ℚ(∛D)/ℚ is NOT a Galois extension.  The splitting field of x³−D
//    is ℚ(∛D, ω) where ω = e^(2πi/3) is the primitive cube root of
//    unity.  The two complex roots ω·ρ and ω²·ρ lie outside ℚ(∛D).
//    Gal(ℚ(∛D,ω)/ℚ) ≅ S₃  (the symmetric group on 3 letters).
//
//    This does not prevent exact arithmetic in ℚ(∛D) itself — it means
//    the norm and trace are computed using the *full* splitting field
//    but land in ℚ.
//
//  Field norm (product of element with all conjugates):
//    N(a+bρ+cρ²) = a³ + Db³ + D²c³ − 3Dabc
//    Derivable as det of the multiplication matrix over ℚ.
//
//  Product rule (from ρ³=D):
//    (a+bρ+cρ²)(p+qρ+rρ²) =
//        (ap + D(br+cq)) + (aq+bp + D·cr)·ρ + (ar+bq+cp)·ρ²
// ═══════════════════════════════════════════════════════════════════
template<int D>
class CubicExt {
    static_assert(D > 1, "D must be squarefree > 1");
public:
    Q a, b, c;  // element  =  a + b·ρ + c·ρ²  where ρ=∛D

    CubicExt(Q a={}, Q b={}, Q c={}) : a(a), b(b), c(c) {}

    CubicExt operator-()            const { return {-a,-b,-c}; }
    CubicExt operator+(CubicExt r)  const { return {a+r.a, b+r.b, c+r.c}; }
    CubicExt operator-(CubicExt r)  const { return {a-r.a, b-r.b, c-r.c}; }

    //  Product rule derived from ρ³=D:  ρ⁴=D·ρ,  ρ⁵=D·ρ²
    CubicExt operator*(CubicExt r)  const {
        Q Dq{D};
        return {
            a*r.a + Dq*(b*r.c + c*r.b),   // ρ⁰ coefficient
            a*r.b + b*r.a + Dq*(c*r.c),   // ρ¹ coefficient
            a*r.c + b*r.b + c*r.a          // ρ² coefficient
        };
    }

    //  Field norm:  N(α) = det(multiplication matrix)
    //             = a³ + Db³ + D²c³ − 3Dabc
    [[nodiscard]] Q norm() const {
        Q Dq{D};
        return a*a*a + Dq*b*b*b + Dq*Dq*c*c*c - Q{3}*Dq*a*b*c;
    }

    bool operator==(CubicExt r) const { return a==r.a && b==r.b && c==r.c; }
    bool operator!=(CubicExt r) const { return !(*this==r); }

    [[nodiscard]] double approx() const noexcept {
        double rho = std::cbrt(double(D));
        return a.approx() + b.approx()*rho + c.approx()*rho*rho;
    }

    [[nodiscard]] std::string str() const {
        return a.str() + " + (" + b.str() + ")·∛" + std::to_string(D)
             + " + (" + c.str() + ")·∛" + std::to_string(D) + "²";
    }
};

// ═══════════════════════════════════════════════════════════════════
//  §4  Convenience aliases and helper
// ═══════════════════════════════════════════════════════════════════
using Q2      = SqrtExt<2>;          // ℚ(√2)  — roots of x²−2
using Q3      = SqrtExt<3>;          // ℚ(√3)  — roots of x²−3
using Q5      = SqrtExt<5>;          // ℚ(√5)  — contains golden ratio φ
using Q7      = SqrtExt<7>;
using Q2_3    = SqrtExt<3, Q2>;      // ℚ(√2,√3) — degree 4 over ℚ, basis {1,√2,√3,√6}
using Qcbrt2  = CubicExt<2>;         // ℚ(∛2)  — roots of x³−2

// Rational literal helper: rat(p,q) returns p/q ∈ ℚ
inline constexpr Q rat(long long p, long long q=1) noexcept { return {p,q}; }
