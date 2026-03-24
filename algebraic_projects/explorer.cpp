#include "algebraic_field.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <cmath>

void print_help() {
    std::cout << "\nAlgebraic Explorer - Exact Arithmetic vs Floats\n";
    std::cout << "Available Commands:\n";
    std::cout << "  sqrt2   : Show (sqrt(2))^2 exact vs float\n";
    std::cout << "  golden  : Show phi^2 = phi + 1 exact vs float\n";
    std::cout << "  drift2  : Show norm( (1+sqrt(2))^n ) exact vs float\n";
    std::cout << "  exit    : Quit explorer\n\n";
}

void show_sqrt2() {
    Q2 s2 = { rat(0), rat(1) };
    Q2 s2sq = s2 * s2;
    double fs2 = std::sqrt(2.0);
    double fs2sq = fs2 * fs2;

    std::cout << "\n--- (sqrt(2))^2 ---\n";
    std::cout << "Exact Q(sqrt(2)): " << s2sq.str() << " (Value: " << s2sq.approx() << ")\n";
    std::cout << "Float64:          " << std::setprecision(20) << fs2sq << "\n";
    std::cout << "Absolute error:   " << std::abs(s2sq.approx() - fs2sq) << "\n";
}

void show_golden() {
    Q5 phi = { rat(1,2), rat(1,2) };
    Q5 phi_sq = phi * phi;
    Q5 phi_p1 = phi + Q5(rat(1));

    double fphi = (1.0 + std::sqrt(5.0)) / 2.0;
    double fphi_sq = fphi * fphi;
    double fphi_p1 = fphi + 1.0;

    std::cout << "\n--- phi^2 vs phi + 1 ---\n";
    std::cout << "Exact Q(sqrt(5)):\n";
    std::cout << "  phi^2   = " << phi_sq.str() << "\n";
    std::cout << "  phi + 1 = " << phi_p1.str() << "\n";
    std::cout << "  Equal?    " << (phi_sq == phi_p1 ? "YES" : "NO") << "\n";

    std::cout << "Float64:\n";
    std::cout << "  phi^2   = " << std::setprecision(20) << fphi_sq << "\n";
    std::cout << "  phi + 1 = " << fphi_p1 << "\n";
    std::cout << "  Equal?    " << (fphi_sq == fphi_p1 ? "YES" : "NO (typically)") << "\n";
    std::cout << "  Abs error: " << std::abs(fphi_sq - fphi_p1) << "\n";
}

void show_drift2() {
    Q2 base = { rat(1), rat(1) };
    long double fa = 1.0, fb = 0.0;
    Q2 curQ = { rat(1), rat(0) };

    std::cout << "\n--- (1 + sqrt(2))^n Norm --- (Should be 1 or -1)\n";
    std::cout << std::setw(5) << "n" << " | " << std::setw(15) << "Exact Norm" << " | " << std::setw(25) << "Float64 Norm" << "\n";
    std::cout << std::string(55, '-') << "\n";

    for (int n = 0; n <= 60; ++n) {
        if (n % 10 == 0) {
            long double normF = fa*fa - 2.0L*fb*fb;
            std::cout << std::setw(5) << n << " | "
                      << std::setw(15) << curQ.norm().str() << " | "
                      << std::fixed << std::setprecision(1) << std::setw(25) << (double)normF << "\n";
        }
        // Iterate exact
        curQ = curQ * base;
        // Iterate float
        long double nfa = fa + 2.0L*fb;
        long double nfb = fa + fb;
        fa = nfa; fb = nfb;
    }
    std::cout << "\nNote: As n increases, a and b grow exponentially.\n"
              << "At n=60, a and b are approx 10^23. Floating point (IEEE 754 double)\n"
              << "has only 15-17 digits of precision, making it impossible to represent\n"
              << "the small difference between a^2 and 2*b^2 (which is exactly +/-1).\n";
}

int main() {
    print_help();
    std::string cmd;
    while (std::cout << "> ", std::cin >> cmd) {
        if (cmd == "exit") break;
        else if (cmd == "sqrt2") show_sqrt2();
        else if (cmd == "golden") show_golden();
        else if (cmd == "drift2") show_drift2();
        else if (cmd == "help") print_help();
        else std::cout << "Unknown command. Type 'help'.\n";
    }
    return 0;
}
