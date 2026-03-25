# ─────────────────────────────────────────────────────────────────────
#  Makefile for algebraic field extension demos
#  Requires: g++17, cairo, fltk
# ─────────────────────────────────────────────────────────────────────
CXX      := g++
CXXFLAGS := -std=c++17 -O2 -Wall -Wextra
CAIRO    := $(shell pkg-config --cflags --libs cairo)
FLTK     := $(shell fltk-config --cxxflags --ldflags)

# Sources
SRCDIR     := ..
DEMOSDIR   := .
CORE_H     := $(SRCDIR)/algebraic_field.hpp

# Targets
STATIC_DEMOS := demo algebraic_viz cubic_viz snowflake_viz rotation_drift_viz explorer
INTERACTIVE_DEMOS := interactive_theodorus interactive_cubic interactive_snowflake interactive_rotation

all: $(STATIC_DEMOS) $(INTERACTIVE_DEMOS)

# Static Demos
demo: $(SRCDIR)/demo.cpp $(CORE_H)
	$(CXX) $(CXXFLAGS) $< -o $@ -lm

explorer: $(SRCDIR)/explorer.cpp $(CORE_H)
	$(CXX) $(CXXFLAGS) $< -o $@ -lm

algebraic_viz: $(DEMOSDIR)/algebraic_viz.cpp $(CORE_H)
	$(CXX) $(CXXFLAGS) $< -o $@ $(CAIRO) -lm

cubic_viz: $(DEMOSDIR)/cubic_viz.cpp $(CORE_H)
	$(CXX) $(CXXFLAGS) $< -o $@ $(CAIRO) -lm

snowflake_viz: $(DEMOSDIR)/snowflake_viz.cpp $(CORE_H)
	$(CXX) $(CXXFLAGS) $< -o $@ $(CAIRO) -lm

rotation_drift_viz: $(DEMOSDIR)/rotation_drift_viz.cpp $(CORE_H)
	$(CXX) $(CXXFLAGS) $< -o $@ $(CAIRO) -lm

# Interactive Demos
interactive_theodorus: $(SRCDIR)/interactive_theodorus.cpp $(CORE_H)
	$(CXX) $(CXXFLAGS) $< -o $@ $(FLTK) -lm

interactive_cubic: $(SRCDIR)/interactive_cubic.cpp $(CORE_H)
	$(CXX) $(CXXFLAGS) $< -o $@ $(FLTK) -lm

interactive_snowflake: $(SRCDIR)/interactive_snowflake.cpp $(CORE_H)
	$(CXX) $(CXXFLAGS) $< -o $@ $(FLTK) -lm

interactive_rotation: $(SRCDIR)/interactive_rotation.cpp $(CORE_H)
	$(CXX) $(CXXFLAGS) $< -o $@ $(FLTK) -lm

clean:
	rm -f $(STATIC_DEMOS) $(INTERACTIVE_DEMOS) *.png

.PHONY: all clean
