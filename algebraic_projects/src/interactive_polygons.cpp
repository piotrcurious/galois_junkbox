#include "../include/algebraic_field.hpp"
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/fl_draw.H>
#include <vector>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <sstream>

// For N=4, we use Qi (Q(i))
// For N=3,6 we use Qomega (Q(w))

class PolygonWidget : public Fl_Widget {
public:
    double zoom;
    double offset_x, offset_y;
    int sides; // 3, 4, or 6 for now
    int rotations;
    bool show_float;

    PolygonWidget(int X, int Y, int W, int H) : Fl_Widget(X, Y, W, H), zoom(200.0), offset_x(0), offset_y(0), sides(4), rotations(0), show_float(true) {}

    void draw() override {
        fl_push_clip(x(), y(), w(), h());
        fl_color(FL_BLACK);
        fl_rectf(x(), y(), w(), h());

        double cx = x() + w() / 2.0 + offset_x;
        double cy = y() + h() / 2.0 + offset_y;

        std::vector<std::pair<double, double>> ptsQ, ptsF;

        if (sides == 4) {
            Qi pQ(rat(1), rat(0));
            Qi rotQ(rat(0), rat(1)); // i
            double pf_re = 1.0, pf_im = 0.0;
            const double f_cos = 0.0, f_sin = 1.0;

            for(int i=0; i<4; ++i) {
                auto pc = pQ.approx_complex();
                ptsQ.push_back({pc.first, pc.second});
                ptsF.push_back({pf_re, pf_im});
                pQ = pQ * rotQ;
                double nre = pf_re * f_cos - pf_im * f_sin;
                double nim = pf_re * f_sin + pf_im * f_cos;
                pf_re = nre; pf_im = nim;
            }
        } else if (sides == 3) {
            Qomega pQ(rat(1), rat(0));
            Qomega rotQ(rat(0), rat(1)); // w
            double pf_re = 1.0, pf_im = 0.0;
            const double f_cos = -0.5, f_sin = std::sqrt(3.0)/2.0;

            for(int i=0; i<3; ++i) {
                auto pc = pQ.approx();
                ptsQ.push_back({pc.first, pc.second});
                ptsF.push_back({pf_re, pf_im});
                pQ = pQ * rotQ;
                double nre = pf_re * f_cos - pf_im * f_sin;
                double nim = pf_re * f_sin + pf_im * f_cos;
                pf_re = nre; pf_im = nim;
            }
        }

        // Draw exact (Green)
        fl_color(FL_GREEN);
        fl_line_style(FL_SOLID, 2);
        for(size_t i=0; i<ptsQ.size(); ++i) {
            int next = (i+1)%ptsQ.size();
            fl_line(cx + ptsQ[i].first * zoom, cy - ptsQ[i].second * zoom,
                    cx + ptsQ[next].first * zoom, cy - ptsQ[next].second * zoom);
        }

        // Multiple rotations to show drift
        if (rotations > 0) {
            double ex, ey, pf_re = 1.0, pf_im = 0.0;
            if (sides == 4) {
                Qi pQ(rat(1), rat(0));
                Qi rotQ(rat(0), rat(1));
                const double f_cos = 0.0, f_sin = 1.0;
                for(int r=0; r<rotations; ++r) {
                    pQ = pQ * rotQ;
                    double nre = pf_re * f_cos - pf_im * f_sin;
                    double nim = pf_re * f_sin + pf_im * f_cos;
                    pf_re = nre; pf_im = nim;
                }
                auto pc = pQ.approx_complex(); ex = pc.first; ey = pc.second;
            } else {
                Qomega pQ(rat(1), rat(0));
                Qomega rotQ(rat(0), rat(1));
                const double f_cos = -0.5, f_sin = std::sqrt(3.0)/2.0;
                for(int r=0; r<rotations; ++r) {
                    pQ = pQ * rotQ;
                    double nre = pf_re * f_cos - pf_im * f_sin;
                    double nim = pf_re * f_sin + pf_im * f_cos;
                    pf_re = nre; pf_im = nim;
                }
                auto pc = pQ.approx(); ex = pc.first; ey = pc.second;
            }
            fl_color(FL_CYAN); fl_pie(cx + ex*zoom - 5, cy - ey*zoom - 5, 10, 10, 0, 360);
            fl_color(FL_RED); fl_pie(cx + pf_re*zoom - 3, cy - pf_im*zoom - 3, 6, 6, 0, 360);
            double err = std::sqrt(std::pow(ex-pf_re, 2) + std::pow(ey-pf_im, 2));
            fl_color(FL_WHITE);
            fl_draw(("Error after " + std::to_string(rotations) + " rotations: " + std::to_string(err)).c_str(), x()+10, y()+h()-20);
        }

        fl_color(FL_WHITE);
        fl_font(FL_HELVETICA, 14);
        fl_draw("Exact Polygon Rotation: Q(zeta_n)", x()+10, y()+20);
        fl_draw(("Sides: " + std::to_string(sides) + " (3/4 to switch)").c_str(), x()+10, y()+40);
        fl_draw(("Rotations: " + std::to_string(rotations) + " (Up/Down)").c_str(), x()+10, y()+60);

        fl_pop_clip();
    }

    int handle(int event) override {
        switch (event) {
            case FL_FOCUS: return 1;
            case FL_PUSH: Fl::focus(this); return 1;
            case FL_KEYDOWN:
                if (Fl::event_key() == '3') sides = 3;
                else if (Fl::event_key() == '4') sides = 4;
                else if (Fl::event_key() == FL_Up) rotations += 1000;
                else if (Fl::event_key() == FL_Down && rotations >= 1000) rotations -= 1000;
                redraw();
                return 1;
        }
        return Fl_Widget::handle(event);
    }
};

int main() {
    Fl_Double_Window* win = new Fl_Double_Window(800, 600, "Exact Regular Polygons");
    PolygonWidget* pw = new PolygonWidget(0, 0, 800, 600);
    win->resizable(pw);
    win->show();
    Fl::focus(pw);
    return Fl::run();
}
