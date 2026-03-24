#include "../algebraic_field.hpp"
#include <cairo/cairo.h>
#include <algorithm>
#include <cmath>
#include <complex>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

static const int WIN_W = 1200, WIN_H = 800;

struct RGB { double r,g,b; };
static constexpr RGB BG        { 0.05, 0.05, 0.07 };
static constexpr RGB PANEL_BG  { 0.08, 0.08, 0.10 };
static constexpr RGB BORDER    { 0.20, 0.20, 0.25 };
static constexpr RGB WHITE     { 0.90, 0.90, 0.92 };
static constexpr RGB EXACT_CLR { 0.20, 0.80, 0.60 };
static constexpr RGB COMPLEX_CLR { 0.80, 0.40, 0.90 };
static constexpr RGB ACCENT    { 0.90, 0.70, 0.10 };

struct Ctx {
    cairo_t* cr;
    void set(RGB c, double a=1.0){ cairo_set_source_rgba(cr,c.r,c.g,c.b,a); }
    void fill_rect(double x,double y,double w,double h){
        cairo_rectangle(cr,x,y,w,h); cairo_fill(cr);
    }
    void line(double x0,double y0,double x1,double y1,double lw=1.0){
        cairo_set_line_width(cr,lw);
        cairo_move_to(cr,x0,y0); cairo_line_to(cr,x1,y1); cairo_stroke(cr);
    }
    void circle(double cx,double cy,double r){
        cairo_arc(cr,cx,cy,r,0,2*M_PI);
    }
    void text(double x,double y,const char* s, double sz=12){
        cairo_set_font_size(cr,sz);
        cairo_move_to(cr,x,y); cairo_show_text(cr,s);
    }
};

void draw_cubic_roots(Ctx& c, double px, double py, double pw, double ph) {
    c.set(PANEL_BG); c.fill_rect(px, py, pw, ph);
    c.set(BORDER); cairo_set_line_width(c.cr, 1.0); cairo_rectangle(c.cr, px, py, pw, ph); cairo_stroke(c.cr);

    double cx = px + pw/2, cy = py + ph/2;
    double scale = 150.0;

    c.set(BORDER, 0.3);
    for(int i=-2; i<=2; ++i) {
        c.line(cx + i*scale, py, cx + i*scale, py+ph, 0.5);
        c.line(px, cy + i*scale, px+pw, cy + i*scale, 0.5);
    }

    double rho = std::pow(2.0, 1.0/3.0);
    std::complex<double> r1(rho, 0);
    std::complex<double> r2(rho * -0.5, rho * std::sqrt(3.0)/2.0);
    std::complex<double> r3(rho * -0.5, rho * -std::sqrt(3.0)/2.0);

    c.set(WHITE, 0.1);
    c.circle(cx, cy, rho*scale); cairo_stroke(c.cr);

    auto draw_root = [&](std::complex<double> r, RGB col, const char* lbl) {
        double rx = cx + r.real()*scale;
        double ry = cy - r.imag()*scale;
        c.set(col);
        c.circle(rx, ry, 5); cairo_fill(c.cr);
        c.set(WHITE);
        c.text(rx+8, ry-8, lbl, 14);
    };

    draw_root(r1, EXACT_CLR, "∛2 (Real)");
    draw_root(r2, COMPLEX_CLR, "ω∛2");
    draw_root(r3, COMPLEX_CLR, "ω²∛2");

    c.set(ACCENT);
    c.text(px+20, py+30, "Splitting field of x³ - 2 in ℂ", 18);
    c.text(px+20, py+55, "ℚ(∛2) contains only the real root.", 14);
    c.text(px+20, py+75, "Full Galois closure requires adjoining ω.", 14);
}

void draw_norm_invariance(Ctx& c, double px, double py, double pw, double ph) {
    c.set(PANEL_BG); c.fill_rect(px, py, pw, ph);
    c.set(BORDER); cairo_set_line_width(c.cr, 1.0); cairo_rectangle(c.cr, px, py, pw, ph); cairo_stroke(c.cr);

    c.set(ACCENT);
    c.text(px+20, py+30, "Norm Multiplicativity in ℚ(∛2)", 18);

    Qcbrt2 alpha(rat(1), rat(1), rat(0));
    Qcbrt2 beta(rat(0), rat(1), rat(1));
    Qcbrt2 gamma = alpha * beta;

    auto n_alpha = alpha.norm();
    auto n_beta = beta.norm();
    auto n_gamma = gamma.norm();

    c.set(WHITE);
    int y = py + 70;
    c.text(px+20, y, ("α = " + alpha.str()).c_str(), 14); y+=25;
    c.text(px+20, y, ("N(α) = " + n_alpha.str()).c_str(), 14); y+=40;

    c.text(px+20, y, ("β = " + beta.str()).c_str(), 14); y+=25;
    c.text(px+20, y, ("N(β) = " + n_beta.str()).c_str(), 14); y+=40;

    c.set(EXACT_CLR);
    c.text(px+20, y, ("α·β = " + gamma.str()).c_str(), 14); y+=25;
    c.text(px+20, y, ("N(α·β) = " + n_gamma.str()).c_str(), 14); y+=30;

    bool check = (n_alpha * n_beta == n_gamma);
    c.set(check ? EXACT_CLR : ACCENT);
    c.text(px+20, y, check ? "✓ N(α)·N(β) == N(α·β)  (EXACT)" : "✗ Norm identity failed", 16);
}

int main() {
    cairo_surface_t* surf = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, WIN_W, WIN_H);
    cairo_t* cr = cairo_create(surf);
    Ctx ctx{cr};

    ctx.set(BG); cairo_paint(cr);

    draw_cubic_roots(ctx, 50, 50, 500, 700);
    draw_norm_invariance(ctx, 600, 50, 550, 700);

    cairo_surface_write_to_png(surf, "cubic_viz.png");
    cairo_destroy(cr);
    cairo_surface_destroy(surf);
    return 0;
}
