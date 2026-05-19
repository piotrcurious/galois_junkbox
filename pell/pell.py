import sympy as sp

# Define the symbol for sqrt(2) to keep calculations exact and algebraic
sqrt2 = sp.sqrt(2)

# 1. Define the fundamental unit alpha
alpha = 1 + sqrt2

print("--- Expanding Powers of Alpha ---")
# Let's look at the first 6 powers of alpha: alpha^n = x_n + y_n*sqrt(2)
for n in range(6):
    # Expand the power algebraically
    expr = sp.expand(alpha**n)
    
    # Extract the coefficients: x_n (rational part) and y_n (coefficient of sqrt(2))
    # as_coeff_Add() splits the expression into (rational_part, irrational_part)
    x_n, irrational = expr.as_coeff_Add()
    
    # Extract y_n by dividing out the sqrt(2)
    y_n = irrational.as_coeff_Mul()[0] if irrational != 0 else 0
    
    print(f"alpha^{n} = {expr:<18} -> x_{n} = {x_n}, y_{n} (Pell) = {y_n}")

print("\n--- Galois Conjugation & The Norm ---")
# 2. Demonstrate the Galois Automorphism (sqrt(2) -> -sqrt(2))
# We pick n = 4 as an example
n = 4
alpha_n = sp.expand(alpha**n)

# The nontrivial Galois automorphism swaps sqrt(2) with -sqrt(2)
alpha_n_conjugate = alpha_n.subs(sqrt2, -sqrt2)

# Calculate the Field Norm: N(beta) = beta * sigma(beta)
field_norm = sp.expand(alpha_n * alpha_n_conjugate)

print(f"For n = {n}:")
print(f"  alpha^{n}            = {alpha_n}")
print(f"  Galois Conjugate   = {alpha_n_conjugate}")
print(f"  Field Norm (x^2 - 2y^2) = {alpha_n} * {alpha_n_conjugate} = {field_norm}  (which is (-1)^{n})")

print("\n--- Binet's Formula via Galois Conjugates ---")
# 3. Reconstruct Pell numbers using Binet's formula derived from the conjugates
def binet_pell(n):
    # alpha and its Galois conjugate
    a = 1 + sqrt2
    b = 1 - sqrt2
    # Binet's formula: (alpha^n - sigma(alpha)^n) / (alpha - sigma(alpha))
    return sp.simplify((a**n - b**n) / (2 * sqrt2))

print("Pell numbers generated via Galois conjugates:")
pell_sequence = [binet_pell(i) for i in range(6)]
print(pell_sequence)
