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

    CubicRootWidget(int X, int Y, int W, int H) : Fl_Widget(X, Y, W, H), scale(120.0), offset_x(180), offset_y(0),
                                                   alpha(rat(1), rat(1), rat(0)), beta(rat(0), rat(1), rat(1)), edit_mode(0), coeff_idx(0) {
        gamma = alpha * beta;
    }

    void draw_matrix(int x, int y, Qcbrt2 val, const char* label) {
        fl_color(FL_WHITE);
        fl_font(FL_HELVETICA, 12);
        fl_draw((std::string("Multiplication Matrix for ") + label).c_str(), x, y-10);

        // Matrix for a + b*rho + c*rho^2 is:
        // [ a   2c  2b ]
        // [ b   a   2c ]
        // [ c   b   a  ]
        // (assuming rho^3 = 2)

        auto s = [](Q q) { return q.str(); };
        std::string m[3][3] = {
            { s(val.a), s(rat(2)*val.c), s(rat(2)*val.b) },
            { s(val.b), s(val.a),        s(rat(2)*val.c) },
            { s(val.c), s(val.b),        s(val.a) }
        };

        int cw = 60, ch = 25;
        for(int i=0; i<3; ++i) {
            for(int j=0; j<3; ++j) {
                fl_rect(x + j*cw, y + i*ch, cw, ch);
                fl_draw(m[i][j].c_str(), x + j*cw + 5, y + i*ch + 18);
            }
        }
        fl_draw((std::string("det(M) = Norm = ") + val.norm().str()).c_str(), x, y + 3*ch + 15);
    }

    void draw() override {
        fl_push_clip(x(), y(), w(), h());
        fl_color(FL_BLACK);
        fl_rectf(x(), y(), w(), h());

        double cx = x() + w() / 2.0 + offset_x;
        double cy = y() + h() / 2.0 + offset_y;

        // Grid
        fl_color(FL_LIGHT2);
        for(int i=-10; i<=10; ++i) {
            fl_line(cx + i*scale, y(), cx + i*scale, y()+h());
            fl_line(x(), cy + i*scale, x()+w(), cy + i*scale);
        }

        double rho = std::pow(2.0, 1.0/3.0);
        std::complex<double> roots[3] = {
            {rho, 0},
            {rho * -0.5, rho * std::sqrt(3.0)/2.0},
            {rho * -0.5, rho * -std::sqrt(3.0)/2.0}
        };

        fl_color(FL_GRAY);
        fl_arc(cx - rho*scale, cy - rho*scale, 2*rho*scale, 2*rho*scale, 0, 360);

        for(int i=0; i<3; ++i) {
            double rx = cx + roots[i].real()*scale;
            double ry = cy - roots[i].imag()*scale;
            fl_color(i==0 ? FL_GREEN : FL_MAGENTA);
            fl_pie(rx-5, ry-5, 10, 10, 0, 360);
        }

        // Side Panel UI
        int pw = 360;
        fl_color(FL_DARK2);
        fl_rectf(x(), y(), pw, h());
        fl_color(FL_WHITE);
        fl_line(x()+pw, y(), x()+pw, y()+h());

        fl_color(FL_YELLOW);
        fl_font(FL_HELVETICA_BOLD, 20);
        fl_draw("Cubic Field Analysis", x()+20, y()+35);

        fl_font(FL_HELVETICA, 14);
        int ty = y() + 70;

        auto draw_editor = [&](const char* label, Qcbrt2& val, int mode) {
            bool active = (edit_mode == mode);
            fl_color(active ? FL_CYAN : FL_WHITE);
            fl_draw((std::string(label) + " = " + val.str()).c_str(), x()+20, ty); ty+=25;

            if (active) {
                const char* coeffs[] = {"a", "b", "c"};
                fl_draw((std::string("Editing ") + coeffs[coeff_idx] + " (Use Up/Down, 1/2/3)").c_str(), x()+40, ty); ty+=20;
            }
            draw_matrix(x()+20, ty, val, label);
            ty += 110;
        };

        draw_editor("alpha", alpha, 0);
        draw_editor("beta", beta, 1);

        fl_color(FL_GREEN);
        fl_draw((std::string("alpha * beta = ") + gamma.str()).c_str(), x()+20, ty); ty+=25;
        bool check = (alpha.norm() * beta.norm() == gamma.norm());
        fl_color(check ? FL_GREEN : FL_RED);
        fl_draw(check ? "EXACT N(a)*N(b) == N(ab) satisfied." : "ERROR: Norm Identity Violation", x()+20, ty);

        fl_color(FL_WHITE);
        fl_draw("TAB: Switch alpha/beta, Drag: Pan, Scroll: Zoom", x()+20, h()-20);

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
    Fl_Double_Window* win = new Fl_Double_Window(1200, 800, "Cubic Field Element & Matrix Analysis");
    CubicRootWidget* cw = new CubicRootWidget(0, 0, 1200, 800);
    win->resizable(cw);
    win->show();
    Fl::focus(cw);
    return Fl::run();
}
