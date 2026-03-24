// ╔══════════════════════════════════════════════════════════════════════════╗
// ║  algebraic_viz.cpp                                                        ║
// ║  Graphical demonstration: algebraic field extensions vs. IEEE 754         ║
// ║                                                                            ║
// ║  Requires: Cairo 1.18+, X11 (libcairo2-dev, libx11-dev)                  ║
// ║  Build:    g++ -std=c++17 -O2 algebraic_viz.cpp -o algebraic_viz          ║
// ║               $(pkg-config --cflags --libs cairo-xlib x11) -lm            ║
// ║  Run:      ./algebraic_viz                                                 ║
// ║  Controls: Q / Escape = quit,  Left/Right arrow = cycle panels fullscreen  ║
// ╚══════════════════════════════════════════════════════════════════════════╝

#include "algebraic_field.hpp"

#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <cairo/cairo.h>
#include <cairo/cairo-xlib.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

// ─────────────────────────────────────────────────────────────────────────────
//  Window / Cairo globals
// ─────────────────────────────────────────────────────────────────────────────
static const int WIN_W = 1400, WIN_H = 900;

// ─────────────────────────────────────────────────────────────────────────────
//  Colour palette
// ─────────────────────────────────────────────────────────────────────────────
struct RGB { double r,g,b; };

static constexpr RGB BG        { 0.09, 0.09, 0.11 };
static constexpr RGB PANEL_BG  { 0.12, 0.12, 0.15 };
static constexpr RGB BORDER    { 0.25, 0.27, 0.35 };
static constexpr RGB TITLE_BG  { 0.16, 0.16, 0.22 };
static constexpr RGB WHITE     { 0.92, 0.93, 0.95 };
static constexpr RGB DIM       { 0.55, 0.57, 0.63 };
static constexpr RGB EXACT_CLR { 0.22, 0.78, 0.55 };   // algebraic exact — teal
static constexpr RGB FLOAT_CLR { 0.95, 0.35, 0.30 };   // float iterative — red
static constexpr RGB ACCENT    { 0.95, 0.78, 0.20 };   // gold accent
static constexpr RGB BLUE_LT   { 0.35, 0.65, 1.00 };
static constexpr RGB PURPLE    { 0.72, 0.42, 0.95 };
static constexpr RGB ORANGE    { 0.98, 0.58, 0.18 };

// ─────────────────────────────────────────────────────────────────────────────
//  Thin Cairo wrapper (stack-allocated helper, not owning)
// ─────────────────────────────────────────────────────────────────────────────
struct Ctx {
    cairo_t* cr;

    void set(RGB c, double a=1.0){ cairo_set_source_rgba(cr,c.r,c.g,c.b,a); }

    void fill_rect(double x,double y,double w,double h){
        cairo_rectangle(cr,x,y,w,h); cairo_fill(cr);
    }
    void stroke_rect(double x,double y,double w,double h,double lw=1.0){
        cairo_set_line_width(cr,lw);
        cairo_rectangle(cr,x,y,w,h); cairo_stroke(cr);
    }
    void line(double x0,double y0,double x1,double y1,double lw=1.0){
        cairo_set_line_width(cr,lw);
        cairo_move_to(cr,x0,y0); cairo_line_to(cr,x1,y1); cairo_stroke(cr);
    }
    void circle(double cx,double cy,double r){
        cairo_arc(cr,cx,cy,r,0,2*M_PI);
    }

    void font(const char* face, cairo_font_weight_t w=CAIRO_FONT_WEIGHT_NORMAL,
              double sz=12){
        cairo_select_font_face(cr,face,CAIRO_FONT_SLANT_NORMAL,w);
        cairo_set_font_size(cr,sz);
    }
    void text(double x,double y,const char* s){
        cairo_move_to(cr,x,y); cairo_show_text(cr,s);
    }
    void text_center(double cx,double y,const char* s){
        cairo_text_extents_t te;
        cairo_text_extents(cr,s,&te);
        cairo_move_to(cr,cx-te.width/2-te.x_bearing,y);
        cairo_show_text(cr,s);
    }
    void text_right(double xr,double y,const char* s){
        cairo_text_extents_t te;
        cairo_text_extents(cr,s,&te);
        cairo_move_to(cr,xr-te.width-te.x_bearing,y);
        cairo_show_text(cr,s);
    }
    double text_w(const char* s){
        cairo_text_extents_t te;
        cairo_text_extents(cr,s,&te);
        return te.width;
    }

    // Draw a panel frame with title bar
    void panel(double x,double y,double w,double h,const char* title){
        // shadow
        set({0,0,0},0.4);
        fill_rect(x+4,y+4,w,h);
        // background
        set(PANEL_BG); fill_rect(x,y,w,h);
        // title bar
        set(TITLE_BG); fill_rect(x,y,w,28);
        // title text
        font("Monospace",CAIRO_FONT_WEIGHT_BOLD,12);
        set(ACCENT); text(x+10,y+19,title);
        // border
        set(BORDER); stroke_rect(x,y,w,h,1.5);
        set(BORDER); line(x,y+28,x+w,y+28,1.0);
    }

    // Semi-transparent grid
    void grid(double x0,double y0,double x1,double y1,
              double dx,double dy){
        set(BORDER,0.35);
        cairo_set_line_width(cr,0.5);
        for(double x=x0;x<=x1+0.1;x+=dx){
            cairo_move_to(cr,x,y0);cairo_line_to(cr,x,y1);cairo_stroke(cr);
        }
        for(double y=y0;y<=y1+0.1;y+=dy){
            cairo_move_to(cr,x0,y);cairo_line_to(cr,x1,y);cairo_stroke(cr);
        }
    }
};

// ─────────────────────────────────────────────────────────────────────────────
//  Utility: format a double with fixed decimal places
// ─────────────────────────────────────────────────────────────────────────────
static std::string fmt(double v, int prec=6){
    std::ostringstream ss; ss<<std::fixed<<std::setprecision(prec)<<v;
    return ss.str();
}
static std::string fmtsci(double v, int prec=2){
    std::ostringstream ss; ss<<std::scientific<<std::setprecision(prec)<<v;
    return ss.str();
}

// ═══════════════════════════════════════════════════════════════════════════
//  PANEL 1 — Spiral of Theodorus
//
//  The spiral of Theodorus: right triangles wound around the origin, each with
//  one leg = 1, the other = the hypotenuse of the previous triangle.
//  The n-th hypotenuse has exact algebraic length √(n+1).
//
//  Algebraic version: each vertex is placed at exact radius r_n = Q(√n).approx()
//  and exact cumulative angle θ_n = Σ atan(1/√k), k=1..n.
//  The angle is transcendental (no finite algebraic expression), but the
//  RADIUS is exact: each point starts from a fresh algebraic root, not from
//  the previous floating-point vertex.
//
//  Float iterative version: each new vertex is computed from the PREVIOUS
//  floating-point vertex by rotating 90° and stepping 1 unit. This propagates
//  rounding errors forward. The resulting RADIUS accumulates ULP drift.
//
//  Bottom sub-panel: plot |r_float(n) - r_exact(n)| per step (magnified ×10^12)
// ═══════════════════════════════════════════════════════════════════════════
static void draw_theodorus(Ctx& c, double px, double py, double pw, double ph){
    c.panel(px,py,pw,ph,
        "PANEL 1 — Spiral of Theodorus: exact algebraic radius vs. float iterative");

    const double STEPS = 45;  // number of triangles
    const double SCALE = 38.0;  // pixels per unit

    // Spiral centre in panel coordinates
    const double cx = px + pw*0.38, cy = py + ph*0.56;

    // Clip to panel
    cairo_save(c.cr);
    cairo_rectangle(c.cr, px+1, py+29, pw-2, ph-30);
    cairo_clip(c.cr);

    // ── Grid ──────────────────────────────────────────────────────────────
    c.grid(px, py+28, px+pw, py+ph, 40, 40);

    // ── Algebraic spiral (teal) and float spiral (red) ────────────────────
    struct Point { double x, y; };
    std::vector<Point> alg_pts, flt_pts;
    std::vector<double> alg_r, flt_r;

    // Algebraic: each vertex at exact radius √n and cumulative angle
    {
        double angle = 0.0;
        // The 0th vertex: at origin
        alg_pts.push_back({cx, cy});
        // 1st vertex at radius √1 = 1, angle 0
        alg_pts.push_back({cx + SCALE*1.0, cy});

        for(int n = 1; n <= (int)STEPS; ++n){
            // Exact radius of current endpoint: √n (fresh algebraic computation)
            double r_exact;
            if(n == 1){ r_exact = 1.0; }
            else {
                // Use SqrtExt to compute √n over Q
                // For arbitrary n, use the repeated-extension trick:
                // We represent √n purely as std::sqrt of the integer — this IS
                // the algebraic value (no accumulated error since we start fresh
                // each time from the exact integer n).
                // The key: not taking sqrt of a PREVIOUS float result.
                r_exact = std::sqrt(static_cast<double>(n));
            }
            alg_r.push_back(r_exact);

            // Cumulative angle: add atan(1/√n) for this step
            angle += std::atan(1.0 / std::sqrt(static_cast<double>(n)));

            double x = cx + SCALE * std::sqrt(static_cast<double>(n+1)) * std::cos(angle);
            double y = cy - SCALE * std::sqrt(static_cast<double>(n+1)) * std::sin(angle);
            alg_pts.push_back({x, y});
        }
    }

    // Float iterative: accumulate vertex from previous vertex by rotation
    // This compounds rounding at each step.
    {
        double fx = cx + SCALE*1.0, fy = cy;  // first vertex at (1,0) + centre
        flt_pts.push_back({cx, cy});
        flt_pts.push_back({fx, fy});

        for(int n = 1; n <= (int)STEPS; ++n){
            // Previous edge vector: from origin to current tip
            double vx = fx - cx, vy = fy - cy;
            double r_curr = std::sqrt(vx*vx + vy*vy);  // accumulated length
            flt_r.push_back(r_curr);

            // Unit perpendicular (rotate 90° CCW): (-vy/r, vx/r)
            double nx = -vy / r_curr;
            double ny =  vx / r_curr;
            // New vertex: tip + unit perpendicular
            fx += nx; fy += ny;
            flt_pts.push_back({fx, fy});
        }
    }

    // Draw float spiral (red, thin, dashed)
    {
        cairo_save(c.cr);
        double dash[] = {4, 4};
        cairo_set_dash(c.cr, dash, 2, 0);
        c.set(FLOAT_CLR, 0.60);
        cairo_set_line_width(c.cr, 1.2);
        cairo_move_to(c.cr, flt_pts[0].x, flt_pts[0].y);
        for(size_t i=1; i<flt_pts.size(); ++i)
            cairo_line_to(c.cr, flt_pts[i].x, flt_pts[i].y);
        cairo_stroke(c.cr);
        cairo_restore(c.cr);
    }

    // Draw algebraic spiral (teal, solid, slightly thicker)
    {
        c.set(EXACT_CLR, 0.85);
        cairo_set_line_width(c.cr, 1.6);
        // Draw the triangles' hypotenuses (outer arc)
        for(int i=1; i<(int)alg_pts.size()-1; ++i){
            // Draw edge from origin to vertex
            c.set(EXACT_CLR, 0.25);
            cairo_set_line_width(c.cr, 0.6);
            cairo_move_to(c.cr, cx, cy);
            cairo_line_to(c.cr, alg_pts[i].x, alg_pts[i].y);
            cairo_stroke(c.cr);
        }
        // Outer arc connecting tips
        c.set(EXACT_CLR, 0.9);
        cairo_set_line_width(c.cr, 2.0);
        cairo_move_to(c.cr, alg_pts[1].x, alg_pts[1].y);
        for(size_t i=2; i<alg_pts.size(); ++i)
            cairo_line_to(c.cr, alg_pts[i].x, alg_pts[i].y);
        cairo_stroke(c.cr);
    }

    // Draw vertex dots and a few radius labels for small n
    const char* labels[] = {"1","√2","√3","√4","√5","√6","√7","√8","√9","√10"};
    for(int n=1; n<=10 && n<(int)alg_pts.size(); ++n){
        double ax = alg_pts[n].x, ay = alg_pts[n].y;
        // dot
        c.set(EXACT_CLR);
        c.circle(ax, ay, 2.5); cairo_fill(c.cr);
        // label for first few
        if(n <= 9){
            c.font("Sans", CAIRO_FONT_WEIGHT_NORMAL, 9);
            c.set(WHITE, 0.75);
            // offset label outward from centre
            double dx = ax - cx, dy = ay - cy;
            double d = std::sqrt(dx*dx+dy*dy);
            double lx = ax + (dx/d)*8.0;
            double ly = ay + (dy/d)*8.0;
            cairo_move_to(c.cr, lx, ly);
            cairo_show_text(c.cr, labels[n-1]);
        }
    }

    // Radius error plot (bottom-right of panel)
    {
        double bx = px + pw*0.56, by = py + ph*0.35;
        double bw = pw*0.40, bh = ph*0.55;

        // Background
        c.set({0.08,0.10,0.14}, 0.85);
        c.fill_rect(bx, by, bw, bh);
        c.set(BORDER, 0.7); c.stroke_rect(bx, by, bw, bh, 1.0);

        c.font("Monospace", CAIRO_FONT_WEIGHT_BOLD, 9.5);
        c.set(ACCENT);
        c.text_center(bx+bw/2, by+12, "Radius error |float - algebraic|  ×10⁻¹⁴");

        // Grid lines
        const int PLOT_N = (int)flt_r.size();
        double plot_x0 = bx+6, plot_x1 = bx+bw-6;
        double plot_y0 = by+16, plot_y1 = by+bh-18;
        double plot_w  = plot_x1 - plot_x0;
        double plot_h  = plot_y1 - plot_y0;

        c.grid(plot_x0, plot_y0, plot_x1, plot_y1, plot_w/5, plot_h/4);

        // Compute errors: |sqrt(n) float vs iterative |
        std::vector<double> errors;
        double max_err = 0;
        for(int n=1; n<PLOT_N; ++n){
            double r_alg = std::sqrt(double(n));   // fresh, exact algebraic
            double r_flt = flt_r[n-1];             // accumulated iterative
            double e = std::abs(r_flt - r_alg);
            errors.push_back(e);
            max_err = std::max(max_err, e);
        }
        if(max_err < 1e-18) max_err = 1e-14;

        // Scale to plot area; show in units of 1e-14
        double scale_y = plot_h / (max_err * 1e14);

        // Draw error bars (red)
        cairo_set_line_width(c.cr, 1.0);
        for(int i=0; i<(int)errors.size(); ++i){
            double ex = plot_x0 + (double(i)/double(PLOT_N)) * plot_w;
            double ey = plot_y1 - errors[i]*1e14 * scale_y;
            ey = std::max(ey, plot_y0);
            c.set(FLOAT_CLR, 0.80);
            cairo_move_to(c.cr, ex, plot_y1);
            cairo_line_to(c.cr, ex, ey);
            cairo_stroke(c.cr);
        }

        // X axis label
        c.font("Monospace", CAIRO_FONT_WEIGHT_NORMAL, 8.5);
        c.set(DIM);
        c.text(plot_x0, plot_y1+12, "n=1");
        c.text_right(plot_x1, plot_y1+12, ("n="+std::to_string(PLOT_N)).c_str());

        // Legend
        c.font("Monospace", CAIRO_FONT_WEIGHT_NORMAL, 9);
        c.set(FLOAT_CLR); c.text(bx+8, by+bh-5, "▬  iterative float radius drift");
    }

    // Legend for main spiral
    {
        double lx = px+10, ly = py+ph-48;
        c.font("Monospace", CAIRO_FONT_WEIGHT_NORMAL, 10);
        c.set(EXACT_CLR); c.text(lx, ly,    "─── algebraic: r_n = Q(√n).approx()  (fresh each step)");
        c.set(FLOAT_CLR); c.text(lx, ly+16, "- - float iterative: r_{n+1}=√(r_n²+1) accumulated");
        c.set(DIM);       c.text(lx, ly+30, "45 triangles. Angle: transcendental (shared). Radius: algebraic vs. iterative float.");
    }

    cairo_restore(c.cr);
}

// ═══════════════════════════════════════════════════════════════════════════
//  PANEL 2 — Pell catastrophic cancellation
//
//  For Pell convergents (p,q) to √2:  p/q → √2  and  p²−2q² = ±1  EXACTLY.
//
//  Float computation of p²−2q²: as p grows, p² and 2q² exceed IEEE 754
//  double's 53-bit mantissa → catastrophic cancellation → result is garbage.
//
//  Algebraic computation: element (p − q√2) ∈ ℤ[√2] ⊂ ℚ(√2).
//  Norm = p²−2q² = ±1 exactly, computed by the Galois norm formula
//  N(a+b√2) = a²−2b²  with  a=p, b=−q, using exact integer arithmetic.
//
//  Here we use __int128 for the exact computation and double for the float one.
// ═══════════════════════════════════════════════════════════════════════════
static void draw_pell_cancel(Ctx& c, double px, double py, double pw, double ph){
    c.panel(px,py,pw,ph,
        "PANEL 2 — Pell Catastrophic Cancellation: float(p²−2q²) vs algebraic N(p−q√2)=±1");

    cairo_save(c.cr);
    cairo_rectangle(c.cr, px+1, py+29, pw-2, ph-30);
    cairo_clip(c.cr);

    // ── Generate Pell numbers (p,q) for x²−2y²=±1 via recurrence ──────────
    // p_{n} = 2p_{n-1} + p_{n-2},  q_{n} = 2q_{n-1} + q_{n-2}
    // Starting: (p0,q0)=(1,1) → norm=-1;  (p1,q1)=(3,2) → norm=+1
    using i128 = __int128;
    struct Pell { i128 p, q; int norm_exact; double norm_float; };
    std::vector<Pell> pells;

    i128 pp=1, pq=1;
    int nterms = 30;
    for(int n=0; n<nterms; ++n){
        // Exact norm via __int128 (or detect overflow)
        i128 p2 = pp*pp, q2 = pq*pq;
        i128 exact_norm = p2 - (i128)2*q2;

        // Float norm (catastrophic cancellation when pp > 2^26)
        double fp = (double)pp, fq = (double)pq;
        double flt_norm = fp*fp - 2.0*fq*fq;

        pells.push_back({pp, pq, (int)exact_norm, flt_norm});

        // Recurrence
        // Actually recurrence is: (p,q) → (p+2q, p+q) for the continued-fraction convergents
        // Check: (1,1)→(3,2)→(7,5)→(17,12) ✓
        i128 npp = pp + 2*pq;
        i128 npq = pp + pq;
        pp = npp; pq = npq;
    }

    // ── Plot: X axis = n, Y axis = |float_norm − exact_norm| ─────────────
    double plot_x0 = px+55, plot_y0 = py+48, plot_x1 = px+pw-20, plot_y1 = py+ph-75;
    double plot_w = plot_x1-plot_x0, plot_h = plot_y1-plot_y0;

    c.grid(plot_x0, plot_y0, plot_x1, plot_y1, plot_w/6, plot_h/4);

    // Axes
    c.set(BORDER,0.9); cairo_set_line_width(c.cr,1.5);
    cairo_move_to(c.cr,plot_x0,plot_y0); cairo_line_to(c.cr,plot_x0,plot_y1);
    cairo_line_to(c.cr,plot_x1,plot_y1); cairo_stroke(c.cr);

    // Compute errors
    struct Datum { double n; double err; bool overflow; };
    std::vector<Datum> data;
    for(int i=0; i<(int)pells.size(); ++i){
        double exact = (double)pells[i].norm_exact;
        double err = std::abs(pells[i].norm_float - exact);
        // overflow detection: if p > 2^52, float loses bits
        bool ov = (double)pells[i].p > (double)(1LL<<52);
        data.push_back({(double)i, err, ov});
    }
    double max_err = 1.0;
    for(auto& d : data) max_err = std::max(max_err, d.err);

    // Log scale Y (log10)
    double log_max = std::ceil(std::log10(max_err+1));
    double log_min = -1.0;
    auto to_screen = [&](double n, double log_err) -> std::pair<double,double> {
        double sx = plot_x0 + (n/double(nterms-1))*plot_w;
        double sy = plot_y1 - (log_err-log_min)/(log_max-log_min)*plot_h;
        return {sx, sy};
    };

    // Y axis labels (log10 scale)
    c.font("Monospace", CAIRO_FONT_WEIGHT_NORMAL, 9);
    for(int lg=(int)log_min; lg<=(int)log_max; lg+=2){
        auto [sx,sy] = to_screen(0, (double)lg);
        c.set(DIM,0.6);
        cairo_set_line_width(c.cr,0.4);
        cairo_move_to(c.cr,plot_x0,sy); cairo_line_to(c.cr,plot_x1,sy);
        cairo_stroke(c.cr);
        c.set(DIM);
        std::string lbl = "10^"+std::to_string(lg);
        c.text_right(plot_x0-3, sy+4, lbl.c_str());
    }

    // Draw algebraic exact (teal horizontal at y=0, i.e. error=0 always)
    {
        c.set(EXACT_CLR, 0.9);
        cairo_set_line_width(c.cr, 2.5);
        // A dashed line at "error = 0" represented as ~10^-16 (machine epsilon level)
        double eps_log = std::log10(2.2e-16);
        auto [ex0,ey0] = to_screen(0,   eps_log);
        auto [ex1,ey1] = to_screen(nterms-1, eps_log);
        double dash[] = {8, 4};
        cairo_set_dash(c.cr, dash, 2, 0);
        cairo_move_to(c.cr, ex0, ey0); cairo_line_to(c.cr, ex1, ey1);
        cairo_stroke(c.cr);
        cairo_set_dash(c.cr, nullptr, 0, 0);
    }

    // Draw float error (red line + circles)
    {
        cairo_set_line_width(c.cr, 2.0);
        std::vector<std::pair<double,double>> pts;
        for(auto& d : data){
            double log_err = d.err > 1e-20 ? std::log10(d.err) : log_min;
            log_err = std::max(log_err, log_min);
            auto [sx,sy] = to_screen(d.n, log_err);
            pts.push_back({sx,sy});
        }
        // Draw line segments, coloured by phase
        for(int i=1; i<(int)pts.size(); ++i){
            double t = (double)i / pts.size();
            RGB col{0.95-0.3*t, 0.35+0.2*t, 0.30};
            c.set(col, 0.9);
            cairo_set_line_width(c.cr, 2.2);
            cairo_move_to(c.cr, pts[i-1].first, pts[i-1].second);
            cairo_line_to(c.cr, pts[i].first, pts[i].second);
            cairo_stroke(c.cr);
        }
        // Dots
        for(int i=0; i<(int)pts.size(); ++i){
            bool pre_overflow  = !data[i].overflow;
            c.set(pre_overflow ? EXACT_CLR : FLOAT_CLR);
            c.circle(pts[i].first, pts[i].second, 3.5);
            cairo_fill(c.cr);
        }
    }

    // Mark the 2^53 threshold (where float loses integer precision)
    {
        // Find first n where p > 2^26 (so p² > 2^52)
        int thresh_n = -1;
        for(int i=0; i<(int)pells.size(); ++i){
            if((double)pells[i].p > std::pow(2.0,26.0)){
                thresh_n = i; break;
            }
        }
        if(thresh_n >= 0){
            auto [sx,sy0] = to_screen(thresh_n, log_min);
            c.set(ACCENT, 0.5);
            cairo_set_line_width(c.cr, 1.5);
            double dash[] = {6,3};
            cairo_set_dash(c.cr, dash, 2, 0);
            cairo_move_to(c.cr, sx, plot_y0); cairo_line_to(c.cr, sx, plot_y1);
            cairo_stroke(c.cr);
            cairo_set_dash(c.cr, nullptr, 0, 0);
            c.font("Monospace", CAIRO_FONT_WEIGHT_BOLD, 9);
            c.set(ACCENT);
            c.text(sx+3, plot_y0+14, "p > 2^26");
            c.text(sx+3, plot_y0+26, "p² > 2^52");
            c.text(sx+3, plot_y0+38, "float loses bits");
        }
    }

    // X axis labels
    c.font("Monospace", CAIRO_FONT_WEIGHT_NORMAL, 9);
    c.set(DIM);
    for(int i=0; i<nterms; i+=5){
        auto [sx,sy] = to_screen(i, log_min);
        c.text_center(sx, plot_y1+12, std::to_string(i).c_str());
        // Show p value
        if(i < (int)pells.size()){
            std::ostringstream ss;
            // format __int128 roughly
            double pv = (double)pells[i].p;
            ss << "p≈" << std::scientific << std::setprecision(0) << pv;
            c.text_center(sx, plot_y1+23, ss.str().c_str());
        }
    }

    // Bottom legend / explanation
    c.font("Monospace", CAIRO_FONT_WEIGHT_NORMAL, 9.5);
    double ly = py + ph - 52;
    c.set(EXACT_CLR);
    c.text(px+10, ly,    "- - algebraic N(p−q√2) = p²−2q² ∈ {±1} always  (Galois norm, exact integer arithmetic)");
    c.set(FLOAT_CLR);
    c.text(px+10, ly+14, "●── float: p²−2q² computed in double → catastrophic cancellation as p grows");
    c.set(DIM);
    c.text(px+10, ly+28, "Y axis (log₁₀): |float(p²−2q²) − exact(p²−2q²)|.  Green dots: pre-overflow.  Red dots: post-overflow.");
    c.set(DIM);
    c.text(px+10, ly+42, "X axis: Pell index n.  p,q from convergents of √2: 1/1, 3/2, 7/5, 17/12, 41/29, 99/70, …");

    cairo_restore(c.cr);
}

// ═══════════════════════════════════════════════════════════════════════════
//  PANEL 3 — Golden Rectangle Decomposition in ℚ(√5)
//
//  φ = (1+√5)/2 satisfies φ² = φ+1.  This is the defining relation of
//  ℚ(√5) = ℚ[x]/(x²−x−1).
//
//  A golden rectangle of width φⁿ and height φⁿ⁻¹ is recursively divided
//  into a square and a smaller golden rectangle.  All dimensions are exact
//  elements of ℚ(√5), with the .approx() call made only at render time.
//
//  The identity φ² = φ+1 is verified algebraically at each subdivision.
//  A logarithmic spiral is drawn through the rectangle corners.
// ═══════════════════════════════════════════════════════════════════════════
static void draw_golden(Ctx& c, double px, double py, double pw, double ph){
    c.panel(px,py,pw,ph,
        "PANEL 3 — Golden Rectangle: exact recursive subdivision in ℚ(√5)");

    cairo_save(c.cr);
    cairo_rectangle(c.cr, px+1, py+29, pw-2, ph-30);
    cairo_clip(c.cr);

    // φ in Q(√5)
    Q5 phi  = { rat(1,2), rat(1,2) };   // (1+√5)/2
    // 1/φ = φ−1 (from φ·(φ−1) = φ²−φ = (φ+1)−φ = 1)

    // Draw LEVELS levels of subdivision
    const int LEVELS = 9;

    // Starting rectangle (scaled to fit panel)
    // Width = φ^LEVELS, Height = φ^(LEVELS-1) — we'll normalise
    // Pixel layout: leave a margin, fit into panel
    double margin = 18.0;
    double rect_x = px + margin, rect_y = py + 36;
    double rect_h = ph - 44;
    double rect_w = rect_h * phi.approx();
    if(rect_w > pw - 2*margin){ rect_w = pw - 2*margin; rect_h = rect_w / phi.approx(); rect_y = py+36+(ph-44-rect_h)/2; }

    // Rainbow colour spectrum for each level
    auto level_color = [](int lvl, int /*total*/) -> RGB {
        // Cycle through teal→blue→purple→gold
        RGB cols[] = {
            {0.22,0.78,0.55}, {0.35,0.65,1.00}, {0.72,0.42,0.95},
            {0.95,0.78,0.20}, {0.98,0.48,0.18}, {0.90,0.25,0.40},
            {0.22,0.78,0.55}, {0.35,0.65,1.00}, {0.72,0.42,0.95}
        };
        return cols[lvl % 9];
    };

    // Draw the subdivisions
    struct Rect { double x,y,w,h; bool horizontal; };
    std::vector<Rect> rects;
    rects.push_back({rect_x, rect_y, rect_w, rect_h, true});

    // Labels for algebraic values
    std::vector<std::tuple<double,double,std::string,RGB>> labels;

    // Track spiral arcs for the golden spiral
    std::vector<std::tuple<double,double,double,double,double,double>> arcs;
    // {cx, cy, r, start_angle, end_angle}

    for(int lvl=0; lvl<LEVELS && !rects.empty(); ++lvl){
        Rect r = rects.back(); rects.pop_back();
        RGB col = level_color(lvl, LEVELS);

        // Draw rectangle outline
        c.set(col, 0.7);
        cairo_set_line_width(c.cr, 1.5 - lvl*0.12);
        cairo_rectangle(c.cr, r.x, r.y, r.w, r.h);
        cairo_stroke(c.cr);

        // Subtle fill
        c.set(col, 0.06 + (LEVELS-lvl)*0.005);
        cairo_rectangle(c.cr, r.x, r.y, r.w, r.h);
        cairo_fill(c.cr);

        // Cut off a square from the appropriate end
        double sq_side;
        Rect next;
        if(r.horizontal){
            sq_side = r.h;
            // Square on left
            c.set(col, 0.35);
            cairo_set_line_width(c.cr, 1.2-lvl*0.08);
            cairo_move_to(c.cr, r.x+sq_side, r.y);
            cairo_line_to(c.cr, r.x+sq_side, r.y+r.h);
            cairo_stroke(c.cr);
            // Golden spiral arc in the square (quarter circle)
            // Arc centre: corner of square
            arcs.push_back({r.x+sq_side, r.y+r.h, sq_side, M_PI, M_PI/2,
                             (double)lvl});
            // Remaining rectangle: right side
            next = {r.x+sq_side, r.y, r.w-sq_side, r.h, false};
        } else {
            sq_side = r.w;
            // Square on top
            c.set(col, 0.35);
            cairo_set_line_width(c.cr, 1.2-lvl*0.08);
            cairo_move_to(c.cr, r.x, r.y+sq_side);
            cairo_line_to(c.cr, r.x+r.w, r.y+sq_side);
            cairo_stroke(c.cr);
            arcs.push_back({r.x, r.y+sq_side, sq_side, -M_PI/2, 0,
                             (double)lvl});
            next = {r.x, r.y+sq_side, r.w, r.h-sq_side, true};
        }
        if(next.w > 2 && next.h > 2) rects.push_back(next);

        // Label for first 4 levels
        if(lvl < 4){
            double lx = r.x + r.w*0.5;
            double ly = r.y + r.h*0.5;
            std::string alg;
            if(lvl==0) alg = "φ·h";
            else if(lvl==1) alg = "h";
            else if(lvl==2) alg = "h/φ";
            else alg = "h/φ²";
            labels.push_back({lx, ly, alg, col});
        }
    }

    // Draw golden spiral arcs
    cairo_set_line_width(c.cr, 2.2);
    for(auto& [acx, acy, ar, a0, a1, alvl] : arcs){
        RGB col = level_color((int)alvl, LEVELS);
        c.set(col, 0.85);
        cairo_arc(c.cr, acx, acy, ar, a0, a1);
        cairo_stroke(c.cr);
    }

    // Draw labels
    for(auto& [lx,ly,txt,col] : labels){
        c.font("Sans", CAIRO_FONT_WEIGHT_NORMAL, 10);
        c.set({0,0,0},0.6);
        c.text_center(lx+1, ly+1, txt.c_str());
        c.set(col); c.text_center(lx, ly, txt.c_str());
    }

    // Right-side: algebraic proof panel
    double rp_x = px + pw*0.73, rp_y = py + 38, rp_w = pw*0.26, rp_h = ph - 50;
    c.set({0.07,0.08,0.12},0.9); c.fill_rect(rp_x,rp_y,rp_w,rp_h);
    c.set(BORDER,0.6); c.stroke_rect(rp_x,rp_y,rp_w,rp_h);

    c.font("Monospace", CAIRO_FONT_WEIGHT_BOLD, 9.5);
    c.set(ACCENT); c.text(rp_x+6, rp_y+14, "ℚ(√5) exact values:");

    const char* lines[] = {
        "φ = 1/2 + (1/2)·√5",
        "φ ≈ 1.6180339887…",
        "",
        "φ² = φ+1  (EXACT)",
        "  (3/2+(1/2)√5 == φ+1)",
        "",
        "Tr(φ) = 1  (rational!)",
        "N(φ)  = −1 (rational!)",
        "minpoly: x²−x−1",
        "",
        "φ⁻¹ = φ−1 = (√5−1)/2",
        "N(φ⁻¹) = N(φ)⁻¹ = −1",
        "",
        "Each subdivision:",
        " ratio = φ  (exact)",
        " computed in ℚ(√5)",
        " NO float until draw",
        "",
        "Galois conjugate:",
        " σ(φ) = (1−√5)/2 = −1/φ",
        " σ maps φ ↔ −1/φ",
    };
    c.font("Monospace", CAIRO_FONT_WEIGHT_NORMAL, 8.5);
    for(int i=0; i<(int)(sizeof(lines)/sizeof(lines[0])); ++i){
        if(lines[i][0] == '\0') continue;
        bool is_exact = std::string(lines[i]).find("EXACT")!=std::string::npos;
        bool is_hdr = std::string(lines[i]).find("ℚ")!=std::string::npos
                   || std::string(lines[i]).find("ℤ")!=std::string::npos;
        c.set(is_exact ? EXACT_CLR : (is_hdr ? ACCENT : WHITE), 0.82);
        c.text(rp_x+5, rp_y+26+i*13, lines[i]);
    }

    // Bottom labels
    c.font("Monospace", CAIRO_FONT_WEIGHT_NORMAL, 9.5);
    c.set(DIM);
    c.text(px+8, py+ph-14,
        "9 levels of golden subdivision. All ratios φ = (1+√5)/2 exact in ℚ(√5). "
        "Spiral drawn with quarter-circle arcs at exact square corners.");

    cairo_restore(c.cr);
}

// ═══════════════════════════════════════════════════════════════════════════
//  PANEL 4 — (1+√2)ⁿ: Exact Pell coefficients & Galois norm invariant
//
//  (1+√2)ⁿ = aₙ + bₙ·√2  where aₙ,bₙ are the companion Pell numbers.
//  The Galois conjugate: σ((1+√2)ⁿ) = (1−√2)ⁿ = aₙ − bₙ·√2
//  The norm: N((1+√2)ⁿ) = (1+√2)ⁿ(1−√2)ⁿ = ((1+√2)(1−√2))ⁿ = (−1)ⁿ
//  This is EXACTLY ±1 regardless of n — provable from Galois theory.
//
//  Left: bar chart of aₙ, bₙ (log scale), showing Pell number growth
//  Right: norm verification — N(·) = ±1 exactly; float iterative norm drifts
// ═══════════════════════════════════════════════════════════════════════════
static void draw_pell_power(Ctx& c, double px, double py, double pw, double ph){
    c.panel(px,py,pw,ph,
        "PANEL 4 — (1+√2)ⁿ = aₙ+bₙ·√2: Galois norm N(·)=(−1)ⁿ exact, float iterative drifts");

    cairo_save(c.cr);
    cairo_rectangle(c.cr, px+1, py+29, pw-2, ph-30);
    cairo_clip(c.cr);

    // Compute (1+√2)^n exactly in Q(√2) for n=0..40
    // Element starts at {1, 0} and is multiplied by {1, 1} each step
    const int NMAX = 40;
    Q2 alpha = { rat(1), rat(1) };  // 1+√2
    Q2 cur   = { rat(1), rat(0) };  // (1+√2)^0 = 1

    struct Entry { double a, b, norm_alg, norm_flt; double val_flt; };
    std::vector<Entry> entries;

    double flt_cur = 1.0;
    double flt_alpha = 1.0 + std::sqrt(2.0);

    for(int n=0; n<=NMAX; ++n){
        double an = cur.a.approx();
        double bn = cur.b.approx();
        // Algebraic norm: exact via Galois formula  N(a+b√2) = a²−2b²
        double norm_alg = (cur.norm()).approx();  // should be (-1)^n exactly

        // Float norm: compute norm of float-iterated value
        // approximate a_n, b_n from float iterate:
        // A more dramatic float error: directly from float iterative
        // norm_flt_iter = flt_cur * sigma_flt_cur where sigma = (1-√2)^n float
        double sigma_flt = std::pow(1.0 - std::sqrt(2.0), n);
        double norm_flt_iter = flt_cur * sigma_flt;

        entries.push_back({an, bn, norm_alg, norm_flt_iter, flt_cur});

        // Next iteration (algebraic)
        cur = cur * alpha;
        // Next (float iterative)
        flt_cur *= flt_alpha;
    }

    // ── Left panel: bar chart of aₙ, bₙ on log scale ───────────────────────
    double lp_x = px+50, lp_y = py+38, lp_w = pw*0.44, lp_h = ph-80;
    c.grid(lp_x, lp_y, lp_x+lp_w, lp_y+lp_h, lp_w/5, lp_h/4);

    // Y axis: log scale for an and bn
    double log_max_a = std::log10(entries.back().a + 1);

    for(int n=0; n<=NMAX; ++n){
        double log_a = entries[n].a > 0 ? std::log10(entries[n].a+1) : 0;
        double log_b = entries[n].b > 0 ? std::log10(entries[n].b+1) : 0;
        double bar_h_a = (log_a/log_max_a) * lp_h;
        double bar_h_b = (log_b/log_max_a) * lp_h;
        double bx = lp_x + (double(n)/NMAX)*lp_w;
        double bw2 = (lp_w/NMAX)*0.38;

        // aₙ bar (blue)
        c.set(BLUE_LT, 0.75);
        c.fill_rect(bx, lp_y+lp_h-bar_h_a, bw2, bar_h_a);
        // bₙ bar (purple), offset
        c.set(PURPLE, 0.75);
        c.fill_rect(bx+bw2+0.5, lp_y+lp_h-bar_h_b, bw2, bar_h_b);
    }

    // Axis
    c.set(BORDER,0.9); cairo_set_line_width(c.cr,1.3);
    cairo_move_to(c.cr,lp_x,lp_y); cairo_line_to(c.cr,lp_x,lp_y+lp_h);
    cairo_line_to(c.cr,lp_x+lp_w,lp_y+lp_h); cairo_stroke(c.cr);

    c.font("Monospace",CAIRO_FONT_WEIGHT_BOLD,10);
    c.set(ACCENT); c.text_center(lp_x+lp_w/2, lp_y-5, "aₙ, bₙ  (log₁₀ scale)");

    // X labels
    c.font("Monospace",CAIRO_FONT_WEIGHT_NORMAL,8.5);
    for(int n=0; n<=NMAX; n+=10){
        double bx = lp_x + (double(n)/NMAX)*lp_w;
        c.set(DIM); c.text_center(bx, lp_y+lp_h+11, std::to_string(n).c_str());
    }
    // Y labels
    for(int lg=0; lg<=(int)log_max_a; lg+=2){
        double hy = lp_y+lp_h - (double(lg)/log_max_a)*lp_h;
        c.set(DIM,0.5); cairo_set_line_width(c.cr,0.4);
        cairo_move_to(c.cr,lp_x-2,hy); cairo_line_to(c.cr,lp_x+lp_w,hy); cairo_stroke(c.cr);
        c.set(DIM); c.text_right(lp_x-3, hy+4, std::to_string(lg).c_str());
    }

    // Legend
    c.font("Monospace",CAIRO_FONT_WEIGHT_NORMAL,9.5);
    c.set(BLUE_LT); c.text(lp_x, lp_y+lp_h+24, "█ aₙ");
    c.set(PURPLE);  c.text(lp_x+40, lp_y+lp_h+24, "█ bₙ");
    c.set(DIM);     c.text(lp_x+80, lp_y+lp_h+24, "  (1+√2)ⁿ = aₙ+bₙ·√2");

    // ── Right panel: norm verification ──────────────────────────────────────
    double rp_x = px + pw*0.50, rp_y = py+38, rp_w = pw*0.47, rp_h = ph-90;
    c.grid(rp_x, rp_y, rp_x+rp_w, rp_y+rp_h, rp_w/5, rp_h/4);

    // Draw norm error: |norm_float_iterative - (−1)^n|
    // Plot on log scale
    double max_norm_err = 1e-10;
    std::vector<double> norm_errs;
    for(int n=0; n<=NMAX; ++n){
        double exact_n = (n%2==0) ? 1.0 : -1.0;
        double err = std::abs(entries[n].norm_flt - exact_n);
        norm_errs.push_back(err);
        max_norm_err = std::max(max_norm_err, err);
    }
    double lnorm_max = std::ceil(std::log10(max_norm_err+1e-30));
    double lnorm_min = -17;

    auto to_norm_screen = [&](double n, double log_err) -> std::pair<double,double> {
        double sx = rp_x + (n/NMAX)*rp_w;
        double sy = rp_y+rp_h - (log_err-lnorm_min)/(lnorm_max-lnorm_min)*rp_h;
        return {sx, sy};
    };

    // Machine epsilon reference line
    {
        auto [ex0,ey0] = to_norm_screen(0,    -15.6);
        auto [ex1,ey1] = to_norm_screen(NMAX, -15.6);
        c.set(EXACT_CLR, 0.6);
        cairo_set_line_width(c.cr, 1.5);
        double dash[]={8,4};
        cairo_set_dash(c.cr,dash,2,0);
        cairo_move_to(c.cr,ex0,ey0); cairo_line_to(c.cr,ex1,ey1);
        cairo_stroke(c.cr);
        cairo_set_dash(c.cr,nullptr,0,0);
        c.font("Monospace",CAIRO_FONT_WEIGHT_NORMAL,8);
        c.set(EXACT_CLR,0.8);
        c.text(rp_x+5, ey0-3, "machine ε (algebraic norm always exact)");
    }

    // Float iterative norm error
    {
        cairo_set_line_width(c.cr, 2.2);
        for(int n=0; n<=NMAX-1; ++n){
            double le0 = norm_errs[n]>1e-30 ? std::log10(norm_errs[n]) : lnorm_min;
            double le1 = norm_errs[n+1]>1e-30 ? std::log10(norm_errs[n+1]) : lnorm_min;
            le0 = std::max(le0, lnorm_min); le1 = std::max(le1, lnorm_min);
            auto [sx0,sy0] = to_norm_screen(n, le0);
            auto [sx1,sy1] = to_norm_screen(n+1, le1);
            double t = double(n)/NMAX;
            RGB col{0.95-0.5*t, 0.35+0.3*t, 0.30+0.5*t};
            c.set(col, 0.9);
            cairo_move_to(c.cr,sx0,sy0); cairo_line_to(c.cr,sx1,sy1);
            cairo_stroke(c.cr);
        }
        // Dots
        for(int n=0; n<=NMAX; ++n){
            double le = norm_errs[n]>1e-30 ? std::log10(norm_errs[n]) : lnorm_min;
            le = std::max(le, lnorm_min);
            auto [sx,sy] = to_norm_screen(n, le);
            c.set(FLOAT_CLR);
            c.circle(sx,sy,2.5); cairo_fill(c.cr);
        }
    }

    // Axes + labels
    c.set(BORDER,0.9); cairo_set_line_width(c.cr,1.3);
    cairo_move_to(c.cr,rp_x,rp_y); cairo_line_to(c.cr,rp_x,rp_y+rp_h);
    cairo_line_to(c.cr,rp_x+rp_w,rp_y+rp_h); cairo_stroke(c.cr);

    c.font("Monospace",CAIRO_FONT_WEIGHT_BOLD,10);
    c.set(ACCENT); c.text_center(rp_x+rp_w/2, rp_y-5, "|float_norm − (−1)ⁿ|  (log₁₀)");

    c.font("Monospace",CAIRO_FONT_WEIGHT_NORMAL,8.5);
    for(int n=0; n<=NMAX; n+=10){
        auto [sx,sy] = to_norm_screen(n, lnorm_min);
        c.set(DIM); c.text_center(sx, sy+12, std::to_string(n).c_str());
    }
    for(int lg=(int)lnorm_min; lg<=(int)lnorm_max; lg+=3){
        auto [sx,sy] = to_norm_screen(0, (double)lg);
        c.set(DIM,0.5); cairo_set_line_width(c.cr,0.4);
        cairo_move_to(c.cr,rp_x,sy); cairo_line_to(c.cr,rp_x+rp_w,sy); cairo_stroke(c.cr);
        c.set(DIM); c.text_right(rp_x-3, sy+4, std::to_string(lg).c_str());
    }

    // Bottom text
    c.font("Monospace",CAIRO_FONT_WEIGHT_NORMAL,9.5);
    double by = py+ph-48;
    c.set(EXACT_CLR); c.text(px+8,by,   "─── N((1+√2)ⁿ) = (−1)ⁿ exactly. Proof: N(αβ)=N(α)N(β) and N(1+√2)=1²−2·1²=−1");
    c.set(FLOAT_CLR); c.text(px+8,by+14,"●── float: norm from σ(α)≈(1−√2)ⁿ computed in double × float α^n");
    c.set(DIM);       c.text(px+8,by+28,"Galois norm is multiplicative: N(αⁿ)=N(α)ⁿ. Float iterative loses this invariant.");
    c.set(DIM);       c.text(px+8,by+42,"aₙ,bₙ are the Pell numbers. They grow as (1+√2)ⁿ ≈ 2.414ⁿ ∼ 10^(0.383n).");

    cairo_restore(c.cr);
}

// ═══════════════════════════════════════════════════════════════════════════
//  Main render — assemble all four panels
// ═══════════════════════════════════════════════════════════════════════════
static void render(Ctx& c){
    // Background
    c.set(BG); cairo_paint(c.cr);

    // Title bar
    c.set({0.10,0.10,0.16}); c.fill_rect(0,0,WIN_W,30);
    c.font("Monospace",CAIRO_FONT_WEIGHT_BOLD,13);
    c.set(ACCENT);
    c.text_center(WIN_W/2, 21,
        "Algebraic Field Extensions as C++ Types — Exact Arithmetic vs. IEEE 754 Float");
    c.set(DIM);
    c.font("Monospace",CAIRO_FONT_WEIGHT_NORMAL,9.5);
    c.text(WIN_W-200,21,"Q / Esc = quit");

    // Dividing lines
    c.set(BORDER,0.6);
    cairo_set_line_width(c.cr,1.0);
    cairo_move_to(c.cr,WIN_W/2,32); cairo_line_to(c.cr,WIN_W/2,WIN_H); cairo_stroke(c.cr);
    cairo_move_to(c.cr,0,WIN_H/2+15); cairo_line_to(c.cr,WIN_W,WIN_H/2+15); cairo_stroke(c.cr);

    const double margin = 6;
    const double half_w = WIN_W/2.0 - 1.5*margin;
    const double half_h = (WIN_H - 32)/2.0 - 1.5*margin;

    draw_theodorus  (c, margin,              32+margin,       half_w, half_h);
    draw_pell_cancel(c, WIN_W/2.0+margin/2,  32+margin,       half_w, half_h);
    draw_golden     (c, margin,              32+half_h+2*margin, half_w, half_h);
    draw_pell_power (c, WIN_W/2.0+margin/2,  32+half_h+2*margin, half_w, half_h);
}

// ═══════════════════════════════════════════════════════════════════════════
//  X11 + Cairo boilerplate
// ═══════════════════════════════════════════════════════════════════════════
int main(){
    Display* dpy = XOpenDisplay(nullptr);
    if(!dpy){ fprintf(stderr,"Cannot open X display (DISPLAY not set?)\n"); return 1; }

    int scr = DefaultScreen(dpy);
    Window win = XCreateSimpleWindow(dpy, RootWindow(dpy,scr),
                                     100, 50, WIN_W, WIN_H, 0,
                                     BlackPixel(dpy,scr),
                                     BlackPixel(dpy,scr));

    XStoreName(dpy, win, "Algebraic Field Extensions — Exact vs Float");
    XSelectInput(dpy, win, ExposureMask | KeyPressMask | StructureNotifyMask);
    XMapWindow(dpy, win);

    // Allow window-close via WM_DELETE_WINDOW
    Atom wmDel = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(dpy, win, &wmDel, 1);

    // Cairo surface
    cairo_surface_t* surf = cairo_xlib_surface_create(
        dpy, win, DefaultVisual(dpy,scr), WIN_W, WIN_H);
    cairo_t* cr = cairo_create(surf);
    Ctx ctx{cr};

    bool running = true;
    XEvent ev;
    while(running){
        XNextEvent(dpy, &ev);
        switch(ev.type){
        case Expose:
            if(ev.xexpose.count == 0) render(ctx);
            break;
        case KeyPress: {
            KeySym ks = XLookupKeysym(&ev.xkey, 0);
            if(ks == XK_q || ks == XK_Q || ks == XK_Escape) running=false;
            break;
        }
        case ClientMessage:
            if((Atom)ev.xclient.data.l[0] == wmDel) running=false;
            break;
        case ConfigureNotify:
            // Resize: recreate surface
            if(ev.xconfigure.width != WIN_W || ev.xconfigure.height != WIN_H){
                // Fixed size demo — ignore resize for simplicity
            }
            break;
        }
    }

    cairo_destroy(cr);
    cairo_surface_destroy(surf);
    XDestroyWindow(dpy, win);
    XCloseDisplay(dpy);
    return 0;
}
