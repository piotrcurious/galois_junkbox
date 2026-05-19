A clean way to place Pell numbers inside Galois theory is to work in the quadratic field . The Pell numbers are defined by

P_0=0,\quad P_1=1,\quad P_n=2P_{n-1}+P_{n-2},

P_n=\frac{(1+\sqrt2)^n-(1-\sqrt2)^n}{2\sqrt2}. \, \text{0}

Now the Galois-theory part:  is a quadratic extension, so its Galois group has order . The nontrivial automorphism fixes  and sends . More generally, a Galois group consists of the field automorphisms that leave the base field fixed. 

Take . Its Galois conjugate is . The field norm on  is

N(x+y\sqrt2)=(x+y\sqrt2)(x-y\sqrt2)=x^2-2y^2,

N(\alpha)=(1+\sqrt2)(1-\sqrt2)=-1,

N(\alpha^n)=(-1)^n.

\alpha^n=x_n+y_n\sqrt2,

x_n^2-2y_n^2=(-1)^n.

So the Galois-theoretic picture is this: the group  swaps the two conjugates , and the Pell numbers are the integer coefficients that appear when you expand powers of . In that sense, Pell numbers are not “a Galois group” themselves; they are the arithmetic shadow of a very small Galois group acting on a quadratic field. 
