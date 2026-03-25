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
    double zoom;
    double offset_x, offset_y;
    int steps;
public:
    TheodorusWidget(int X, int Y, int W, int H) : Fl_Widget(X, Y, W, H), zoom(40.0), offset_x(0), offset_y(0), steps(45) {}

    void draw() override {
        fl_push_clip(x(), y(), w(), h());
        fl_color(FL_BLACK);
        fl_rectf(x(), y(), w(), h());

        double cx = x() + w() / 2.0 + offset_x;
        double cy = y() + h() / 2.0 + offset_y;

        // Exact points on the Spiral of Theodorus are (sqrt(n), atan(1/sqrt(n))) in polar
        // Here we draw the exact spiral in Green and float in Red

        struct Point { double x, y; };
        std::vector<Point> alg_pts, flt_pts;

        // Points on the Spiral of Theodorus are (sqrt(n), theta_n)
        // theta_n = sum_{k=1}^{n-1} atan(1/sqrt(k))
        {
            double angle = 0.0;
            alg_pts.push_back({cx, cy});
            alg_pts.push_back({cx + zoom, cy});
            for(int n = 1; n <= steps; ++n) {
                // Here we use Q2 to represent the exact squared-radius relationship.
                // Q2(rat(n+1)) represents the squared radius.
                Q2 r_sq(rat(n+1));
                double r = std::sqrt(r_sq.approx()); // radius is sqrt(n+1)

                angle += std::atan(1.0 / std::sqrt(static_cast<double>(n)));
                alg_pts.push_back({cx + zoom * r * std::cos(angle), cy - zoom * r * std::sin(angle)});
            }
        }

        // Floating point construction (accumulated normals)
        {
            double fx = cx + zoom, fy = cy;
            flt_pts.push_back({cx, cy});
            flt_pts.push_back({fx, fy});
            for(int n = 1; n <= steps; ++n) {
                double vx = fx - cx, vy = fy - cy;
                double r_curr = std::sqrt(vx*vx + vy*vy);
                double nx = -vy / r_curr, ny =  vx / r_curr;
                fx += nx * (zoom / 40.0); // scale normal to zoom
                fy += ny * (zoom / 40.0);
                flt_pts.push_back({fx, fy});
            }
        }

        // Draw float spiral (Red)
        fl_color(FL_RED);
        for(size_t i = 1; i < flt_pts.size() - 1; ++i) {
            fl_line(flt_pts[i].x, flt_pts[i].y, flt_pts[i+1].x, flt_pts[i+1].y);
        }

        // Draw exact spiral (Green)
        fl_color(FL_GREEN);
        fl_line_style(FL_SOLID, 2);
        for(size_t i = 1; i < alg_pts.size() - 1; ++i) {
            fl_line(alg_pts[i].x, alg_pts[i].y, alg_pts[i+1].x, alg_pts[i+1].y);
        }
        fl_line_style(0);

        fl_color(FL_WHITE);
        fl_font(FL_HELVETICA, 14);
        fl_draw("Spiral of Theodorus: Green (Exact Radius) vs Red (Float Normal Accumulation)", x()+10, y()+20);
        fl_draw("Scroll to Zoom, Drag to Pan. Up/Down to change steps.", x()+10, y()+40);
        fl_draw(("Steps: " + std::to_string(steps)).c_str(), x()+10, y()+60);

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
                if (Fl::event_key() == FL_Up) steps++;
                else if (Fl::event_key() == FL_Down && steps > 1) steps--;
                redraw();
                return 1;
        }
        return Fl_Widget::handle(event);
    }
};

int main() {
    Fl_Double_Window* win = new Fl_Double_Window(800, 600, "Interactive Spiral of Theodorus");
    new TheodorusWidget(0, 0, 800, 600);
    win->resizable(win);
    win->show();
    return Fl::run();
}
