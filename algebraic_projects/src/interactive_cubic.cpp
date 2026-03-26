#include "../include/algebraic_field.hpp"
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/fl_draw.H>
#include <complex>
#include <vector>
#include <string>
#include <iomanip>
#include <sstream>

class CubicRootWidget : public Fl_Widget {
public:
    double scale;
    double offset_x, offset_y;
    Qcbrt2 alpha, beta, gamma;
    int edit_mode; // 0: alpha, 1: beta
    int coeff_idx; // 0: a, 1: b, 2: c

    CubicRootWidget(int X, int Y, int W, int H) : Fl_Widget(X, Y, W, H), scale(150.0), offset_x(0), offset_y(0),
                                                   alpha(rat(1), rat(1), rat(0)), beta(rat(0), rat(1), rat(1)), edit_mode(0), coeff_idx(0) {
        gamma = alpha * beta;
    }

    void draw() override {
        fl_push_clip(x(), y(), w(), h());
        fl_color(FL_BLACK);
        fl_rectf(x(), y(), w(), h());

        double cx = x() + w() / 2.0 + offset_x;
        double cy = y() + h() / 2.0 + offset_y;

        // Grid lines
        fl_color(FL_LIGHT2);
        for(int i=-5; i<=5; ++i) {
            fl_line(cx + i*scale, y(), cx + i*scale, y()+h());
            fl_line(x(), cy + i*scale, x()+w(), cy + i*scale);
        }

        double rho = std::pow(2.0, 1.0/3.0);
        std::complex<double> r1(rho, 0);
        std::complex<double> r2(rho * -0.5, rho * std::sqrt(3.0)/2.0);
        std::complex<double> r3(rho * -0.5, rho * -std::sqrt(3.0)/2.0);

        // Circle for ∛2
        fl_color(FL_GRAY);
        fl_arc(cx - rho*scale, cy - rho*scale, 2*rho*scale, 2*rho*scale, 0, 360);

        auto draw_root = [&](std::complex<double> r, Fl_Color col, const char* lbl) {
            double rx = cx + r.real()*scale;
            double ry = cy - r.imag()*scale;
            fl_color(col);
            fl_pie(rx-5, ry-5, 10, 10, 0, 360);
            fl_color(FL_WHITE);
            fl_font(FL_HELVETICA, 14);
            fl_draw(lbl, rx+8, ry-8);
        };

        draw_root(r1, FL_GREEN, "∛2 (Real)");
        draw_root(r2, FL_MAGENTA, "ω∛2 (Complex)");
        draw_root(r3, FL_MAGENTA, "ω²∛2 (Complex)");

        // UI Panel
        fl_color(FL_BLACK);
        fl_rectf(x(), y(), 350, h());
        fl_color(FL_GRAY);
        fl_rect(x(), y(), 350, h());

        fl_color(FL_YELLOW);
        fl_font(FL_HELVETICA_BOLD, 18);
        fl_draw("Interactive Cubic Explorer", x()+20, y()+30);

        fl_color(FL_WHITE);
        fl_font(FL_HELVETICA, 14);
        int ty = y() + 60;

        auto draw_val = [&](const char* label, Qcbrt2 val, int mode) {
            fl_color(edit_mode == mode ? FL_CYAN : FL_WHITE);
            fl_draw((std::string(label) + " = " + val.str()).c_str(), x()+20, ty); ty+=20;
            fl_color(FL_WHITE);
            fl_draw(("N(" + std::string(label) + ") = " + val.norm().str()).c_str(), x()+20, ty); ty+=30;
        };

        draw_val("α", alpha, 0);
        draw_val("β", beta, 1);

        fl_color(FL_GREEN);
        fl_draw(("α · β = " + gamma.str()).c_str(), x()+20, ty); ty+=20;
        fl_draw(("N(α · β) = " + gamma.norm().str()).c_str(), x()+20, ty); ty+=30;

        bool check = (alpha.norm() * beta.norm() == gamma.norm());
        fl_color(check ? FL_GREEN : FL_RED);
        fl_draw(check ? "✓ Norm Multiplicativity EXACT" : "✗ Error in Norm Identity", x()+20, ty); ty+=40;

        fl_color(FL_WHITE);
        fl_draw("TAB: Switch α/β, Keys 1/2/3: coeff", x()+20, ty); ty+=20;
        fl_draw("Up/Down: Adjust coefficient", x()+20, ty); ty+=20;
        fl_draw("Drag: Pan, Scroll: Zoom", x()+20, ty);

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
                if (Fl::event_dy() < 0) scale *= 1.1;
                else scale /= 1.1;
                redraw();
                return 1;
            case FL_KEYDOWN: {
                int k = Fl::event_key();
                if (k == FL_Tab) edit_mode = 1 - edit_mode;
                else if (k == '1') coeff_idx = 0;
                else if (k == '2') coeff_idx = 1;
                else if (k == '3') coeff_idx = 2;
                else if (k == FL_Up || k == FL_Down) {
                    Qcbrt2 &target = (edit_mode == 0) ? alpha : beta;
                    Q &c = (coeff_idx == 0) ? target.a : (coeff_idx == 1 ? target.b : target.c);
                    if (k == FL_Up) c = c + rat(1);
                    else c = c - rat(1);
                    gamma = alpha * beta;
                }
                redraw();
                return 1;
            }
        }
        return Fl_Widget::handle(event);
    }
};

int main() {
    Fl_Double_Window* win = new Fl_Double_Window(1000, 700, "Cubic Field Element Analysis");
    new CubicRootWidget(0, 0, 1000, 700);
    win->resizable(win);
    win->show();
    return Fl::run();
}
