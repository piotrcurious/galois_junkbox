#include "../include/algebraic_field.hpp"
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/fl_draw.H>
#include <vector>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <sstream>

enum PrecisionType { PRE_FLOAT, PRE_DOUBLE };

class TheodorusWidget : public Fl_Widget {
public:
    double zoom;
    double offset_x, offset_y;
    int steps;
    bool auto_zoom;
    int mouse_x, mouse_y;
    bool show_glass;
    PrecisionType current_precision;

    TheodorusWidget(int X, int Y, int W, int H) : Fl_Widget(X, Y, W, H), zoom(40.0), offset_x(0), offset_y(0), steps(45), auto_zoom(false), mouse_x(0), mouse_y(0), show_glass(false), current_precision(PRE_DOUBLE) {}

    template<typename T>
    void construct_float_path(std::vector<std::pair<double, double>>& path) {
        T fx = (T)1.0, fy = (T)0.0;
        path.push_back({0, 0});
        path.push_back({1.0, 0});
        for(int n = 1; n <= steps; ++n) {
            T r_curr = std::sqrt(fx*fx + fy*fy);
            T nx = -fy / r_curr, ny =  fx / r_curr;
            fx += nx; fy += ny;
            path.push_back({(double)fx, (double)fy});
        }
    }

    void draw() override {
        fl_push_clip(x(), y(), w(), h());
        fl_color(FL_BLACK);
        fl_rectf(x(), y(), w(), h());

        double cx = x() + w() / 2.0 + offset_x;
        double cy = y() + h() / 2.0 + offset_y;

        struct Point { double x, y; };
        std::vector<Point> alg_pts;
        std::vector<std::pair<double, double>> flt_pts_raw;

        // Exact Construction
        {
            double angle = 0.0;
            alg_pts.push_back({0, 0});
            alg_pts.push_back({1.0, 0});
            for(int n = 1; n <= steps; ++n) {
                angle += std::atan(1.0 / std::sqrt(static_cast<double>(n)));
                double r = std::sqrt(static_cast<double>(n+1));
                alg_pts.push_back({r * std::cos(angle), -r * std::sin(angle)});
            }
        }

        // Floating point construction
        if (current_precision == PRE_FLOAT) construct_float_path<float>(flt_pts_raw);
        else construct_float_path<double>(flt_pts_raw);

        std::vector<Point> flt_pts;
        for(auto& p : flt_pts_raw) flt_pts.push_back({p.first, p.second});

        if (auto_zoom) {
            cx = x() + w()/2.0 - alg_pts.back().x * zoom;
            cy = y() + h()/2.0 - alg_pts.back().y * zoom;
        }

        auto draw_paths_internal = [&](double center_x, double center_y, double current_zoom) {
            // Float (Red)
            fl_color(FL_RED); fl_line_style(FL_SOLID, 1);
            for(size_t i = 0; i < flt_pts.size() - 1; ++i) {
                fl_line(center_x + flt_pts[i].x * current_zoom, center_y + flt_pts[i].y * current_zoom,
                        center_x + flt_pts[i+1].x * current_zoom, center_y + flt_pts[i+1].y * current_zoom);
            }
            // Exact (Green)
            fl_color(FL_GREEN); fl_line_style(FL_SOLID, 2);
            for(size_t i = 0; i < alg_pts.size() - 1; ++i) {
                fl_line(center_x + alg_pts[i].x * current_zoom, center_y + alg_pts[i].y * current_zoom,
                        center_x + alg_pts[i+1].x * current_zoom, center_y + alg_pts[i+1].y * current_zoom);
            }
            fl_line_style(0);
        };

        draw_paths_internal(cx, cy, zoom);

        // Magnifying Glass
        if (show_glass) {
            int gx = mouse_x, gy = mouse_y, gr = 120;
            fl_push_clip(gx - gr, gy - gr, 2*gr, 2*gr);
            fl_color(FL_BLACK); fl_rectf(gx - gr, gy - gr, 2*gr, 2*gr);

            double g_zoom_mult = 500.0;
            // Bug fix: The glass should zoom into the spot where the mouse IS,
            // relative to the main coordinate system.
            // Model coordinates under mouse:
            double mx_model = (mouse_x - cx) / zoom;
            double my_model = (mouse_y - cy) / zoom;

            // In the glass, we want mx_model to be at (mouse_x, mouse_y)
            double glass_cx = mouse_x - mx_model * (zoom * g_zoom_mult);
            double glass_cy = mouse_y - my_model * (zoom * g_zoom_mult);

            draw_paths_internal(glass_cx, glass_cy, zoom * g_zoom_mult);

            fl_pop_clip();
            fl_color(FL_WHITE); fl_arc(gx - gr, gy - gr, 2*gr, 2*gr, 0, 360);
            fl_draw("500x Glass", gx - 30, gy + gr + 15);
        }

        // Stats
        double tip_err = std::sqrt(std::pow(alg_pts.back().x - flt_pts.back().x, 2) + std::pow(alg_pts.back().y - flt_pts.back().y, 2));
        fl_color(FL_WHITE); fl_font(FL_HELVETICA, 14);
        fl_draw("Spiral of Theodorus: Precision Analysis", x()+20, y()+30);
        fl_draw(("Steps: " + std::to_string(steps) + " (Up/Down)").c_str(), x()+20, y()+50);
        fl_draw(("Precision: " + std::string(current_precision == PRE_FLOAT ? "32-bit float" : "64-bit double") + " (P to toggle)").c_str(), x()+20, y()+70);
        fl_draw(("Tip Discrepancy: " + std::to_string(tip_err)).c_str(), x()+20, y()+90);
        fl_draw("G: Toggle Glass, A: Toggle Tip Follow, Drag: Pan, Scroll: Zoom", x()+20, y()+110);

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
                mouse_x = Fl::event_x(); mouse_y = Fl::event_y();
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
                else if (k == 'p') { current_precision = (current_precision == PRE_FLOAT ? PRE_DOUBLE : PRE_FLOAT); redraw(); }
                return 1;
        }
        return Fl_Widget::handle(event);
    }
};

int main() {
    Fl_Double_Window* win = new Fl_Double_Window(1000, 800, "Theodorus Precision: Multi-Level Comparison");
    TheodorusWidget* tw = new TheodorusWidget(0, 0, 1000, 800);
    win->resizable(tw);
    win->show();
    Fl::focus(tw);
    return Fl::run();
}
