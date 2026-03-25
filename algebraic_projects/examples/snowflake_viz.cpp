#include "../include/algebraic_field.hpp"
#include <cairo/cairo.h>
#include <algorithm>
#include <cmath>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>

static const int WIN_W = 1200, WIN_H = 800;

struct Vec2Q { Q3 x, y; };
struct Vec2F { double x, y; };

void koch_step_exact(std::vector<Vec2Q>& points) {
    std::vector<Vec2Q> next;
    Q3 one_third = Q3(rat(1, 3));
    Q3 half = Q3(rat(1, 2));
    Q3 sqrt3_6 = Q3(rat(0), rat(1, 6));

    for (size_t i = 0; i < points.size() - 1; ++i) {
        Vec2Q p1 = points[i];
        Vec2Q p2 = points[i+1];
        Q3 dx = p2.x - p1.x;
        Q3 dy = p2.y - p1.y;
        Vec2Q a = { p1.x + one_third * dx, p1.y + one_third * dy };
        Vec2Q c = { p1.x + Q3(rat(2, 3)) * dx, p1.y + Q3(rat(2, 3)) * dy };
        Vec2Q b = {
            half * (p1.x + p2.x) + sqrt3_6 * dy,
            half * (p1.y + p2.y) - sqrt3_6 * dx
        };
        next.push_back(p1);
        next.push_back(a);
        next.push_back(b);
        next.push_back(c);
    }
    next.push_back(points.back());
    points = next;
}

void koch_step_float(std::vector<Vec2F>& points) {
    std::vector<Vec2F> next;
    const double inv3 = 1.0 / 3.0;
    const double s3_6 = std::sqrt(3.0) / 6.0;

    for (size_t i = 0; i < points.size() - 1; ++i) {
        Vec2F p1 = points[i];
        Vec2F p2 = points[i+1];
        double dx = p2.x - p1.x;
        double dy = p2.y - p1.y;
        Vec2F a = { p1.x + inv3 * dx, p1.y + inv3 * dy };
        Vec2F c = { p1.x + 2.0 * inv3 * dx, p1.y + 2.0 * inv3 * dy };
        Vec2F b = {
            0.5 * (p1.x + p2.x) + s3_6 * dy,
            0.5 * (p1.y + p2.y) - s3_6 * dx
        };
        next.push_back(p1);
        next.push_back(a);
        next.push_back(b);
        next.push_back(c);
    }
    next.push_back(points.back());
    points = next;
}

int main() {
    cairo_surface_t* surf = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, WIN_W, WIN_H);
    cairo_t* cr = cairo_create(surf);

    cairo_set_source_rgb(cr, 0.05, 0.05, 0.1);
    cairo_paint(cr);

    Q3 side = Q3(rat(500));
    Q3 height = Q3(rat(0), rat(250));

    Vec2Q p1q = { Q3(rat(50)), Q3(rat(200)) + height };
    Vec2Q p2q = { Q3(rat(50)) + side, Q3(rat(200)) + height };
    Vec2Q p3q = { Q3(rat(300)), Q3(rat(200)) };
    std::vector<Vec2Q> ptsQ = { p1q, p3q, p2q, p1q };

    Vec2F p1f = { 50.0, 200.0 + 250.0 * std::sqrt(3.0) };
    Vec2F p2f = { 550.0, 200.0 + 250.0 * std::sqrt(3.0) };
    Vec2F p3f = { 300.0, 200.0 };
    std::vector<Vec2F> ptsF = { p1f, p3f, p2f, p1f };

    const int steps = 6;
    for (int i = 0; i < steps; ++i) {
        koch_step_exact(ptsQ);
        koch_step_float(ptsF);
    }

    // Full snowflake
    cairo_set_source_rgb(cr, 0.2, 0.5, 0.8);
    cairo_set_line_width(cr, 1.0);
    cairo_move_to(cr, ptsQ[0].x.approx(), ptsQ[0].y.approx());
    for (size_t i = 1; i < ptsQ.size(); ++i) cairo_line_to(cr, ptsQ[i].x.approx(), ptsQ[i].y.approx());
    cairo_stroke(cr);

    // Zoom panel setup
    double zx = 650, zy = 100, zw = 500, zh = 500;
    cairo_set_source_rgb(cr, 0.1, 0.1, 0.2);
    cairo_rectangle(cr, zx, zy, zw, zh);
    cairo_fill(cr);
    cairo_set_source_rgb(cr, 0.3, 0.3, 0.5);
    cairo_rectangle(cr, zx, zy, zw, zh);
    cairo_stroke(cr);

    // Zoom into a specific point (e.g., the top-most vertex or one deep in recursion)
    size_t target_idx = ptsQ.size() / 3;
    double target_qx = ptsQ[target_idx].x.approx();
    double target_qy = ptsQ[target_idx].y.approx();
    double zoom_factor = 1e6;

    auto to_zoom = [&](double px, double py) -> std::pair<double, double> {
        return { zx + zw/2 + (px - target_qx) * zoom_factor,
                 zy + zh/2 + (py - target_qy) * zoom_factor };
    };

    // Draw exact path in zoom
    cairo_set_source_rgb(cr, 0.2, 0.9, 0.4);
    cairo_set_line_width(cr, 2.0);
    for (int i = -10; i <= 10; ++i) {
        size_t idx = (target_idx + i + ptsQ.size()) % ptsQ.size();
        auto [sx, sy] = to_zoom(ptsQ[idx].x.approx(), ptsQ[idx].y.approx());
        if (i == -10) cairo_move_to(cr, sx, sy);
        else cairo_line_to(cr, sx, sy);
    }
    cairo_stroke(cr);

    // Draw float path in zoom
    cairo_set_source_rgb(cr, 0.9, 0.3, 0.2);
    cairo_set_line_width(cr, 1.0);
    for (int i = -10; i <= 10; ++i) {
        size_t idx = (target_idx + i + ptsF.size()) % ptsF.size();
        auto [sx, sy] = to_zoom(ptsF[idx].x, ptsF[idx].y);
        if (i == -10) cairo_move_to(cr, sx, sy);
        else cairo_line_to(cr, sx, sy);
    }
    cairo_stroke(cr);

    // Labels
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 20);
    cairo_set_source_rgb(cr, 0.9, 0.9, 0.9);
    cairo_move_to(cr, 50, 50);
    cairo_show_text(cr, "Koch Snowflake Drift: Q(sqrt(3)) vs float64");

    cairo_set_font_size(cr, 14);
    cairo_move_to(cr, zx, zy - 10);
    cairo_show_text(cr, "1,000,000x Zoom at recursive vertex");

    cairo_set_source_rgb(cr, 0.2, 0.9, 0.4);
    cairo_move_to(cr, zx, zy + zh + 30);
    cairo_show_text(cr, "GREEN: Exact algebraic path (No drift)");

    cairo_set_source_rgb(cr, 0.9, 0.3, 0.2);
    cairo_move_to(cr, zx, zy + zh + 55);
    cairo_show_text(cr, "RED: Floating-point path (Accumulated drift)");

    // Detail text
    std::stringstream ss;
    ss << std::fixed << std::setprecision(15);
    ss << "Exact X: " << ptsQ[target_idx].x.str() << " (approx: " << ptsQ[target_idx].x.approx() << ")";
    cairo_set_source_rgb(cr, 0.7, 0.7, 0.7);
    cairo_move_to(cr, 50, WIN_H - 100);
    cairo_show_text(cr, ss.str().c_str());

    ss.str("");
    ss << "Float X: " << ptsF[target_idx].x;
    cairo_move_to(cr, 50, WIN_H - 80);
    cairo_show_text(cr, ss.str().c_str());

    double diff = std::abs(ptsQ[target_idx].x.approx() - ptsF[target_idx].x);
    ss.str("");
    ss << "Absolute Error: " << diff;
    cairo_move_to(cr, 50, WIN_H - 60);
    cairo_show_text(cr, ss.str().c_str());

    cairo_surface_write_to_png(surf, "snowflake_viz.png");
    cairo_destroy(cr);
    cairo_surface_destroy(surf);
    return 0;
}
