Now writing the full multi-panel graphical demo — this will be a substantial program. Let me design and implement all four visualization panels.

Clean compile — only cosmetic warnings. Let me fix those and finalize.

---

## Build and run

```bash
# Install deps (once)
sudo apt install libcairo2-dev libx11-dev

# Build both targets
make

# Run the graphical demo
./algebraic_viz
```

## What the four panels show

**Panel 1 — Spiral of Theodorus.** 45 right triangles wound around the origin, each with legs of length 1 and √k. The algebraic version computes each radius as a **fresh** algebraic root `√(n+1)` — no dependency on the previous step. The float iterative version propagates the previous float vertex coordinates forward via perpendicular rotation, accumulating ULP drift. A bottom sub-panel plots the per-step radius error on a magnified scale.

**Panel 2 — Pell Catastrophic Cancellation.** The most dramatic panel. For Pell convergents $(p,q)$ to $\sqrt{2}$, the algebraic Galois norm gives $N(p - q\sqrt{2}) = p^2 - 2q^2 = \pm 1$ **exactly** (computed via `__int128`). When $p$ exceeds $2^{26}$, the quantity $p^2$ exceeds IEEE 754's 53-bit mantissa, and the float computation `p*p - 2.0*q*q` suffers catastrophic cancellation — the result diverges from $\pm 1$ by orders of magnitude shown on a log scale. The gold vertical line marks the overflow threshold.

**Panel 3 — Golden Rectangle Decomposition.** Nine levels of recursive subdivision, all ratios exactly $\varphi = (1+\sqrt{5})/2 \in \mathbb{Q}(\sqrt{5})$. Every rectangle dimension is an exact element `Q5{a, b}` with `.approx()` called once per rectangle at render time. The right sidebar shows the algebraic proof: $\varphi^2 = \varphi + 1$ is verified as `phi*phi == phi + Q5(rat(1))` — an algebraic equality, not a float comparison. The golden spiral is drawn with quarter-circle arcs at exact square corners.

**Panel 4 — $(1+\sqrt{2})^n$ Power Sequence.** Left: log-scale bar chart of the companion Pell number pairs $(a_n, b_n)$ where $(1+\sqrt{2})^n = a_n + b_n\sqrt{2}$ — blue for $a_n$, purple for $b_n$, growing as $2.414^n$. Right: the Galois norm invariant $N((1+\sqrt{2})^n) = (-1)^n$ plotted in both algebraic (exact, at machine epsilon) and float iterative forms, showing that the float iterative norm loses the multiplicativity $N(\alpha\beta) = N(\alpha)N(\beta)$ that Galois theory guarantees for the algebraic representation.
