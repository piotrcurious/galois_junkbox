# ─────────────────────────────────────────────────────────────────────
#  Makefile for algebraic field extension demos
#  Requires: g++17, cairo
# ─────────────────────────────────────────────────────────────────────
CXX      := g++
CXXFLAGS := -std=c++17 -O2 -Wall -Wextra
CAIRO    := $(shell pkg-config --cflags --libs cairo)

SRCDIR  := ..
DEMODIR := .

all: demo algebraic_viz cubic_viz snowflake_viz rotation_drift_viz explorer

demo: $(SRCDIR)/demo.cpp $(SRCDIR)/algebraic_field.hpp
	$(CXX) $(CXXFLAGS) $< -o $@ -lm

explorer: $(SRCDIR)/explorer.cpp $(SRCDIR)/algebraic_field.hpp
	$(CXX) $(CXXFLAGS) $< -o $@ -lm

algebraic_viz: $(DEMODIR)/algebraic_viz.cpp $(SRCDIR)/algebraic_field.hpp
	$(CXX) $(CXXFLAGS) $< -o $@ $(CAIRO) -lm

cubic_viz: $(DEMODIR)/cubic_viz.cpp $(SRCDIR)/algebraic_field.hpp
	$(CXX) $(CXXFLAGS) $< -o $@ $(CAIRO) -lm

snowflake_viz: $(DEMODIR)/snowflake_viz.cpp $(SRCDIR)/algebraic_field.hpp
	$(CXX) $(CXXFLAGS) $< -o $@ $(CAIRO) -lm

rotation_drift_viz: $(DEMODIR)/rotation_drift_viz.cpp $(SRCDIR)/algebraic_field.hpp
	$(CXX) $(CXXFLAGS) $< -o $@ $(CAIRO) -lm

clean:
	rm -f demo algebraic_viz cubic_viz snowflake_viz rotation_drift_viz explorer *.png

.PHONY: all clean
