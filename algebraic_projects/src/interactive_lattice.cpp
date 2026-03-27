#include "../include/algebraic_field.hpp"
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/fl_draw.H>
#include <vector>
#include <cmath>
#include <iostream>
#include <string>

class LatticeWidget : public Fl_Widget {
    int field_type; // 0: Qi, 1: Qomega, 2: Qsqrt2
    double scale = 50.0;
    int mx, my;
    bool selected = false;
    std::pair<long long, long long> selected_point = {0,0};

public:
    LatticeWidget(int X, int Y, int W, int H) : Fl_Widget(X, Y, W, H), field_type(0) {}

    void draw() override {
        fl_push_clip(x(), y(), w(), h());
        fl_color(FL_BLACK); fl_rectf(x(), y(), w(), h());

        double cx = x() + w()/2.0, cy = y() + h()/2.0;

        // Draw lattice points
        fl_color(FL_GRAY);
        for(int i=-15; i<=15; ++i) {
            for(int j=-15; j<=15; ++j) {
                double px, py;
                if(field_type == 0) { // Qi: a + bi
                    px = cx + i*scale; py = cy - j*scale;
                } else if(field_type == 1) { // Qomega: a + b*(-0.5 + i*sqrt(3)/2)
                    px = cx + (i - 0.5*j)*scale; py = cy - (0.5*std::sqrt(3.0)*j)*scale;
                } else { // Qsqrt2: a + b*sqrt(2) (Projected onto complex plane as a, b*sqrt2?)
                    // Actually let's just do Qi and Qomega for lattice.
                    px = cx + i*scale; py = cy - j*scale;
                }
                fl_pie(px-2, py-2, 4, 4, 0, 360);
            }
        }

        fl_color(FL_WHITE);
        fl_draw("Field Lattice Visualization", x()+20, y()+30);
        fl_draw("1: Gaussian Integers Z[i], 2: Eisenstein Integers Z[w]", x()+20, y()+50);
        fl_draw("Click a point to select it.", x()+20, y()+70);

        if(selected) {
            double px, py;
            if(field_type == 0) { px = cx + selected_point.first*scale; py = cy - selected_point.second*scale; }
            else { px = cx + (selected_point.first - 0.5*selected_point.second)*scale; py = cy - (0.5*std::sqrt(3.0)*selected_point.second)*scale; }
            fl_color(FL_YELLOW); fl_pie(px-4, py-4, 8, 8, 0, 360);

            std::string s;
            if(field_type == 0) {
                Qi val(rat(selected_point.first), rat(selected_point.second));
                s = "Selected: " + val.str() + " (Norm: " + (val.a*val.a + val.b*val.b).str() + ")";
            } else {
                Qomega val(rat(selected_point.first), rat(selected_point.second));
                s = "Selected: " + val.str() + " (Norm: " + (val.a*val.a + val.b*val.b - val.a*val.b).str() + ")";
            }
            fl_draw(s.c_str(), x()+20, y()+h()-20);
        }

        fl_pop_clip();
    }

    int handle(int e) override {
        if(e == FL_PUSH) {
            Fl::focus(this);
            double cx = x() + w()/2.0, cy = y() + h()/2.0;
            // Reverse mapping from mouse to lattice point
            double dx = (Fl::event_x() - cx) / scale;
            double dy = (cy - Fl::event_y()) / scale;
            if(field_type == 0) {
                selected_point = {(long long)std::round(dx), (long long)std::round(dy)};
            } else {
                double j = dy / (0.5*std::sqrt(3.0));
                double i = dx + 0.5*j;
                selected_point = {(long long)std::round(i), (long long)std::round(j)};
            }
            selected = true; redraw(); return 1;
        }
        if(e == FL_KEYDOWN) {
            if(Fl::event_key() == '1') field_type = 0;
            if(Fl::event_key() == '2') field_type = 1;
            redraw(); return 1;
        }
        return Fl_Widget::handle(e);
    }
};

int main() {
    Fl_Double_Window* win = new Fl_Double_Window(800, 600, "Algebraic Integer Lattice");
    new LatticeWidget(0, 0, 800, 600);
    win->resizable(win); win->show();
    return Fl::run();
}
