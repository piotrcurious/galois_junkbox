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

struct Vec2Q { Q3 x, y; };
struct Vec2D { double x, y; };

void rotation_timeout_callback(void* data);

class RotationWidget : public Fl_Widget {
public:
    double zoom;
    double offset_x, offset_y;
    int iterations;
    bool show_float;
    int angle_type;
    bool auto_rotate;
    PrecisionType current_precision;
    std::vector<double> error_history;

    RotationWidget(int X, int Y, int W, int H) : Fl_Widget(X, Y, W, H), zoom(200.0), offset_x(150), offset_y(0), iterations(1000),
                                                   show_float(true), angle_type(0), auto_rotate(false), current_precision(PRE_DOUBLE) {}

    template<typename T>
    void compute_paths(std::vector<Vec2D>& pathQ, std::vector<Vec2D>& pathF, double& final_err) {
        Q3 cosA, sinA;
        T fcosA, fsinA;
        if (angle_type == 0) { // 30 deg
            cosA = { rat(0), rat(1, 2) }; sinA = { rat(1, 2), rat(0) };
            fcosA = (T)(std::sqrt(3.0) / 2.0); fsinA = (T)0.5;
        } else { // 60 deg
            cosA = { rat(1, 2), rat(0) }; sinA = { rat(0), rat(1, 2) };
            fcosA = (T)0.5; fsinA = (T)(std::sqrt(3.0) / 2.0);
        }

        Vec2Q pQ = { Q3(rat(1)), Q3(rat(0)) };
        struct { T x, y; } pF = { (T)1.0, (T)0.0 };

        error_history.clear();
        for (int i = 0; i <= iterations; ++i) {
            pathQ.push_back({pQ.x.approx(), pQ.y.approx()});
            pathF.push_back({(double)pF.x, (double)pF.y});

            T norm_f = pF.x*pF.x + pF.y*pF.y;
            error_history.push_back(std::abs((double)norm_f - 1.0));

            // Exact
            Vec2Q nextQ = { pQ.x * cosA - pQ.y * sinA, pQ.x * sinA + pQ.y * cosA };
            pQ = nextQ;
            // Float
            T nfx = pF.x * fcosA - pF.y * fsinA;
            T nfy = pF.x * fsinA + pF.y * fcosA;
            pF.x = nfx; pF.y = nfy;
        }
        final_err = error_history.back();
    }

    void draw() override {
        fl_push_clip(x(), y(), w(), h());
        fl_color(FL_BLACK);
        fl_rectf(x(), y(), w(), h());

        double cx = x() + w() / 2.0 + offset_x;
        double cy = y() + h() / 2.0 + offset_y;

        std::vector<Vec2D> pathQ, pathF;
        double final_err = 0;
        if (current_precision == PRE_FLOAT) compute_paths<float>(pathQ, pathF, final_err);
        else compute_paths<double>(pathQ, pathF, final_err);

        // Paths
        fl_color(FL_GREEN);
        for (size_t i = 0; i < pathQ.size() - 1; i++) fl_line(cx + pathQ[i].x * zoom, cy + pathQ[i].y * zoom, cx + pathQ[i+1].x * zoom, cy + pathQ[i+1].y * zoom);

        if (show_float) {
            fl_color(FL_RED);
            for (size_t i = 0; i < pathF.size() - 1; i++) fl_line(cx + pathF[i].x * zoom, cy + pathF[i].y * zoom, cx + pathF[i+1].x * zoom, cy + pathF[i+1].y * zoom);
        }

        // Plot
        int pw = 300, ph = 200, px = x() + 20, py = y() + h() - ph - 20;
        fl_color(FL_DARK3); fl_rectf(px, py, pw, ph);
        fl_color(FL_WHITE); fl_rect(px, py, pw, ph);
        fl_draw("LOG10(Norm Error)", px+5, py-5);
        if (!error_history.empty()) {
            fl_color(FL_RED);
            int step = iterations / 100 + 1;
            for (size_t i = 1; i < error_history.size(); i += step) {
                double e0 = error_history[i-1] > 1e-20 ? std::log10(error_history[i-1]) : -20.0;
                double e1 = error_history[i] > 1e-20 ? std::log10(error_history[i]) : -20.0;
                auto to_p = [&](int idx, double val) -> std::pair<int, int> {
                    return { px + (int)((double)idx/iterations * pw), py + ph - (int)((val + 20.0)/20.0 * ph) };
                };
                auto [x0, y0] = to_p(i-1, e0); auto [x1, y1] = to_p(i, e1);
                fl_line(x0, y0, x1, y1);
            }
        }

        fl_color(FL_WHITE); fl_font(FL_HELVETICA, 14);
        fl_draw("Rotation Precision Demo", x() + 20, y() + 30);
        fl_draw(("Iterations: " + std::to_string(iterations)).c_str(), x() + 20, y() + 50);
        fl_draw(("Precision: " + std::string(current_precision == PRE_FLOAT ? "float32" : "float64") + " (P to toggle)").c_str(), x() + 20, y() + 70);

        std::stringstream ss; ss << std::scientific << std::setprecision(10) << "Final Norm Error: " << final_err;
        fl_color(final_err > 1e-12 ? FL_RED : FL_WHITE); fl_draw(ss.str().c_str(), x() + 20, y() + 100);
        fl_color(FL_GREEN); fl_draw("Exact Norm Error: 0.0000000000e+00", x() + 20, y() + 120);

        fl_pop_clip();
    }

    int handle(int event) override {
        static int last_x, last_y;
        switch (event) {
            case FL_FOCUS: return 1;
            case FL_PUSH: Fl::focus(this); last_x = Fl::event_x(); last_y = Fl::event_y(); return 1;
            case FL_DRAG: offset_x += (Fl::event_x() - last_x); offset_y += (Fl::event_y() - last_y); last_x = Fl::event_x(); last_y = Fl::event_y(); redraw(); return 1;
            case FL_MOUSEWHEEL: if (Fl::event_dy() < 0) zoom *= 1.2; else zoom /= 1.2; redraw(); return 1;
            case FL_KEYDOWN:
                int k = Fl::event_key();
                if (k == FL_Up) { iterations += 100; redraw(); }
                else if (k == FL_Down && iterations > 100) { iterations -= 100; redraw(); }
                else if (k == '1') { angle_type = 0; redraw(); }
                else if (k == '2') { angle_type = 1; redraw(); }
                else if (k == 'f') { show_float = !show_float; redraw(); }
                else if (k == 'p') { current_precision = (current_precision == PRE_FLOAT ? PRE_DOUBLE : PRE_FLOAT); redraw(); }
                else if (k == 'a') { auto_rotate = !auto_rotate; if (auto_rotate) Fl::add_timeout(0.05, rotation_timeout_callback, this); else Fl::remove_timeout(rotation_timeout_callback, this); redraw(); }
                return 1;
        }
        return Fl_Widget::handle(event);
    }
};

void rotation_timeout_callback(void* data) {
    RotationWidget* w = (RotationWidget*)data;
    if (w->auto_rotate) { w->iterations += 5; if (w->iterations > 20000) w->iterations = 1000; w->redraw(); Fl::repeat_timeout(0.02, rotation_timeout_callback, data); }
}

int main() {
    Fl_Double_Window* win = new Fl_Double_Window(1000, 800, "Rotation Precision Analysis: float vs double vs exact");
    RotationWidget* rw = new RotationWidget(0, 0, 1000, 800);
    win->resizable(rw); win->show(); Fl::focus(rw);
    return Fl::run();
}
