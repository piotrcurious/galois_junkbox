# ─────────────────────────────────────────────────────────────────────
#  Comprehensive Makefile for algebraic field extension suite
#  Requires: g++17, cairo, fltk
# ─────────────────────────────────────────────────────────────────────
CXX      := g++
CXXFLAGS := -std=c++17 -O2 -Wall -Wextra -Iinclude
CAIRO    := $(shell pkg-config --cflags --libs cairo)
FLTK     := $(shell fltk-config --cxxflags --ldflags)

SRCDIR   := src
EXDIR    := examples
BINDIR   := bin
OUTDIR   := output
INCDIR   := include
CORE_H   := $(INCDIR)/algebraic_field.hpp

# Targets
STATIC_DEMOS  := demo explorer tests
EXAMPLE_DEMOS := algebraic_viz cubic_viz snowflake_viz rotation_drift_viz
INTERACTIVE   := interactive_theodorus interactive_cubic interactive_snowflake \
                 interactive_rotation interactive_polygons interactive_fft \
                 interactive_dashboard interactive_lattice

ALL_BINARIES := $(addprefix $(BINDIR)/, $(STATIC_DEMOS) $(EXAMPLE_DEMOS) $(INTERACTIVE))

.PHONY: all clean run_examples test

all: $(BINDIR) $(OUTDIR) $(ALL_BINARIES)

$(BINDIR):
	mkdir -p $(BINDIR)

$(OUTDIR):
	mkdir -p $(OUTDIR)

# Static Source Demos
$(BINDIR)/%: $(SRCDIR)/%.cpp $(CORE_H)
	$(CXX) $(CXXFLAGS) $< -o $@ -lm

# Example Visualizations (Cairo)
$(BINDIR)/%: $(EXDIR)/%.cpp $(CORE_H)
	$(CXX) $(CXXFLAGS) $< -o $@ $(CAIRO) -lm

# Interactive Demos (FLTK)
$(BINDIR)/interactive_%: $(SRCDIR)/interactive_%.cpp $(CORE_H)
	$(CXX) $(CXXFLAGS) $< -o $@ $(FLTK) -lm

# Command to generate all PNG artifacts into output/
run_examples: all
	cd $(OUTDIR) && ../$(BINDIR)/algebraic_viz
	cd $(OUTDIR) && ../$(BINDIR)/cubic_viz
	cd $(OUTDIR) && ../$(BINDIR)/snowflake_viz
	cd $(OUTDIR) && ../$(BINDIR)/rotation_drift_viz
	@echo "Static visualizations updated in $(OUTDIR)/"

test: $(BINDIR)/tests
	./$(BINDIR)/tests

clean:
	rm -rf $(BINDIR)/* $(OUTDIR)/*.png

.PHONY: all clean run_examples test
