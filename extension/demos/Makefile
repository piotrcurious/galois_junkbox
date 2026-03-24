# ─────────────────────────────────────────────────────────────────────
#  Makefile for algebraic field extension demos
#  Requires: g++17, cairo, x11 (libcairo2-dev, libx11-dev)
# ─────────────────────────────────────────────────────────────────────
CXX      := g++
CXXFLAGS := -std=c++17 -O2 -Wall -Wextra
CAIRO    := $(shell pkg-config --cflags --libs cairo-xlib x11)

all: algebraic_viz demo

algebraic_viz: algebraic_viz.cpp algebraic_field.hpp
	$(CXX) $(CXXFLAGS) $< -o $@ $(CAIRO) -lm

demo: demo.cpp algebraic_field.hpp
	$(CXX) $(CXXFLAGS) $< -o $@ -lm

clean:
	rm -f algebraic_viz demo

.PHONY: all clean
