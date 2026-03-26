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

    TheodorusWidget(int X, int Y, int W, int H) : Fl_Widget(X, Y, W, H), zoom(40.0), offset_x(0), offset_y(0), steps(45), auto_zoom(false) {}

    void draw() override {
        fl_push_clip(x(), y(), w(), h());
        fl_color(FL_BLACK);
        fl_rectf(x(), y(), w(), h());

        double cx = x() + w() / 2.0 + offset_x;
        double cy = y() + h() / 2.0 + offset_y;

        struct Point { double x, y; };
        std::vector<Point> alg_pts, flt_pts;

        // Exact-radial construction
        {
            double angle = 0.0;
            alg_pts.push_back({0, 0});
            alg_pts.push_back({zoom, 0});
            for(int n = 1; n <= steps; ++n) {
                angle += std::atan(1.0 / std::sqrt(static_cast<double>(n)));
                double r = zoom * std::sqrt(static_cast<double>(n+1));
                alg_pts.push_back({r * std::cos(angle), -r * std::sin(angle)});
            }
        }

        // Floating point accumulation construction
        {
            double fx = zoom, fy = 0;
            flt_pts.push_back({0, 0});
            flt_pts.push_back({fx, fy});
            for(int n = 1; n <= steps; ++n) {
                double r_curr = std::sqrt(fx*fx + fy*fy);
                double nx = -fy / r_curr, ny =  fx / r_curr;
                fx += nx * zoom;
                fy += ny * zoom;
                flt_pts.push_back({fx, fy});
            }
        }

        if (auto_zoom) {
            cx = x() + w()/2.0 - alg_pts.back().x;
            cy = y() + h()/2.0 - alg_pts.back().y;
        }

        // Draw float spiral (Red)
        fl_color(FL_RED);
        for(size_t i = 1; i < flt_pts.size() - 1; ++i) {
            fl_line(cx + flt_pts[i].x, cy + flt_pts[i].y,
                    cx + flt_pts[i+1].x, cy + flt_pts[i+1].y);
        }

        // Draw exact-radial spiral (Green)
        fl_color(FL_GREEN);
        fl_line_style(FL_SOLID, 2);
        for(size_t i = 1; i < alg_pts.size() - 1; ++i) {
            fl_line(cx + alg_pts[i].x, cy + alg_pts[i].y,
                    cx + alg_pts[i+1].x, cy + alg_pts[i+1].y);
        }
        fl_line_style(0);

        // Error analysis at the tip
        double tip_err = std::sqrt(std::pow(alg_pts.back().x - flt_pts.back().x, 2) + std::pow(alg_pts.back().y - flt_pts.back().y, 2)) / zoom;

        fl_color(FL_WHITE);
        fl_font(FL_HELVETICA, 14);
        fl_draw("Spiral of Theodorus: GREEN (Exact Radial) vs RED (Float Accumulation)", x()+10, y()+20);
        fl_draw(("Steps: " + std::to_string(steps) + " (Up/Down to change)").c_str(), x()+10, y()+40);

        std::stringstream ss;
        ss << std::fixed << std::setprecision(10) << "Normalized Tip Error: " << tip_err;
        fl_draw(ss.str().c_str(), x()+10, y()+60);

        fl_draw("Scroll: Zoom, Drag: Pan, A: Toggle Tip Auto-Follow", x()+10, y()+80);

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
                if (Fl::event_dy() < 0) zoom *= 1.1;
                else zoom /= 1.1;
                redraw();
                return 1;
            case FL_KEYDOWN:
                if (Fl::event_key() == FL_Up) steps += 10;
                else if (Fl::event_key() == FL_Down && steps > 10) steps -= 10;
                else if (Fl::event_key() == 'a') auto_zoom = !auto_zoom;
                redraw();
                return 1;
        }
        return Fl_Widget::handle(event);
    }
};

int main() {
    Fl_Double_Window* win = new Fl_Double_Window(800, 600, "Theodorus Precision Analysis");
    new TheodorusWidget(0, 0, 800, 600);
    win->resizable(win);
    win->show();
    return Fl::run();
}
