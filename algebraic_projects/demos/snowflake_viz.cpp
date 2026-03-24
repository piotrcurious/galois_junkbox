#include "../algebraic_field.hpp"
#include <cairo/cairo.h>
#include <algorithm>
#include <cmath>
#include <vector>

static const int WIN_W = 1000, WIN_H = 1000;

struct Vec2 { Q3 x, y; };

void koch_step(std::vector<Vec2>& points) {
    std::vector<Vec2> next;
    Q3 one_third = Q3(rat(1, 3));
    Q3 half = Q3(rat(1, 2));
    Q3 sqrt3_6 = Q3(rat(0), rat(1, 6));

    for (size_t i = 0; i < points.size() - 1; ++i) {
        Vec2 p1 = points[i];
        Vec2 p2 = points[i+1];

        Q3 dx = p2.x - p1.x;
        Q3 dy = p2.y - p1.y;

        Vec2 a = { p1.x + one_third * dx, p1.y + one_third * dy };
        Vec2 c = { p1.x + Q3(rat(2, 3)) * dx, p1.y + Q3(rat(2, 3)) * dy };

        Vec2 b = {
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

int main() {
    cairo_surface_t* surf = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, WIN_W, WIN_H);
    cairo_t* cr = cairo_create(surf);

    cairo_set_source_rgb(cr, 0.05, 0.05, 0.1);
    cairo_paint(cr);

    Q3 side = Q3(rat(800));
    Q3 height = Q3(rat(0), rat(400));

    Vec2 p1 = { Q3(rat(100)), Q3(rat(250)) + height };
    Vec2 p2 = { Q3(rat(100)) + side, Q3(rat(250)) + height };
    Vec2 p3 = { Q3(rat(500)), Q3(rat(250)) };

    std::vector<Vec2> points = { p1, p3, p2, p1 };

    for (int i = 0; i < 4; ++i) {
        koch_step(points);
    }

    cairo_set_source_rgb(cr, 0.3, 0.7, 1.0);
    cairo_set_line_width(cr, 1.0);
    cairo_move_to(cr, points[0].x.approx(), points[0].y.approx());
    for (size_t i = 1; i < points.size(); ++i) {
        cairo_line_to(cr, points[i].x.approx(), points[i].y.approx());
    }
    cairo_stroke(cr);

    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 24);
    cairo_move_to(cr, 50, 50);
    cairo_set_source_rgb(cr, 0.9, 0.9, 0.9);
    cairo_show_text(cr, "Koch Snowflake in Q(sqrt(3))");

    cairo_set_font_size(cr, 16);
    cairo_move_to(cr, 50, 80);
    cairo_show_text(cr, "All vertex coordinates are EXACT algebraic elements.");

    cairo_surface_write_to_png(surf, "snowflake_viz.png");
    cairo_destroy(cr);
    cairo_surface_destroy(surf);
    return 0;
}
