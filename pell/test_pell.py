import unittest
import sys
import os

# Add the parent directory to sys.path to allow running the test directly
sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from pell.pell import Q_sqrt2

class TestPell(unittest.TestCase):
    def test_pell_sequence(self):
        # P_n = 0, 1, 2, 5, 12, 29, 70, 169, 408, 985...
        expected_pell = [0, 1, 2, 5, 12, 29, 70, 169, 408, 985]
        alpha = Q_sqrt2(1, 1)
        for n, expected in enumerate(expected_pell):
            alpha_n = alpha**n
            self.assertEqual(alpha_n.b, expected, f"Pell number mismatch at n={n}")

    def test_norm_identity(self):
        alpha = Q_sqrt2(1, 1)
        for n in range(10):
            alpha_n = alpha**n
            expected_norm = (-1)**n
            self.assertEqual(alpha_n.norm(), expected_norm, f"Norm mismatch at n={n}")

    def test_recurrence(self):
        # P_n = 2*P_{n-1} + P_{n-2}
        # x_n = 2*x_{n-1} + x_{n-2}
        alpha = Q_sqrt2(1, 1)
        for n in range(2, 10):
            p_n = (alpha**n).b
            p_n_1 = (alpha**(n-1)).b
            p_n_2 = (alpha**(n-2)).b
            self.assertEqual(p_n, 2 * p_n_1 + p_n_2, f"Pell recurrence failed at n={n}")

            x_n = (alpha**n).a
            x_n_1 = (alpha**(n-1)).a
            x_n_2 = (alpha**(n-2)).a
            self.assertEqual(x_n, 2 * x_n_1 + x_n_2, f"x_n recurrence failed at n={n}")

    def test_algebraic_operations(self):
        a = Q_sqrt2(1, 2) # 1 + 2*sqrt(2)
        b = Q_sqrt2(3, 4) # 3 + 4*sqrt(2)

        # Addition: (1+3) + (2+4)*sqrt(2) = 4 + 6*sqrt(2)
        c = a + b
        self.assertEqual(c.a, 4)
        self.assertEqual(c.b, 6)

        # Subtraction: (1-3) + (2-4)*sqrt(2) = -2 - 2*sqrt(2)
        d = a - b
        self.assertEqual(d.a, -2)
        self.assertEqual(d.b, -2)

        # Multiplication: (1+2*sqrt(2))*(3+4*sqrt(2)) = (1*3 + 2*2*4) + (1*4 + 2*3)*sqrt(2)
        # = (3 + 16) + (4 + 6)*sqrt(2) = 19 + 10*sqrt(2)
        e = a * b
        self.assertEqual(e.a, 19)
        self.assertEqual(e.b, 10)

if __name__ == '__main__':
    unittest.main()
