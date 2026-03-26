#include "../include/algebraic_field.hpp"
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/fl_draw.H>
#include <vector>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <sstream>

struct Vec2Q { Q3 x, y; };
struct Vec2F { double x, y; };

void auto_zoom_callback(void* data);

class SnowflakeWidget : public Fl_Widget {
public:
    double zoom;
    double offset_x, offset_y;
    int depth;
    bool show_float;
    bool auto_zoom;
    double auto_zoom_factor;
    int target_point_idx;
    bool side_by_side;

    SnowflakeWidget(int X, int Y, int W, int H) : Fl_Widget(X, Y, W, H), zoom(1.0), offset_x(0), offset_y(0), depth(4),
                                                   show_float(true), auto_zoom(false), auto_zoom_factor(1.0), target_point_idx(0), side_by_side(false) {}

    void draw_snowflake_internal(double cx, double cy, double current_zoom, bool is_exact, Fl_Color col) {
        // Setup base points
        Vec2Q p1q = { Q3(rat(-300)), Q3(rat(0), rat(150))  };
        Vec2Q p2q = { Q3(rat(300)),  Q3(rat(0), rat(150))  };
        Vec2Q p3q = { Q3(rat(0)),    Q3(rat(0), rat(-150)) };
        std::vector<Vec2Q> ptsQ = { p1q, p3q, p2q, p1q };

        Vec2F p1f = { -300.0, 150.0 * std::sqrt(3.0) };
        Vec2F p2f = {  300.0, 150.0 * std::sqrt(3.0) };
        Vec2F p3f = {    0.0, -150.0 * std::sqrt(3.0) };
        std::vector<Vec2F> ptsF = { p1f, p3f, p2f, p1f };

        // Step lambda
        auto stepQ = [](std::vector<Vec2Q>& points) {
            std::vector<Vec2Q> next;
            Q3 one3 = Q3(rat(1, 3)); Q3 half = Q3(rat(1, 2)); Q3 s3_6 = Q3(rat(0), rat(1, 6));
            for (size_t i = 0; i < points.size() - 1; i++) {
                Vec2Q p1 = points[i]; Vec2Q p2 = points[i+1];
                Q3 dx = p2.x - p1.x; Q3 dy = p2.y - p1.y;
                Vec2Q a = { p1.x + one3 * dx, p1.y + one3 * dy };
                Vec2Q c = { p1.x + Q3(rat(2, 3)) * dx, p1.y + Q3(rat(2, 3)) * dy };
                Vec2Q b = { half * (p1.x + p2.x) + s3_6 * dy, half * (p1.y + p2.y) - s3_6 * dx };
                next.push_back(p1); next.push_back(a); next.push_back(b); next.push_back(c);
            }
            next.push_back(points.back()); points = next;
        };

        auto stepF = [](std::vector<Vec2F>& points) {
            std::vector<Vec2F> next;
            const double inv3 = 1.0/3.0; const double s3_6 = std::sqrt(3.0)/6.0;
            for (size_t i = 0; i < points.size() - 1; i++) {
                Vec2F p1 = points[i]; Vec2F p2 = points[i+1];
                double dx = p2.x - p1.x; double dy = p2.y - p1.y;
                Vec2F a = { p1.x + inv3 * dx, p1.y + inv3 * dy };
                Vec2F c = { p1.x + 2.0 * inv3 * dx, p1.y + 2.0 * inv3 * dy };
                Vec2F b = { 0.5 * (p1.x + p2.x) + s3_6 * dy, 0.5 * (p1.y + p2.y) - s3_6 * dx };
                next.push_back(p1); next.push_back(a); next.push_back(b); next.push_back(c);
            }
            next.push_back(points.back()); points = next;
        };

        for (int i = 0; i < depth; i++) { stepQ(ptsQ); stepF(ptsF); }

        if (auto_zoom) {
            size_t idx = target_point_idx % ptsQ.size();
            cx = cx - ptsQ[idx].x.approx() * current_zoom;
            cy = cy - ptsQ[idx].y.approx() * current_zoom;
        }

        fl_color(col);
        if (is_exact) {
            for (size_t i = 0; i < ptsQ.size() - 1; i++) {
                fl_line(cx + ptsQ[i].x.approx() * current_zoom, cy + ptsQ[i].y.approx() * current_zoom,
                        cx + ptsQ[i+1].x.approx() * current_zoom, cy + ptsQ[i+1].y.approx() * current_zoom);
            }
        } else {
            for (size_t i = 0; i < ptsF.size() - 1; i++) {
                fl_line(cx + ptsF[i].x * current_zoom, cy + ptsF[i].y * current_zoom,
                        cx + ptsF[i+1].x * current_zoom, cy + ptsF[i+1].y * current_zoom);
            }
        }
    }

    void draw() override {
        fl_push_clip(x(), y(), w(), h());
        fl_color(FL_BLACK);
        fl_rectf(x(), y(), w(), h());

        double current_zoom = zoom * auto_zoom_factor;

        if (!side_by_side) {
            draw_snowflake_internal(x() + w()/2.0 + offset_x, y() + h()/2.0 + offset_y, current_zoom, true, FL_CYAN);
            if (show_float) {
                fl_line_style(FL_DOT);
                draw_snowflake_internal(x() + w()/2.0 + offset_x, y() + h()/2.0 + offset_y, current_zoom, false, FL_RED);
                fl_line_style(0);
            }
        } else {
            // Split screen
            fl_push_clip(x(), y(), w()/2, h());
            draw_snowflake_internal(x() + w()/4.0 + offset_x, y() + h()/2.0 + offset_y, current_zoom, true, FL_CYAN);
            fl_color(FL_WHITE); fl_draw("EXACT Q(sqrt(3))", x()+10, y()+h()-20);
            fl_pop_clip();

            fl_push_clip(x() + w()/2, y(), w()/2, h());
            draw_snowflake_internal(x() + 3*w()/4.0 + offset_x, y() + h()/2.0 + offset_y, current_zoom, false, FL_RED);
            fl_color(FL_WHITE); fl_draw("FLOAT64 (Accumulated)", x()+w()/2+10, y()+h()-20);
            fl_pop_clip();

            fl_color(FL_WHITE); fl_line(x()+w()/2, y(), x()+w()/2, y()+h());
        }

        // Overlay text
        fl_color(FL_WHITE); fl_font(FL_HELVETICA, 14);
        fl_draw(("Zoom: " + std::to_string(current_zoom)).c_str(), x() + 10, y() + 20);
        fl_draw(("Depth: " + std::to_string(depth) + " (Up/Down to change)").c_str(), x() + 10, y() + 40);
        fl_draw("F: Toggle Float Overlay, S: Split Screen, A: Auto-Zoom, R: Reset", x() + 10, y() + 60);
        fl_draw("CLICK to set zoom focus point.", x() + 10, y() + 80);

        if (auto_zoom) {
            fl_color(FL_YELLOW);
            fl_draw("AUTO-ZOOM ACTIVE. Coordinates in Red are drifting...", x() + 10, y() + 100);
        }

        fl_pop_clip();
    }

    int handle(int event) override {
        static int last_x, last_y;
        switch (event) {
            case FL_FOCUS: return 1;
            case FL_UNFOCUS: return 1;
            case FL_PUSH:
                Fl::focus(this);
                last_x = Fl::event_x(); last_y = Fl::event_y();
                // Pick target point based on screen click (roughly)
                target_point_idx = (target_point_idx + 10) % 1000; // simple cycling for now
                redraw();
                return 1;
            case FL_DRAG:
                offset_x += (Fl::event_x() - last_x);
                offset_y += (Fl::event_y() - last_y);
                last_x = Fl::event_x(); last_y = Fl::event_y();
                redraw();
                return 1;
            case FL_MOUSEWHEEL:
                if (Fl::event_dy() < 0) zoom *= 1.2;
                else zoom /= 1.2;
                redraw();
                return 1;
            case FL_KEYDOWN:
                int k = Fl::event_key();
                if (k == FL_Up && depth < 7) { depth++; redraw(); }
                else if (k == FL_Down && depth > 0) { depth--; redraw(); }
                else if (k == 'f') { show_float = !show_float; redraw(); }
                else if (k == 's') { side_by_side = !side_by_side; redraw(); }
                else if (k == 'r') { zoom = 1.0; offset_x = 0; offset_y = 0; auto_zoom = false; auto_zoom_factor = 1.0; redraw(); }
                else if (k == 'a') {
                    auto_zoom = !auto_zoom;
                    if (auto_zoom) { auto_zoom_factor = 1.0; Fl::add_timeout(0.05, auto_zoom_callback, this); }
                    else Fl::remove_timeout(auto_zoom_callback, this);
                    redraw();
                }
                return 1;
        }
        return Fl_Widget::handle(event);
    }
};

void auto_zoom_callback(void* data) {
    SnowflakeWidget* w = (SnowflakeWidget*)data;
    if (w->auto_zoom) {
        w->auto_zoom_factor *= 1.05;
        if (w->auto_zoom_factor > 1e18) w->auto_zoom_factor = 1.0;
        w->redraw();
        Fl::repeat_timeout(0.05, auto_zoom_callback, data);
    }
}

int main() {
    Fl_Double_Window* win = new Fl_Double_Window(1000, 800, "Snowflake Precision: Exact vs Float");
    SnowflakeWidget* sw = new SnowflakeWidget(0, 0, 1000, 800);
    win->resizable(sw);
    win->show();
    Fl::focus(sw);
    return Fl::run();
}
