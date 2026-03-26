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

    SnowflakeWidget(int X, int Y, int W, int H) : Fl_Widget(X, Y, W, H), zoom(1.0), offset_x(0), offset_y(0), depth(4), show_float(true), auto_zoom(false), auto_zoom_factor(1.0) {}

    void draw() override {
        fl_push_clip(x(), y(), w(), h());
        fl_color(FL_BLACK);
        fl_rectf(x(), y(), w(), h());

        double cx = x() + w() / 2.0 + offset_x;
        double cy = y() + h() / 2.0 + offset_y;

        // Exact setup
        Vec2Q p1q = { Q3(rat(-300)), Q3(rat(0), rat(150))  };
        Vec2Q p2q = { Q3(rat(300)),  Q3(rat(0), rat(150))  };
        Vec2Q p3q = { Q3(rat(0)),    Q3(rat(0), rat(-150)) };
        std::vector<Vec2Q> ptsQ = { p1q, p3q, p2q, p1q };

        // Float setup
        Vec2F p1f = { -300.0, 150.0 * std::sqrt(3.0) };
        Vec2F p2f = {  300.0, 150.0 * std::sqrt(3.0) };
        Vec2F p3f = {    0.0, -150.0 * std::sqrt(3.0) };
        std::vector<Vec2F> ptsF = { p1f, p3f, p2f, p1f };

        auto stepQ = [](std::vector<Vec2Q>& points) {
            std::vector<Vec2Q> next;
            Q3 one3 = Q3(rat(1, 3));
            Q3 half = Q3(rat(1, 2));
            Q3 s3_6 = Q3(rat(0), rat(1, 6));
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
            const double inv3 = 1.0/3.0;
            const double s3_6 = std::sqrt(3.0)/6.0;
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

        for (int i = 0; i < depth; i++) {
            stepQ(ptsQ);
            if (show_float) stepF(ptsF);
        }

        double current_zoom = zoom * auto_zoom_factor;

        if (auto_zoom) {
            size_t target_idx = ptsQ.size() / 3;
            cx = x() + w()/2.0 - ptsQ[target_idx].x.approx() * current_zoom;
            cy = y() + h()/2.0 - ptsQ[target_idx].y.approx() * current_zoom;
        }

        // Exact (Cyan)
        fl_color(FL_CYAN);
        fl_line_style(FL_SOLID, 2);
        for (size_t i = 0; i < ptsQ.size() - 1; i++) {
            fl_line(cx + ptsQ[i].x.approx() * current_zoom, cy + ptsQ[i].y.approx() * current_zoom,
                    cx + ptsQ[i+1].x.approx() * current_zoom, cy + ptsQ[i+1].y.approx() * current_zoom);
        }

        // Float (Red)
        if (show_float) {
            fl_color(FL_RED);
            fl_line_style(FL_SOLID, 1);
            for (size_t i = 0; i < ptsF.size() - 1; i++) {
                fl_line(cx + ptsF[i].x * current_zoom, cy + ptsF[i].y * current_zoom,
                        cx + ptsF[i+1].x * current_zoom, cy + ptsF[i+1].y * current_zoom);
            }
        }
        fl_line_style(0);

        fl_color(FL_WHITE);
        fl_font(FL_HELVETICA, 14);
        fl_draw(("Zoom: " + std::to_string(current_zoom)).c_str(), x() + 10, y() + 20);
        fl_draw(("Depth: " + std::to_string(depth) + " (Up/Down to change)").c_str(), x() + 10, y() + 40);
        fl_draw("F: Toggle Float, A: Toggle Extreme Auto-Zoom, Scroll/Drag to Pan", x() + 10, y() + 60);
        if (auto_zoom) {
            fl_color(FL_YELLOW);
            fl_draw("AUTO-ZOOM ACTIVE: Watch the Red path drift as zoom increases!", x() + 10, y() + 80);
        }

        fl_pop_clip();
    }

    int handle(int event) override {
        static int last_x, last_y;
        switch (event) {
            case FL_PUSH:
                last_x = Fl::event_x(); last_y = Fl::event_y();
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
                if (Fl::event_key() == FL_Up && depth < 8) depth++;
                else if (Fl::event_key() == FL_Down && depth > 0) depth--;
                else if (Fl::event_key() == 'f') { show_float = !show_float; redraw(); }
                else if (Fl::event_key() == 'a') {
                    auto_zoom = !auto_zoom;
                    auto_zoom_factor = 1.0;
                    if (auto_zoom) Fl::add_timeout(0.05, auto_zoom_callback, this);
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
        w->auto_zoom_factor *= 1.1;
        if (w->auto_zoom_factor > 1e18) w->auto_zoom_factor = 1.0;
        w->redraw();
        Fl::repeat_timeout(0.05, auto_zoom_callback, data);
    }
}

int main() {
    Fl_Double_Window* win = new Fl_Double_Window(800, 600, "Advanced Interactive Snowflake");
    new SnowflakeWidget(0, 0, 800, 600);
    win->resizable(win);
    win->show();
    return Fl::run();
}
