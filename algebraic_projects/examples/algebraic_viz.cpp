#include "../include/algebraic_field.hpp"
#include <cairo/cairo.h>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

static const int WIN_W = 1400, WIN_H = 900;

struct RGB { double r,g,b; };

static constexpr RGB BG        { 0.09, 0.09, 0.11 };
static constexpr RGB PANEL_BG  { 0.12, 0.12, 0.15 };
static constexpr RGB BORDER    { 0.25, 0.27, 0.35 };
static constexpr RGB TITLE_BG  { 0.16, 0.16, 0.22 };
static constexpr RGB WHITE     { 0.92, 0.93, 0.95 };
static constexpr RGB DIM       { 0.55, 0.57, 0.63 };
static constexpr RGB EXACT_CLR { 0.22, 0.78, 0.55 };
static constexpr RGB FLOAT_CLR { 0.95, 0.35, 0.30 };
static constexpr RGB ACCENT    { 0.95, 0.78, 0.20 };
static constexpr RGB BLUE_LT   { 0.35, 0.65, 1.00 };
static constexpr RGB PURPLE    { 0.72, 0.42, 0.95 };
static constexpr RGB ORANGE    { 0.98, 0.58, 0.18 };

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
    void font(const char* face, cairo_font_weight_t w=CAIRO_FONT_WEIGHT_NORMAL, double sz=12){
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
    void panel(double x,double y,double w,double h,const char* title){
        set({0,0,0},0.4); fill_rect(x+4,y+4,w,h);
        set(PANEL_BG); fill_rect(x,y,w,h);
        set(TITLE_BG); fill_rect(x,y,w,28);
        font("Monospace",CAIRO_FONT_WEIGHT_BOLD,12);
        set(ACCENT); text(x+10,y+19,title);
        set(BORDER); stroke_rect(x,y,w,h,1.5);
        set(BORDER); line(x,y+28,x+w,y+28,1.0);
    }
    void grid(double x0,double y0,double x1,double y1, double dx,double dy){
        set(BORDER,0.35); cairo_set_line_width(cr,0.5);
        for(double x=x0;x<=x1+0.1;x+=dx){ cairo_move_to(cr,x,y0);cairo_line_to(cr,x,y1);cairo_stroke(cr); }
        for(double y=y0;y<=y1+0.1;y+=dy){ cairo_move_to(cr,x0,y);cairo_line_to(cr,x1,y);cairo_stroke(cr); }
    }
};

void draw_theodorus(Ctx& c, double px, double py, double pw, double ph){
    c.panel(px,py,pw,ph, "PANEL 1 — Spiral of Theodorus");
    const double STEPS = 45, SCALE = 38.0;
    const double cx = px + pw*0.38, cy = py + ph*0.56;
    cairo_save(c.cr); cairo_rectangle(c.cr, px+1, py+29, pw-2, ph-30); cairo_clip(c.cr);
    c.grid(px, py+28, px+pw, py+ph, 40, 40);
    struct Point { double x, y; };
    std::vector<Point> alg_pts, flt_pts;
    std::vector<double> flt_r;
    {
        double angle = 0.0;
        alg_pts.push_back({cx, cy}); alg_pts.push_back({cx + SCALE*1.0, cy});
        for(int n = 1; n <= (int)STEPS; ++n){
            angle += std::atan(1.0 / std::sqrt(static_cast<double>(n)));
            double x = cx + SCALE * std::sqrt(static_cast<double>(n+1)) * std::cos(angle);
            double y = cy - SCALE * std::sqrt(static_cast<double>(n+1)) * std::sin(angle);
            alg_pts.push_back({x, y});
        }
    }
    {
        double fx = cx + SCALE*1.0, fy = cy;
        flt_pts.push_back({cx, cy}); flt_pts.push_back({fx, fy});
        for(int n = 1; n <= (int)STEPS; ++n){
            double vx = fx - cx, vy = fy - cy;
            double r_curr = std::sqrt(vx*vx + vy*vy); flt_r.push_back(r_curr);
            double nx = -vy / r_curr, ny =  vx / r_curr;
            fx += nx; fy += ny; flt_pts.push_back({fx, fy});
        }
    }
    {
        cairo_save(c.cr); double dash[] = {4, 4}; cairo_set_dash(c.cr, dash, 2, 0);
        c.set(FLOAT_CLR, 0.60); cairo_set_line_width(c.cr, 1.2);
        cairo_move_to(c.cr, flt_pts[0].x, flt_pts[0].y);
        for(size_t i=1; i<flt_pts.size(); ++i) cairo_line_to(c.cr, flt_pts[i].x, flt_pts[i].y);
        cairo_stroke(c.cr); cairo_restore(c.cr);
    }
    {
        c.set(EXACT_CLR, 0.85); cairo_set_line_width(c.cr, 1.6);
        for(int i=1; i<(int)alg_pts.size()-1; ++i){
            c.set(EXACT_CLR, 0.25); cairo_set_line_width(c.cr, 0.6);
            cairo_move_to(c.cr, cx, cy); cairo_line_to(c.cr, alg_pts[i].x, alg_pts[i].y); cairo_stroke(c.cr);
        }
        c.set(EXACT_CLR, 0.9); cairo_set_line_width(c.cr, 2.0);
        cairo_move_to(c.cr, alg_pts[1].x, alg_pts[1].y);
        for(size_t i=2; i<alg_pts.size(); ++i) cairo_line_to(c.cr, alg_pts[i].x, alg_pts[i].y);
        cairo_stroke(c.cr);
    }
    for(int n=1; n<=10 && n<(int)alg_pts.size(); ++n){
        double ax = alg_pts[n].x, ay = alg_pts[n].y;
        c.set(EXACT_CLR); c.circle(ax, ay, 2.5); cairo_fill(c.cr);
    }
    {
        double bx = px + pw*0.56, by = py + ph*0.35, bw = pw*0.40, bh = ph*0.55;
        c.set({0.08,0.10,0.14}, 0.85); c.fill_rect(bx, by, bw, bh);
        c.set(BORDER, 0.7); c.stroke_rect(bx, by, bw, bh, 1.0);
        c.font("Monospace", CAIRO_FONT_WEIGHT_BOLD, 9.5); c.set(ACCENT);
        c.text_center(bx+bw/2, by+12, "Radius error |float - algebraic|  ×10⁻¹⁴");
        const int PLOT_N = (int)flt_r.size();
        double plot_x0 = bx+6, plot_x1 = bx+bw-6, plot_y0 = by+16, plot_y1 = by+bh-18;
        double plot_w  = plot_x1 - plot_x0, plot_h  = plot_y1 - plot_y0;
        c.grid(plot_x0, plot_y0, plot_x1, plot_y1, plot_w/5, plot_h/4);
        std::vector<double> errors; double max_err = 0;
        for(int n=1; n<PLOT_N; ++n){
            double r_alg = std::sqrt(double(n)), e = std::abs(flt_r[n-1] - r_alg);
            errors.push_back(e); max_err = std::max(max_err, e);
        }
        if(max_err < 1e-18) max_err = 1e-14;
        double scale_y = plot_h / (max_err * 1e14);
        cairo_set_line_width(c.cr, 1.0);
        for(int i=0; i<(int)errors.size(); ++i){
            double ex = plot_x0 + (double(i)/double(PLOT_N)) * plot_w;
            double ey = plot_y1 - errors[i]*1e14 * scale_y;
            c.set(FLOAT_CLR, 0.80); cairo_move_to(c.cr, ex, plot_y1); cairo_line_to(c.cr, ex, ey); cairo_stroke(c.cr);
        }
    }
    cairo_restore(c.cr);
}

void draw_pell_cancel(Ctx& c, double px, double py, double pw, double ph){
    c.panel(px,py,pw,ph, "PANEL 2 — Pell Catastrophic Cancellation");
    cairo_save(c.cr); cairo_rectangle(c.cr, px+1, py+29, pw-2, ph-30); cairo_clip(c.cr);
    using i128 = __int128;
    struct Pell { i128 p, q; int norm_exact; double norm_float; };
    std::vector<Pell> pells;
    i128 pp=1, pq=1; int nterms = 30;
    for(int n=0; n<nterms; ++n){
        pells.push_back({pp, pq, (int)(pp*pp - 2*pq*pq), (double)pp*(double)pp - 2.0*(double)pq*(double)pq});
        i128 npp = pp + 2*pq, npq = pp + pq; pp = npp; pq = npq;
    }
    double plot_x0 = px+55, plot_y0 = py+48, plot_x1 = px+pw-20, plot_y1 = py+ph-75;
    double plot_w = plot_x1-plot_x0, plot_h = plot_y1-plot_y0;
    c.grid(plot_x0, plot_y0, plot_x1, plot_y1, plot_w/6, plot_h/4);
    double log_max = 16.0, log_min = -1.0;
    auto to_screen = [&](double n, double log_err) -> std::pair<double,double> {
        return {plot_x0 + (n/double(nterms-1))*plot_w, plot_y1 - (log_err-log_min)/(log_max-log_min)*plot_h};
    };
    for(int i=1; i<(int)pells.size(); ++i){
        double e0 = std::abs(pells[i-1].norm_float - pells[i-1].norm_exact);
        double e1 = std::abs(pells[i].norm_float - pells[i].norm_exact);
        auto [sx0,sy0] = to_screen(i-1, e0 > 1e-20 ? std::log10(e0) : log_min);
        auto [sx1,sy1] = to_screen(i, e1 > 1e-20 ? std::log10(e1) : log_min);
        c.set(FLOAT_CLR); cairo_move_to(c.cr, sx0, sy0); cairo_line_to(c.cr, sx1, sy1); cairo_stroke(c.cr);
    }
    cairo_restore(c.cr);
}

void draw_golden(Ctx& c, double px, double py, double pw, double ph){
    c.panel(px,py,pw,ph, "PANEL 3 — Golden Rectangle Decomposition");
    Q5 phi = { rat(1,2), rat(1,2) };
    double margin = 18.0, rect_x = px + margin, rect_y = py + 36, rect_h = ph - 44;
    double rect_w = rect_h * phi.approx();
    if(rect_w > pw - 2*margin){ rect_w = pw - 2*margin; rect_h = rect_w / phi.approx(); }
    struct Rect { double x,y,w,h; bool horizontal; };
    std::vector<Rect> rects = {{rect_x, rect_y, rect_w, rect_h, true}};
    for(int lvl=0; lvl<9 && !rects.empty(); ++lvl){
        Rect r = rects.back(); rects.pop_back();
        c.set(EXACT_CLR, 0.7); c.stroke_rect(r.x, r.y, r.w, r.h, 1.5-lvl*0.1);
        if(r.horizontal){
            if(r.w-r.h > 2) rects.push_back({r.x+r.h, r.y, r.w-r.h, r.h, false});
        } else {
            if(r.h-r.w > 2) rects.push_back({r.x, r.y+r.w, r.w, r.h-r.w, true});
        }
    }
}

void draw_pell_power(Ctx& c, double px, double py, double pw, double ph){
    c.panel(px,py,pw,ph, "PANEL 4 — (1+√2)ⁿ Galois Norm Invariance");
    const int NMAX = 40; Q2 alpha = { rat(1), rat(1) }, cur = { rat(1), rat(0) };
    double lp_x = px+50, lp_y = py+38, lp_w = pw*0.44, lp_h = ph-80;
    c.grid(lp_x, lp_y, lp_x+lp_w, lp_y+lp_h, lp_w/5, lp_h/4);
    for(int n=0; n<=NMAX; ++n){
        double log_a = std::log10(cur.a.approx() + 1), bar_h = (log_a/15.0) * lp_h;
        c.set(BLUE_LT, 0.75); c.fill_rect(lp_x + (double(n)/NMAX)*lp_w, lp_y+lp_h-bar_h, (lp_w/NMAX)*0.7, bar_h);
        cur = cur * alpha;
    }
}

void render(Ctx& c){
    c.set(BG); cairo_paint(c.cr);
    c.set({0.1,0.1,0.16}); c.fill_rect(0,0,WIN_W,30);
    c.font("Monospace",CAIRO_FONT_WEIGHT_BOLD,13); c.set(ACCENT); c.text_center(WIN_W/2, 21, "Exact Algebraic Arithmetic vs IEEE 754 Float");
    double m = 6, hw = WIN_W/2.0 - 1.5*m, hh = (WIN_H-32)/2.0 - 1.5*m;
    draw_theodorus(c, m, 32+m, hw, hh); draw_pell_cancel(c, WIN_W/2.0+m/2, 32+m, hw, hh);
    draw_golden(c, m, 32+hh+2*m, hw, hh); draw_pell_power(c, WIN_W/2.0+m/2, 32+hh+2*m, hw, hh);
}

int main() {
    cairo_surface_t* surf = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, WIN_W, WIN_H);
    cairo_t* cr = cairo_create(surf);
    Ctx ctx{cr}; render(ctx);
    cairo_surface_write_to_png(surf, "algebraic_viz.png");
    cairo_destroy(cr); cairo_surface_destroy(surf);
    return 0;
}
