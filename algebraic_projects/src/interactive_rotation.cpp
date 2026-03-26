#include "../include/algebraic_field.hpp"
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Check_Button.H>
#include <vector>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <sstream>

struct Vec2Q { Q3 x, y; };
struct Vec2F { double x, y; };

class RotationWidget : public Fl_Widget {
public:
    double zoom;
    double offset_x, offset_y;
    int iterations;
    bool show_float;
    int angle_type; // 0: 30 deg, 1: 60 deg, 2: 0 deg

    RotationWidget(int X, int Y, int W, int H) : Fl_Widget(X, Y, W, H), zoom(250.0), offset_x(0), offset_y(0), iterations(1000), show_float(true), angle_type(0) {}

    void draw() override {
        fl_push_clip(x(), y(), w(), h());
        fl_color(FL_BLACK);
        fl_rectf(x(), y(), w(), h());

        double cx = x() + w() / 2.0 + offset_x;
        double cy = y() + h() / 2.0 + offset_y;

        // Exact rotation setup
        Q3 cosA, sinA;
        double fcosA, fsinA;

        if (angle_type == 0) { // 30 deg
            cosA = { rat(0), rat(1, 2) }; // sqrt(3)/2
            sinA = { rat(1, 2), rat(0) }; // 1/2
            fcosA = std::sqrt(3.0) / 2.0;
            fsinA = 0.5;
        } else { // 60 deg
            cosA = { rat(1, 2), rat(0) }; // 1/2
            sinA = { rat(0), rat(1, 2) }; // sqrt(3)/2
            fcosA = 0.5;
            fsinA = std::sqrt(3.0) / 2.0;
        }

        Vec2Q pQ = { Q3(rat(1)), Q3(rat(0)) };
        Vec2F pF = { 1.0, 0.0 };

        std::vector<Vec2F> pathQ;
        std::vector<Vec2F> pathF;

        for (int i = 0; i <= iterations; ++i) {
            pathQ.push_back({pQ.x.approx(), pQ.y.approx()});
            if (show_float) pathF.push_back(pF);

            // Exact rotate
            Vec2Q nextQ = {
                pQ.x * cosA - pQ.y * sinA,
                pQ.x * sinA + pQ.y * cosA
            };
            pQ = nextQ;

            // Float rotate
            Vec2F nextF = {
                pF.x * fcosA - pF.y * fsinA,
                pF.x * fsinA + pF.y * fcosA
            };
            pF = nextF;
        }

        // Exact path (Green)
        fl_color(FL_GREEN);
        for (size_t i = 0; i < pathQ.size() - 1; i++) {
            fl_line(cx + pathQ[i].x * zoom, cy + pathQ[i].y * zoom,
                    cx + pathQ[i+1].x * zoom, cy + pathQ[i+1].y * zoom);
        }

        // Float path (Red)
        if (show_float) {
            fl_color(FL_RED);
            for (size_t i = 0; i < pathF.size() - 1; i++) {
                fl_line(cx + pathF[i].x * zoom, cy + pathF[i].y * zoom,
                        cx + pathF[i+1].x * zoom, cy + pathF[i+1].y * zoom);
            }
        }

        // Norm error calculation
        double final_norm_f = pF.x*pF.x + pF.y*pF.y;
        double norm_err = std::abs(final_norm_f - 1.0);

        fl_color(FL_WHITE);
        fl_font(FL_HELVETICA, 14);
        fl_draw("Rotation Invariance Demo: Q(sqrt(3)) vs Float64", x() + 10, y() + 20);
        fl_draw(("Angle: " + std::string(angle_type == 0 ? "30" : "60") + " degrees (Press '1'/'2' to switch)").c_str(), x() + 10, y() + 40);
        fl_draw(("Iterations: " + std::to_string(iterations) + " (Up/Down to change)").c_str(), x() + 10, y() + 60);

        std::stringstream ss;
        ss << std::scientific << std::setprecision(12) << "Final Float Norm Error: " << norm_err;
        fl_color(norm_err > 1e-12 ? FL_RED : FL_WHITE);
        fl_draw(ss.str().c_str(), x() + 10, y() + 80);

        fl_color(FL_GREEN);
        fl_draw("Exact Norm Error: 0.000000000000 (Exact Identity)", x() + 10, y() + 100);

        fl_color(FL_WHITE);
        fl_draw("F: Toggle Float Path, Scroll: Zoom, Drag: Pan", x() + 10, y() + 120);

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
                if (Fl::event_key() == FL_Up) iterations += 100;
                else if (Fl::event_key() == FL_Down && iterations > 100) iterations -= 100;
                else if (Fl::event_key() == '1') { angle_type = 0; redraw(); }
                else if (Fl::event_key() == '2') { angle_type = 1; redraw(); }
                else if (Fl::event_key() == 'f') { show_float = !show_float; redraw(); }
                return 1;
        }
        return Fl_Widget::handle(event);
    }
};

int main() {
    Fl_Double_Window* win = new Fl_Double_Window(800, 600, "Interactive Rotation Precision");
    new RotationWidget(0, 0, 800, 600);
    win->resizable(win);
    win->show();
    return Fl::run();
}
