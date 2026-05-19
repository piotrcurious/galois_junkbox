# Pell Numbers in Galois Theory

A clean way to place Pell numbers inside Galois theory is to work in the quadratic field $\mathbb{Q}(\sqrt{2})$.

The Pell numbers are defined by the recurrence:
$$P_0=0, \quad P_1=1, \quad P_n=2P_{n-1}+P_{n-2}$$

This yields the closed-form Binet formula:
$$P_n = \frac{(1+\sqrt{2})^n - (1-\sqrt{2})^n}{2\sqrt{2}}$$

## The Galois-Theoretic Picture

$\mathbb{Q}(\sqrt{2})/\mathbb{Q}$ is a quadratic extension, so its Galois group $\text{Gal}(\mathbb{Q}(\sqrt{2})/\mathbb{Q})$ has order **2**. The nontrivial automorphism $\sigma$ fixes $\mathbb{Q}$ and sends $\sqrt{2} \mapsto -\sqrt{2}$. More generally, a Galois group consists of the field automorphisms that leave the base field fixed.

Take the fundamental unit $\alpha = 1+\sqrt{2}$. Its Galois conjugate is $\sigma(\alpha) = 1-\sqrt{2}$. The field norm on $\mathbb{Q}(\sqrt{2})$ is:
$$N(x+y\sqrt{2}) = (x+y\sqrt{2})(x-y\sqrt{2}) = x^2 - 2y^2$$

Because the norm is multiplicative, we have:
$$N(\alpha) = (1+\sqrt{2})(1-\sqrt{2}) = -1$$
$$N(\alpha^n) = (-1)^n$$

If we expand $\alpha^n = x_n + y_n\sqrt{2}$, this norm identity gives us the classic Pell's equation:
$$x_n^2 - 2y_n^2 = (-1)^n$$

Here, the coefficients map precisely to the sequences:
- $y_n = P_n$ (Pell numbers)
- $x_n = P_n + P_{n-1}$ (Pell-Lucas numbers / companion sequence)

## Conclusion

The Galois-theoretic picture is this: the group swaps the two conjugates $\alpha^n$ and $\sigma(\alpha^n)$, and the Pell numbers are the integer coefficients that appear when you expand powers of $\alpha$. In that sense, Pell numbers are not "a Galois group" themselves; they are the arithmetic shadow of a very small Galois group acting on a quadratic field.
