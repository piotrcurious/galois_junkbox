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

void rotation_timeout_callback(void* data);

class RotationWidget : public Fl_Widget {
public:
    double zoom;
    double offset_x, offset_y;
    int iterations;
    bool show_float;
    int angle_type;
    bool auto_rotate;
    std::vector<double> error_history;

    RotationWidget(int X, int Y, int W, int H) : Fl_Widget(X, Y, W, H), zoom(200.0), offset_x(150), offset_y(0), iterations(1000),
                                                   show_float(true), angle_type(0), auto_rotate(false) {}

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
            cosA = { rat(0), rat(1, 2) }; sinA = { rat(1, 2), rat(0) };
            fcosA = std::sqrt(3.0) / 2.0; fsinA = 0.5;
        } else { // 60 deg
            cosA = { rat(1, 2), rat(0) }; sinA = { rat(0), rat(1, 2) };
            fcosA = 0.5; fsinA = std::sqrt(3.0) / 2.0;
        }

        Vec2Q pQ = { Q3(rat(1)), Q3(rat(0)) };
        Vec2F pF = { 1.0, 0.0 };

        std::vector<Vec2F> pathQ;
        std::vector<Vec2F> pathF;
        error_history.clear();

        for (int i = 0; i <= iterations; ++i) {
            pathQ.push_back({pQ.x.approx(), pQ.y.approx()});
            if (show_float) pathF.push_back(pF);

            double norm_f = pF.x*pF.x + pF.y*pF.y;
            error_history.push_back(std::abs(norm_f - 1.0));

            // Exact rotate
            Vec2Q nextQ = { pQ.x * cosA - pQ.y * sinA, pQ.x * sinA + pQ.y * cosA };
            pQ = nextQ;

            // Float rotate
            Vec2F nextF = { pF.x * fcosA - pF.y * fsinA, pF.x * fsinA + pF.y * fcosA };
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

        // Error Plot Panel
        int pw = 300, ph = 200;
        int px = x() + 20, py = y() + h() - ph - 20;
        fl_color(FL_DARK3);
        fl_rectf(px, py, pw, ph);
        fl_color(FL_WHITE);
        fl_rect(px, py, pw, ph);
        fl_draw("LOG10(Norm Error) over Time", px+5, py-5);

        if (!error_history.empty()) {
            fl_color(FL_RED);
            for (size_t i = 1; i < error_history.size(); i += iterations/100 + 1) {
                double e0 = error_history[i-1] > 1e-20 ? std::log10(error_history[i-1]) : -20.0;
                double e1 = error_history[i] > 1e-20 ? std::log10(error_history[i]) : -20.0;

                auto to_plot = [&](int idx, double val) -> std::pair<int, int> {
                    return { px + (int)((double)idx/iterations * pw),
                             py + ph - (int)((val + 20.0)/20.0 * ph) };
                };
                auto [x0, y0] = to_plot(i-1, e0);
                auto [x1, y1] = to_plot(i, e1);
                fl_line(x0, y0, x1, y1);
            }
        }

        // Overlay text
        fl_color(FL_WHITE); fl_font(FL_HELVETICA, 14);
        fl_draw("Rotation Invariance: Q(sqrt(3)) vs Float64", x() + 20, y() + 30);
        fl_draw(("Iterations: " + std::to_string(iterations)).c_str(), x() + 20, y() + 50);
        fl_draw("1/2: Angle, F: Toggle Float, A: Auto-Rotate, Scroll: Zoom", x() + 20, y() + 70);

        std::stringstream ss;
        ss << std::scientific << std::setprecision(10) << "Current Norm Error: " << error_history.back();
        fl_color(error_history.back() > 1e-12 ? FL_RED : FL_WHITE);
        fl_draw(ss.str().c_str(), x() + 20, y() + 100);

        fl_color(FL_GREEN);
        fl_draw("Exact Norm Error: 0.0000000000e+00", x() + 20, y() + 120);

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
                if (k == FL_Up) { iterations += 100; redraw(); }
                else if (k == FL_Down && iterations > 100) { iterations -= 100; redraw(); }
                else if (k == '1') { angle_type = 0; redraw(); }
                else if (k == '2') { angle_type = 1; redraw(); }
                else if (k == 'f') { show_float = !show_float; redraw(); }
                else if (k == 'a') {
                    auto_rotate = !auto_rotate;
                    if (auto_rotate) Fl::add_timeout(0.05, rotation_timeout_callback, this);
                    else Fl::remove_timeout(rotation_timeout_callback, this);
                    redraw();
                }
                return 1;
        }
        return Fl_Widget::handle(event);
    }
};

void rotation_timeout_callback(void* data) {
    RotationWidget* w = (RotationWidget*)data;
    if (w->auto_rotate) {
        w->iterations += 5;
        if (w->iterations > 20000) w->iterations = 1000;
        w->redraw();
        Fl::repeat_timeout(0.02, rotation_timeout_callback, data);
    }
}

int main() {
    Fl_Double_Window* win = new Fl_Double_Window(1000, 800, "Rotation Precision Analysis");
    RotationWidget* rw = new RotationWidget(0, 0, 1000, 800);
    win->resizable(rw);
    win->show();
    Fl::focus(rw);
    return Fl::run();
}
