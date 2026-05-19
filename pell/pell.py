class Q_sqrt2:
    """Represents a number in the field Q(sqrt(2)) of the form a + b*sqrt(2)."""
    def __init__(self, a, b):
        self.a = int(a)
        self.b = int(b)

    def __add__(self, other):
        return Q_sqrt2(self.a + other.a, self.b + other.b)

    def __sub__(self, other):
        return Q_sqrt2(self.a - other.a, self.b - other.b)

    def __mul__(self, other):
        # (a + b*sqrt(2)) * (c + d*sqrt(2)) = (ac + 2bd) + (ad + bc)sqrt(2)
        new_a = self.a * other.a + 2 * self.b * other.b
        new_b = self.a * other.b + self.b * other.a
        return Q_sqrt2(new_a, new_b)

    def __pow__(self, n):
        res = Q_sqrt2(1, 0)
        base = self
        while n > 0:
            if n % 2 == 1:
                res = res * base
            base = base * base
            n //= 2
        return res

    def conjugate(self):
        return Q_sqrt2(self.a, -self.b)

    def norm(self):
        # N(a + b*sqrt(2)) = a^2 - 2*b^2
        return self.a**2 - 2 * self.b**2

    def __repr__(self):
        if self.b == 0: return f"{self.a}"
        if self.a == 0: return f"{self.b}*sqrt(2)"
        sign = "+" if self.b > 0 else "-"
        abs_b = abs(self.b)
        b_str = f"{abs_b}*sqrt(2)" if abs_b != 1 else "sqrt(2)"
        return f"{self.a} {sign} {b_str}"

def main():
    alpha = Q_sqrt2(1, 1)

    print("--- Expanding Powers of Alpha (alpha = 1 + sqrt(2)) ---")
    for n in range(7):
        alpha_n = alpha**n
        print(f"alpha^{n} = {str(alpha_n):<18} -> x_{n} = {alpha_n.a}, y_{n} (Pell) = {alpha_n.b}")

    print("\n--- Galois Conjugation & The Norm ---")
    n = 4
    alpha_n = alpha**n
    alpha_n_conj = alpha_n.conjugate()
    norm = alpha_n.norm()

    print(f"For n = {n}:")
    print(f"  alpha^{n}            = {alpha_n}")
    print(f"  Galois Conjugate   = {alpha_n_conj}")
    print(f"  Field Norm (x^2 - 2y^2) = ({alpha_n}) * ({alpha_n_conj}) = {norm}  (which is (-1)^{n})")

    print("\n--- Binet's Formula via Galois Conjugates ---")
    # P_n = (alpha^n - sigma(alpha)^n) / (alpha - sigma(alpha))
    # alpha - sigma(alpha) = (1 + sqrt(2)) - (1 - sqrt(2)) = 2*sqrt(2)
    print("Pell numbers generated via Galois conjugates:")
    pell_sequence = []
    for i in range(7):
        num = (alpha**i - alpha.conjugate()**i)
        # num should be of the form 0 + k*sqrt(2)
        # P_i = num.b / 2
        pell_sequence.append(num.b // 2)
    print(pell_sequence)

if __name__ == "__main__":
    main()
