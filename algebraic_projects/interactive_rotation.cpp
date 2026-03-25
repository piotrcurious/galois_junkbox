#include "algebraic_field.hpp"
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/fl_draw.H>
#include <vector>
#include <cmath>
#include <iostream>

struct Vec2Q { Q3 x, y; };
struct Vec2F { double x, y; };

class RotationWidget : public Fl_Widget {
    double zoom;
    double offset_x, offset_y;
    int iterations;
    bool show_float;
public:
    RotationWidget(int X, int Y, int W, int H) : Fl_Widget(X, Y, W, H), zoom(300.0), offset_x(0), offset_y(0), iterations(1000), show_float(true) {}

    void draw() override {
        fl_push_clip(x(), y(), w(), h());
        fl_color(FL_BLACK);
        fl_rectf(x(), y(), w(), h());

        double cx = x() + w() / 2.0 + offset_x;
        double cy = y() + h() / 2.0 + offset_y;

        Q3 cos30 = { rat(0), rat(1, 2) }; // sqrt(3)/2
        Q3 sin30 = { rat(1, 2), rat(0) }; // 1/2
        const double fcos30 = std::sqrt(3.0) / 2.0;
        const double fsin30 = 0.5;

        Vec2Q pQ = { Q3(rat(1)), Q3(rat(0)) };
        Vec2F pF = { 1.0, 0.0 };

        std::vector<Vec2F> pathQ;
        std::vector<Vec2F> pathF;

        for (int i = 0; i < iterations; ++i) {
            pathQ.push_back({pQ.x.approx(), pQ.y.approx()});
            pathF.push_back(pF);

            // Exact rotate
            Vec2Q nextQ = {
                pQ.x * cos30 - pQ.y * sin30,
                pQ.x * sin30 + pQ.y * cos30
            };
            pQ = nextQ;

            // Float rotate
            Vec2F nextF = {
                pF.x * fcos30 - pF.y * fsin30,
                pF.x * fsin30 + pF.y * fcos30
            };
            pF = nextF;
        }

        // Exact path
        fl_color(FL_GREEN);
        for (size_t i = 0; i < pathQ.size() - 1; i++) {
            fl_line(cx + pathQ[i].x * zoom, cy + pathQ[i].y * zoom,
                    cx + pathQ[i+1].x * zoom, cy + pathQ[i+1].y * zoom);
        }

        // Float path
        if (show_float) {
            fl_color(FL_RED);
            for (size_t i = 0; i < pathF.size() - 1; i++) {
                fl_line(cx + pathF[i].x * zoom, cy + pathF[i].y * zoom,
                        cx + pathF[i+1].x * zoom, cy + pathF[i+1].y * zoom);
            }
        }

        fl_color(FL_WHITE);
        fl_font(FL_HELVETICA, 14);
        fl_draw("Green: Exact Q(sqrt(3)), Red: Float64", x() + 10, y() + 20);
        fl_draw("Scroll to zoom, Drag to pan. Click to toggle float path.", x() + 10, y() + 40);
        fl_draw(("Iterations: " + std::to_string(iterations)).c_str(), x() + 10, y() + 60);

        fl_pop_clip();
    }

    int handle(int event) override {
        static int last_x, last_y;
        switch (event) {
            case FL_PUSH:
                last_x = Fl::event_x();
                last_y = Fl::event_y();
                show_float = !show_float;
                redraw();
                return 1;
            case FL_DRAG:
                offset_x += (Fl::event_x() - last_x);
                offset_y += (Fl::event_y() - last_y);
                last_x = Fl::event_x();
                last_y = Fl::event_y();
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
                redraw();
                return 1;
        }
        return Fl_Widget::handle(event);
    }
};

int main() {
    Fl_Double_Window* win = new Fl_Double_Window(800, 600, "Interactive Rotation Drift");
    new RotationWidget(0, 0, 800, 600);
    win->resizable(win);
    win->show();
    return Fl::run();
}
