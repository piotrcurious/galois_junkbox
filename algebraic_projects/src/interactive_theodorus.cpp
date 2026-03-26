#include "../include/algebraic_field.hpp"
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/fl_draw.H>
#include <vector>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <sstream>

class TheodorusWidget : public Fl_Widget {
public:
    double zoom;
    double offset_x, offset_y;
    int steps;
    bool auto_zoom;
    int mouse_x, mouse_y;
    bool show_glass;

    TheodorusWidget(int X, int Y, int W, int H) : Fl_Widget(X, Y, W, H), zoom(40.0), offset_x(0), offset_y(0), steps(45), auto_zoom(false), mouse_x(0), mouse_y(0), show_glass(false) {}

    void draw() override {
        fl_push_clip(x(), y(), w(), h());
        fl_color(FL_BLACK);
        fl_rectf(x(), y(), w(), h());

        double cx = x() + w() / 2.0 + offset_x;
        double cy = y() + h() / 2.0 + offset_y;

        struct Point { double x, y; };
        std::vector<Point> alg_pts, flt_pts;

        // Construction
        {
            double angle = 0.0;
            alg_pts.push_back({0, 0});
            alg_pts.push_back({1.0, 0});
            double fx = 1.0, fy = 0;
            flt_pts.push_back({0, 0});
            flt_pts.push_back({1.0, 0});

            for(int n = 1; n <= steps; ++n) {
                angle += std::atan(1.0 / std::sqrt(static_cast<double>(n)));
                double r = std::sqrt(static_cast<double>(n+1));
                alg_pts.push_back({r * std::cos(angle), -r * std::sin(angle)});

                double r_curr = std::sqrt(fx*fx + fy*fy);
                double nx = -fy / r_curr, ny =  fx / r_curr;
                fx += nx; fy += ny;
                flt_pts.push_back({fx, fy});
            }
        }

        if (auto_zoom) {
            cx = x() + w()/2.0 - alg_pts.back().x * zoom;
            cy = y() + h()/2.0 - alg_pts.back().y * zoom;
        }

        // Draw paths
        auto draw_path = [&](const std::vector<Point>& pts, Fl_Color col, int width) {
            fl_color(col); fl_line_style(FL_SOLID, width);
            for(size_t i = 0; i < pts.size() - 1; ++i) {
                fl_line(cx + pts[i].x * zoom, cy + pts[i].y * zoom,
                        cx + pts[i+1].x * zoom, cy + pts[i+1].y * zoom);
            }
        };

        draw_path(flt_pts, FL_RED, 1);
        draw_path(alg_pts, FL_GREEN, 2);
        fl_line_style(0);

        // Magnifying Glass
        if (show_glass) {
            int gx = mouse_x, gy = mouse_y, gr = 100;
            fl_push_clip(gx - gr, gy - gr, 2*gr, 2*gr);
            fl_color(FL_BLACK); fl_rectf(gx - gr, gy - gr, 2*gr, 2*gr);

            double g_zoom = zoom * 1000.0;
            // Target point in model space under the glass
            double tx = (gx - cx) / zoom;
            double ty = (gy - cy) / zoom;

            double gcx = gx - tx * g_zoom;
            double gcy = gy - ty * g_zoom;

            // Draw paths in glass
            fl_color(FL_RED);
            for(size_t i = 0; i < flt_pts.size() - 1; ++i) {
                fl_line(gcx + flt_pts[i].x * g_zoom, gcy + flt_pts[i].y * g_zoom,
                        gcx + flt_pts[i+1].x * g_zoom, gcy + flt_pts[i+1].y * g_zoom);
            }
            fl_color(FL_GREEN);
            for(size_t i = 0; i < alg_pts.size() - 1; ++i) {
                fl_line(gcx + alg_pts[i].x * g_zoom, gcy + alg_pts[i].y * g_zoom,
                        gcx + alg_pts[i+1].x * g_zoom, gcy + alg_pts[i+1].y * g_zoom);
            }

            fl_pop_clip();
            fl_color(FL_WHITE); fl_circle(gx, gy, gr);
            fl_draw("1000x Glass", gx - 30, gy + gr + 15);
        }

        // Stats
        double tip_err = std::sqrt(std::pow(alg_pts.back().x - flt_pts.back().x, 2) + std::pow(alg_pts.back().y - flt_pts.back().y, 2));
        fl_color(FL_WHITE); fl_font(FL_HELVETICA, 14);
        fl_draw("Spiral of Theodorus: Precision Analysis", x()+20, y()+30);
        fl_draw(("Steps: " + std::to_string(steps) + " (Up/Down)").c_str(), x()+20, y()+50);
        fl_draw(("Normalized Tip Error: " + std::to_string(tip_err)).c_str(), x()+20, y()+70);
        fl_draw("G: Toggle 1000x Magnifying Glass, A: Auto-Follow Tip", x()+20, y()+90);

        fl_pop_clip();
    }

    int handle(int event) override {
        switch (event) {
            case FL_FOCUS: return 1;
            case FL_UNFOCUS: return 1;
            case FL_MOVE:
                mouse_x = Fl::event_x(); mouse_y = Fl::event_y();
                if (show_glass) redraw();
                return 1;
            case FL_PUSH:
                Fl::focus(this);
                return 1;
            case FL_DRAG:
                offset_x += (Fl::event_x() - mouse_x);
                offset_y += (Fl::event_y() - mouse_y);
                mouse_x = Fl::event_x(); mouse_y = Fl::event_y();
                redraw();
                return 1;
            case FL_MOUSEWHEEL:
                if (Fl::event_dy() < 0) zoom *= 1.1;
                else zoom /= 1.1;
                redraw();
                return 1;
            case FL_KEYDOWN:
                int k = Fl::event_key();
                if (k == FL_Up) { steps += 10; redraw(); }
                else if (k == FL_Down && steps > 10) { steps -= 10; redraw(); }
                else if (k == 'a') { auto_zoom = !auto_zoom; redraw(); }
                else if (k == 'g') { show_glass = !show_glass; redraw(); }
                return 1;
        }
        return Fl_Widget::handle(event);
    }
};

int main() {
    Fl_Double_Window* win = new Fl_Double_Window(1000, 800, "Theodorus Precision: Magnifying Glass");
    TheodorusWidget* tw = new TheodorusWidget(0, 0, 1000, 800);
    win->resizable(tw);
    win->show();
    Fl::focus(tw);
    return Fl::run();
}
