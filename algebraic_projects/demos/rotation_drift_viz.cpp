#include "../algebraic_field.hpp"
#include <cairo/cairo.h>
#include <cmath>
#include <vector>
#include <string>
#include <iostream>

static const int WIN_W = 1000, WIN_H = 1000;

// Rotation by 30 degrees in Q(sqrt(3))
// cos(30) = sqrt(3)/2, sin(30) = 1/2
// x' = x*cos - y*sin
// y' = x*sin + y*cos

struct Vec2Q { Q3 x, y; };
struct Vec2F { double x, y; };

Vec2Q rotateQ(Vec2Q p) {
    Q3 cos30 = { rat(0), rat(1, 2) }; // sqrt(3)/2
    Q3 sin30 = { rat(1, 2), rat(0) }; // 1/2
    return {
        p.x * cos30 - p.y * sin30,
        p.x * sin30 + p.y * cos30
    };
}

Vec2F rotateF(Vec2F p) {
    const double cos30 = std::sqrt(3.0) / 2.0;
    const double sin30 = 0.5;
    return {
        p.x * cos30 - p.y * sin30,
        p.x * sin30 + p.y * cos30
    };
}

int main() {
    cairo_surface_t* surf = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, WIN_W, WIN_H);
    cairo_t* cr = cairo_create(surf);

    cairo_set_source_rgb(cr, 0.02, 0.02, 0.05);
    cairo_paint(cr);

    const int iterations = 1000;
    const double scale = 300.0;
    const double cx = WIN_W / 2.0;
    const double cy = WIN_H / 2.0;

    Vec2Q pQ = { Q3(rat(1)), Q3(rat(0)) };
    Vec2F pF = { 1.0, 0.0 };

    std::vector<Vec2F> pathF;
    std::vector<Vec2F> pathQ;

    for (int i = 0; i < iterations; ++i) {
        pathQ.push_back({pQ.x.approx(), pQ.y.approx()});
        pathF.push_back(pF);

        pQ = rotateQ(pQ);
        pF = rotateF(pF);
    }

    // Exact path (perfect circle)
    cairo_set_source_rgb(cr, 0.2, 0.8, 0.3);
    cairo_set_line_width(cr, 1.5);
    cairo_move_to(cr, cx + pathQ[0].x * scale, cy + pathQ[0].y * scale);
    for (const auto& p : pathQ) cairo_line_to(cr, cx + p.x * scale, cy + p.y * scale);
    cairo_stroke(cr);

    // Float path (spirals out/in)
    cairo_set_source_rgb(cr, 0.8, 0.2, 0.2);
    cairo_set_line_width(cr, 1.0);
    cairo_move_to(cr, cx + pathF[0].x * scale, cy + pathF[0].y * scale);
    for (const auto& p : pathF) cairo_line_to(cr, cx + p.x * scale, cy + p.y * scale);
    cairo_stroke(cr);

    // Zoom setup
    double zx = 700, zy = 50, zw = 250, zh = 250;
    cairo_set_source_rgb(cr, 0.1, 0.1, 0.15);
    cairo_rectangle(cr, zx, zy, zw, zh);
    cairo_fill(cr);
    cairo_set_source_rgb(cr, 0.4, 0.4, 0.4);
    cairo_rectangle(cr, zx, zy, zw, zh);
    cairo_stroke(cr);

    double z_target_x = pathQ.back().x;
    double z_target_y = pathQ.back().y;
    double zoom = 5000.0;

    auto to_zoom = [&](double px, double py) -> std::pair<double, double> {
        return { zx + zw/2 + (px - z_target_x) * zoom,
                 zy + zh/2 + (py - z_target_y) * zoom };
    };

    // Zoom exact
    cairo_set_source_rgb(cr, 0.2, 0.8, 0.3);
    for (int i = iterations - 20; i < iterations; ++i) {
        auto [sx, sy] = to_zoom(pathQ[i].x, pathQ[i].y);
        if (i == iterations-20) cairo_move_to(cr, sx, sy);
        else cairo_line_to(cr, sx, sy);
    }
    cairo_stroke(cr);

    // Zoom float
    cairo_set_source_rgb(cr, 0.8, 0.2, 0.2);
    for (int i = iterations - 20; i < iterations; ++i) {
        auto [sx, sy] = to_zoom(pathF[i].x, pathF[i].y);
        if (i == iterations-20) cairo_move_to(cr, sx, sy);
        else cairo_line_to(cr, sx, sy);
    }
    cairo_stroke(cr);

    // Info
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_source_rgb(cr, 0.9, 0.9, 0.9);
    cairo_set_font_size(cr, 24);
    cairo_move_to(cr, 50, 50);
    cairo_show_text(cr, "Rotation Invariance: Q(sqrt(3)) vs Float64");

    cairo_set_font_size(cr, 16);
    cairo_move_to(cr, 50, 80);
    cairo_show_text(cr, "1,000 rotations by 30 degrees");

    cairo_set_source_rgb(cr, 0.2, 0.8, 0.3);
    cairo_move_to(cr, 50, WIN_H - 80);
    cairo_show_text(cr, "GREEN: Exact (Norm stays 1 exactly)");

    cairo_set_source_rgb(cr, 0.8, 0.2, 0.2);
    cairo_move_to(cr, 50, WIN_H - 50);
    cairo_show_text(cr, "RED: Float64 (Drifts due to cos/sin precision)");

    cairo_set_source_rgb(cr, 0.6, 0.6, 0.6);
    cairo_move_to(cr, zx, zy + zh + 20);
    cairo_show_text(cr, "5000x Zoom at end of path");

    cairo_surface_write_to_png(surf, "rotation_drift_viz.png");
    cairo_destroy(cr);
    cairo_surface_destroy(surf);
    return 0;
}
