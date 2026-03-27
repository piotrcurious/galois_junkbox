#include "../include/algebraic_field.hpp"
#include <iostream>
#include <cassert>
#include <vector>

#define TEST(name) void test_##name() { std::cout << "Running " << #name << "... ";
#define END_TEST std::cout << "PASSED\n"; }

TEST(rational_arithmetic)
    Q a(1, 2), b(1, 3);
    assert((a + b) == Q(5, 6));
    assert((a - b) == Q(1, 6));
    assert((a * b) == Q(1, 6));
    assert((a / b) == Q(3, 2));
    assert(Q(2, 4) == Q(1, 2)); // Reduction
    assert(Q(-1, -1) == Q(1, 1));
END_TEST

TEST(sqrt2_extension)
    Q2 a(rat(1), rat(1)), b(rat(2), rat(-1)); // 1+sqrt2, 2-sqrt2
    // (1+s2)(2-s2) = 2 - s2 + 2s2 - 2 = s2
    assert((a * b) == Q2(rat(0), rat(1)));
    assert(a.norm() == rat(-1));
    assert(b.norm() == rat(2));
    assert((a * b).norm() == a.norm() * b.norm()); // Multiplicativity
    assert((a / a) == Q2(rat(1), rat(0)));
END_TEST

TEST(cubic_extension)
    Qcbrt2 a(rat(1), rat(1), rat(0)); // 1 + cbrt2
    Qcbrt2 b = a.inv();
    assert((a * b) == Qcbrt2(rat(1), rat(0), rat(0)));
    assert(a.norm() == rat(3)); // 1^3 + 2*1^3 + 4*0^3 - 0 = 3
END_TEST

TEST(complex_extensions)
    Qi i(rat(0), rat(1));
    assert((i * i) == Qi(rat(-1), rat(0)));

    Qomega w(rat(0), rat(1)); // w = e^2pi i/3
    // w^2 + w + 1 = 0
    assert((w * w + w + Qomega(rat(1), rat(0))) == Qomega(rat(0), rat(0)));
END_TEST

TEST(i128_overflow)
    // Check if __int128 handles large Fibonacci-like growth in Pell units
    Q2 unit(rat(1), rat(1));
    Q2 p = unit;
    for(int i=0; i<30; ++i) p = p * unit;
    assert(p.norm() == rat(1) || p.norm() == rat(-1));
END_TEST

TEST(poly_extension)
    // Q[x]/(x^2 - 2) which is Q(sqrt2)
    // min_poly: x^2 + 0x - 2 = 0 => { -2, 0 }
    PolyExt<2> a({rat(1), rat(1)}, {rat(-2), rat(0)}); // 1 + x
    PolyExt<2> b({rat(2), rat(-1)}, {rat(-2), rat(0)}); // 2 - x
    auto res = a * b; // (1+x)(2-x) = 2 + x - x^2 = 2 + x - 2 = x
    assert(res.coeffs[0] == rat(0));
    assert(res.coeffs[1] == rat(1));
END_TEST

int main() {
    std::cout << "--- ALGEBRAIC FIELD UNIT TESTS ---\n";
    test_rational_arithmetic();
    test_sqrt2_extension();
    test_cubic_extension();
    test_complex_extensions();
    test_i128_overflow();
    test_poly_extension();
    std::cout << "----------------------------------\n";
    std::cout << "ALL TESTS PASSED SUCCESSFULLY\n";
    return 0;
}
