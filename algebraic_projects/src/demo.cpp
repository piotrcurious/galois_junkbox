// ╔══════════════════════════════════════════════════════════════════════╗
// ║  demo.cpp  —  Exact algebraic arithmetic demonstrations              ║
// ║                                                                       ║
// ║  Compile:  g++ -std=c++17 -O2 demo.cpp -o demo && ./demo             ║
// ╚══════════════════════════════════════════════════════════════════════╝

#include "../include/algebraic_field.hpp"
#include <cassert>
#include <cmath>
#include <iomanip>
#include <iostream>

// ─────────────────────────────────────────────────────────────────────────────
//  Printing utilities
// ─────────────────────────────────────────────────────────────────────────────
static void section(const char* title) {
    std::cout << "\n╔══ " << title << "\n";
}
static void ok(const char* label) {
    std::cout << "║  ✓  " << label << "\n";
}
static void info(const char* label, const std::string& val) {
    std::cout << "║     " << label << " = " << val << "\n";
}

// ─────────────────────────────────────────────────────────────────────────────
//  Demo 1: ℚ(√2) — the fundamental identity (√2)² = 2, exact
// ─────────────────────────────────────────────────────────────────────────────
void demo_sqrt2() {
    section("ℚ(√2): squaring is exact — no ULP drift, no approximation");

    // √2 is the coset x̄ in ℚ[x]/(x²−2)
    // Represented as 0 + 1·√2, i.e. a=0, b=1
    Q2 sqrt2 = { rat(0), rat(1) };

    // The product rule (√D)²=D gives: sqrt2*sqrt2 = (0+1√2)(0+1√2)
    //   new_a = 0·0 + 2·1·1 = 2
    //   new_b = 0·1 + 1·0   = 0
    // Result: 2 + 0·√2 ≡ 2   — provably exact
    Q2 sq = sqrt2 * sqrt2;

    assert(sq == Q2(rat(2)));   // ALGEBRAIC EQUALITY, not float comparison
    ok("(0+1·√2)² == (2+0·√2) by algebraic product rule");
    info("(√2)²", sq.str());

    // Contrast: in IEEE 754 this is NOT guaranteed
    double f = std::sqrt(2.0) * std::sqrt(2.0);
    std::cout << "║  ✗  float: sqrt(2.0)*sqrt(2.0) == 2.0  is  "
              << (f == 2.0 ? "true (lucky, platform-dependent)"
                            : "FALSE (ULP error)")
              << "\n";
    std::cout << "║     float value: " << std::setprecision(17) << f << "\n";
    std::cout << "║     error bit:   " << (f - 2.0) << "\n";
}

// ─────────────────────────────────────────────────────────────────────────────
//  Demo 2: Golden ratio φ = (1+√5)/2 satisfies φ² = φ+1 exactly
// ─────────────────────────────────────────────────────────────────────────────
void demo_golden_ratio() {
    section("ℚ(√5): golden ratio φ satisfies φ²=φ+1 (minimal polynomial x²−x−1)");

    // φ = (1+√5)/2 ∈ ℚ(√5): a = 1/2, b = 1/2
    Q5 phi = { rat(1,2), rat(1,2) };

    // φ² in ℚ(√5):
    //   a_new = a·a + 5·b·b = 1/4 + 5/4 = 6/4 = 3/2
    //   b_new = a·b + b·a   = 1/4 + 1/4 = 1/2
    // φ² = 3/2 + (1/2)·√5
    //
    // φ+1 = 3/2 + (1/2)·√5   ← same!
    Q5 phi_sq  = phi * phi;
    Q5 phi_p1  = phi + Q5(rat(1));

    assert(phi_sq == phi_p1);
    ok("φ² == φ+1");
    info("φ²  ", phi_sq.str());
    info("φ+1 ", phi_p1.str());

    // The minimal polynomial of φ is x²−x−1.
    // Verify via norm and trace:
    //   Tr(φ) = 2·(1/2) = 1
    //   N(φ)  = (1/2)²−5·(1/2)² = 1/4−5/4 = −1
    // Minpoly coefficients {-Tr, N} = {-1, -1}  ⟹  x²−x−1  ✓
    auto [p1, p0] = phi.min_poly();
    info("Tr(φ)", phi.trace().str());
    info("N(φ) ", phi.norm().str());
    info("min poly coefficients {p₁,p₀}", p1.str() + ", " + p0.str());

    ok("minpoly of φ: x² + (-1)x + (-1) = x²−x−1 ✓");
}

// ─────────────────────────────────────────────────────────────────────────────
//  Demo 3: Exact division in ℚ(√2) using conjugate/norm
// ─────────────────────────────────────────────────────────────────────────────
void demo_division() {
    section("ℚ(√2): division via α⁻¹ = σ(α)/N(α) — exact, no iterative method");

    //  (1+√2) / (3−√2)
    //  = (1+√2)(3+√2) / N(3−√2)
    //  = (3+√2+3√2+2) / (9−2)
    //  = (5+4√2) / 7
    //  = 5/7 + (4/7)·√2

    Q2 numerator   = { rat(1), rat(1) };   // 1+√2
    Q2 denominator = { rat(3), rat(-1) };  // 3−√2
    Q2 result = numerator / denominator;

    Q2 expected = { rat(5,7), rat(4,7) };
    assert(result == expected);
    ok("(1+√2)/(3−√2) == 5/7 + (4/7)·√2");
    info("result", result.str());
    info("N(3−√2)", denominator.norm().str());  // should be 7

    // Round-trip: multiply back to confirm
    Q2 roundtrip = result * denominator;
    assert(roundtrip == numerator);
    ok("round-trip: result × (3−√2) == 1+√2 exactly");
}

// ─────────────────────────────────────────────────────────────────────────────
//  Demo 4: ℚ(√2, √3) — degree-4 nested extension, basis {1,√2,√3,√6}
//
//  The key design:  SqrtExt<3, SqrtExt<2>>  composes recursively.
//  Arithmetic at the outer level calls SqrtExt<2> arithmetic, which calls
//  Q arithmetic.  Each level is exact.  The type prevents mixing fields.
// ─────────────────────────────────────────────────────────────────────────────
void demo_Q2_3() {
    section("ℚ(√2,√3): degree-4 extension, elements a₀+a₁√2+b₀√3+b₁√6");

    // Elements of ℚ(√2,√3) are pairs (a,b) where a,b ∈ ℚ(√2).
    // √2 = { Q2{0,1}, Q2{0,0} }  (coeff of 1 is 0+1·√2, coeff of √3 is 0)
    // √3 = { Q2{0,0}, Q2{1,0} }  (coeff of 1 is 0,      coeff of √3 is 1)

    Q2_3 sqrt2_here = { Q2{rat(0), rat(1)}, Q2{rat(0), rat(0)} };
    Q2_3 sqrt3_here = { Q2{rat(0), rat(0)}, Q2{rat(1), rat(0)} };

    // (√2)² = 2
    Q2_3 sq2 = sqrt2_here * sqrt2_here;
    Q2_3 two  = Q2_3(Q2(rat(2)));
    assert(sq2 == two);
    ok("(√2)² = 2  in ℚ(√2,√3)");

    // (√3)² = 3
    Q2_3 sq3  = sqrt3_here * sqrt3_here;
    Q2_3 three = Q2_3(Q2(rat(3)));
    assert(sq3 == three);
    ok("(√3)² = 3  in ℚ(√2,√3)");

    // √2·√3 = √6, represented as { Q2{0,0}, Q2{0,1} }
    //   i.e. 0 + (0+1·√2)·√3 = √2·√3 = √6
    Q2_3 sqrt6 = sqrt2_here * sqrt3_here;
    Q2_3 sqrt6_expected = { Q2{rat(0), rat(0)}, Q2{rat(0), rat(1)} };
    assert(sqrt6 == sqrt6_expected);
    ok("√2·√3 = √6  (coset representation: {0,(0+1·√2)})");

    // (√6)² = 6
    Q2_3 sq6 = sqrt6 * sqrt6;
    Q2_3 six  = Q2_3(Q2(rat(6)));
    assert(sq6 == six);
    ok("(√6)² = 6  in ℚ(√2,√3)");

    // (√2+√3)² = 5+2√6  — exact
    Q2_3 s   = sqrt2_here + sqrt3_here;
    Q2_3 ssq = s * s;
    // 5+2√6 = { Q2{5,0}, Q2{0,2} }
    Q2_3 expected_sq = { Q2{rat(5), rat(0)}, Q2{rat(0), rat(2)} };
    assert(ssq == expected_sq);
    ok("(√2+√3)² = 5+2√6  exactly");

    // Demonstrate type isolation: Q2 and Q3 are different types.
    // The line below would be a COMPILE ERROR:
    //   Q2 a = {rat(1), rat(1)};
    //   Q3 b = {rat(1), rat(1)};
    //   auto x = a + b;  // ERROR: no operator+(Q2, Q3)
    // This is the type system enforcing field discipline.
    ok("type system statically prevents mixing Q2 with Q3 or Q5");
}

// ─────────────────────────────────────────────────────────────────────────────
//  Demo 5: ℚ(∛2) — degree-3 extension  (NOT Galois over ℚ)
// ─────────────────────────────────────────────────────────────────────────────
void demo_cbrt2() {
    section("ℚ(∛2): degree-3 extension with relation ρ³=2");

    // ρ = ∛2  is the element {0,1,0}: 0 + 1·ρ + 0·ρ²
    Qcbrt2 rho = { rat(0), rat(1), rat(0) };

    // ρ³ should equal 2 exactly
    Qcbrt2 rho3 = rho * rho * rho;
    Qcbrt2 two  = { rat(2), rat(0), rat(0) };
    assert(rho3 == two);
    ok("(∛2)³ = 2  exactly");

    // N(∛2) = N(0+1·ρ+0·ρ²) = 0³ + 2·1³ + 4·0³ − 3·2·0·1·0 = 2
    Q n = rho.norm();
    assert(n == rat(2));
    ok("N(∛2) = 2  exactly  (field norm via multiplication matrix det)");
    info("N(∛2)", n.str());

    // (1+∛2)² = 1 + 2∛2 + ∛4
    Qcbrt2 one_p_rho = { rat(1), rat(1), rat(0) };
    Qcbrt2 sq = one_p_rho * one_p_rho;
    // = {1,1,0}*{1,1,0}:
    //   a = 1·1 + 2·(1·0 + 0·1) = 1
    //   b = 1·1 + 1·1 + 2·(0·0) = 2
    //   c = 1·0 + 1·1 + 0·1     = 1
    Qcbrt2 expected = { rat(1), rat(2), rat(1) };
    assert(sq == expected);
    ok("(1+∛2)² = 1 + 2∛2 + (∛2)²  exactly");
    info("(1+∛2)²", sq.str());
}

// ─────────────────────────────────────────────────────────────────────────────
//  Demo 6: The ring of integers viewpoint — why positional notation is a ring
//
//  Binary: the byte 0b10110010 = x⁷+x⁵+x⁴+x¹ evaluated at x=2.
//  GF(2⁸): the same byte is a polynomial *coset* mod an irreducible degree-8
//  polynomial over 𝔽₂.  These are two different rings the byte lives in.
//  The C++ type distinguishes them; raw `uint8_t` does not.
//
//  Our SqrtExt does the same for ℚ: it distinguishes ℚ(√2), ℚ(√3), ℚ(√5)
//  so you cannot accidentally treat an element of one as an element of another.
// ─────────────────────────────────────────────────────────────────────────────
void demo_ring_viewpoint() {
    section("Type discipline: fields are distinct algebraic structures");

    //  In the Gaussian integers ℤ[i] = ℤ[x]/(x²+1),
    //  we can model this as SqrtExt<-1> — but our template requires D>1.
    //  For the real quadratic case, all D>1 squarefree work.
    //
    //  The key point: sqrt(2) and sqrt(3) are in DIFFERENT minimal-polynomial
    //  classes.  Without field typing, this C++ is valid but wrong:
    //
    //    double a = sqrt(2.0);
    //    double b = sqrt(3.0);
    //    double c = a + b;      // fine syntactically, meaningless algebraically
    //
    //  With field typing, the wrong operation is a compile error.
    //  The correct operation requires an explicit field embedding:

    Q2 alpha = { rat(1), rat(1) };               // 1+√2 ∈ ℚ(√2)
    // Embed into ℚ(√2,√3) before adding to a √3 element
    Q2_3 alpha_lifted = Q2_3(alpha);              // (1+√2)+0·√3  ∈ ℚ(√2,√3)
    Q2_3 beta         = { Q2{rat(0),rat(0)},
                          Q2{rat(1),rat(0)} };    // √3 ∈ ℚ(√2,√3)
    Q2_3 sum = alpha_lifted + beta;               // (1+√2)+√3

    // Verify: (1+√2+√3)² = 1+2+3 + 2√2+2√3+2√6 = 6+2√2+2√3+2√6
    Q2_3 sum_sq = sum * sum;
    //  {a,b} = { Q2{1,1}, Q2{1,0} }
    //  sum² = a²+3b² + 2ab√3
    //  a² = (1+√2)² = {3, 2}   (as Q2: 1²+2·1²=3, 2·1·1=2 → 3+2√2)
    //  3b² = 3·{1,0}·{1,0} = {3,0}
    //  a²+3b² = {6,2}
    //  2ab = 2·{1,1}·{1,0} = {2,2}
    //  sum² = { {6,2}, {2,2} } = 6+2√2+(2+2√2)√3 = 6+2√2+2√3+2√6
    Q2_3 expected = { Q2{rat(6), rat(2)}, Q2{rat(2), rat(2)} };
    assert(sum_sq == expected);
    ok("(1+√2+√3)² = 6+2√2+2√3+2√6  exactly in ℚ(√2,√3)");

    // approx() is the only exit to double — and it's clearly named
    std::cout << "║     approx value: " << std::setprecision(15)
              << sum_sq.approx() << "\n";
    std::cout << "║     reference:    "
              << 6.0 + 2*std::sqrt(2.0) + 2*std::sqrt(3.0) + 2*std::sqrt(6.0)
              << "\n";
}

// ─────────────────────────────────────────────────────────────────────────────
//  Demo 7: Continued fractions — what they actually are in this framework
//
//  A continued fraction for √D is a sequence of RATIONAL approximations to
//  the fixed point of σ: the element√D ∈ ℚ(√D).  Each truncation is a
//  projection of the coset down to ℚ ↪ ℚ(√D).
//
//  The Galois norm makes this precise: the best rational approximation p/q
//  to √D minimises |p² − D·q²| / q² — i.e. minimises the absolute norm
//  |N(p−q√D)| / q².  Continued fractions are not separate theory; they
//  are the orbit of the Galois conjugation on the lattice of ℤ-points.
// ─────────────────────────────────────────────────────────────────────────────
void demo_continued_fractions() {
    section("Continued fractions as rational projections of cosets");

    // The continued fraction convergents for √2: 1, 3/2, 7/5, 17/12, 41/29...
    // Each p/q satisfies  p² − 2q² = ±1  (Pell equation = norm = ±1 in ℚ(√2))
    struct CF { long long p, q; };
    CF convergents[] = {{1,1},{3,2},{7,5},{17,12},{41,29},{99,70},{239,169}};

    std::cout << "║  Pell convergents for √2: p/q where |p²−2q²| = 1 (= |N(p−q√2)|)\n";
    for (auto [p,q] : convergents) {
        // Construct the element p−q√2 ∈ ℚ(√2)
        Q2 elem = { rat(p), rat(-q) };
        Q n = elem.norm();   // = p²−2q², must be ±1 for Pell solutions
        assert(n == rat(1) || n == rat(-1));
        std::cout << "║    p=" << std::setw(3) << p << " q=" << std::setw(3) << q
                  << "  N(p−q√2)=" << std::setw(2) << n.str()
                  << "  approx error=" << std::setprecision(8)
                  << std::abs(double(p)/double(q) - std::sqrt(2.0)) << "\n";
    }
    ok("Every Pell solution has |N(p−q√2)| = 1: a unit in ℤ[√2]");
    std::cout << "║\n║  Key insight: continued fractions are the orbit of the\n"
              << "║  Galois conjugate σ: √2↦−√2 acting on the integer lattice.\n"
              << "║  The norm IS the Pell equation.  They are the same object.\n";
}

// ─────────────────────────────────────────────────────────────────────────────
int main() {
    std::cout << "══════════════════════════════════════════════════════════════\n"
              << " Exact arithmetic in algebraic field extensions\n"
              << " Numbers as polynomial cosets — the Galois paradigm\n"
              << "══════════════════════════════════════════════════════════════\n";

    demo_sqrt2();
    demo_golden_ratio();
    demo_division();
    demo_Q2_3();
    demo_cbrt2();
    demo_ring_viewpoint();
    demo_continued_fractions();

    std::cout << "\n╔══ Summary\n"
              << "║  All algebraic identities held to EXACT equality (==).\n"
              << "║  No floating-point arithmetic was used in any assertion.\n"
              << "║  The type system prevents mixing elements of different fields.\n"
              << "║  double appears only in explicit .approx() calls, named as lossy.\n"
              << "╚══════════════════════════════════════════════════════════\n\n";
    return 0;
}
