#include "../include/algebraic_field.hpp"
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/fl_draw.H>
#include <vector>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <complex>

void fft4_exact(const Qi* in, Qi* out) {
    Qi r1_0 = in[0] + in[2]; Qi r1_1 = in[0] - in[2];
    Qi r1_2 = in[1] + in[3]; Qi r1_3 = in[1] - in[3];
    Qi i_unit(rat(0), rat(1));
    out[0] = r1_0 + r1_2; out[1] = r1_1 - r1_3 * i_unit;
    out[2] = r1_0 - r1_2; out[3] = r1_1 + r1_3 * i_unit;
}

void fft4_float(const std::complex<double>* in, std::complex<double>* out) {
    std::complex<double> r1_0 = in[0] + in[2]; std::complex<double> r1_1 = in[0] - in[2];
    std::complex<double> r1_2 = in[1] + in[3]; std::complex<double> r1_3 = in[1] - in[3];
    std::complex<double> i_unit(0, 1);
    out[0] = r1_0 + r1_2; out[1] = r1_1 - r1_3 * i_unit;
    out[2] = r1_0 - r1_2; out[3] = r1_1 + r1_3 * i_unit;
}

class FFTWidget : public Fl_Widget {
public:
    Qi signalQ[4];
    FFTWidget(int X, int Y, int W, int H) : Fl_Widget(X, Y, W, H) {
        signalQ[0] = {rat(1), rat(0)}; signalQ[1] = {rat(2), rat(0)};
        signalQ[2] = {rat(3), rat(0)}; signalQ[3] = {rat(4), rat(0)};
    }
    void draw() override {
        fl_push_clip(x(), y(), w(), h());
        fl_color(FL_BLACK); fl_rectf(x(), y(), w(), h());
        Qi outQ[4]; std::complex<double> outF[4], signalF[4];
        for(int i=0; i<4; ++i) signalF[i] = { signalQ[i].a.approx(), signalQ[i].b.approx() };
        fft4_exact(signalQ, outQ); fft4_float(signalF, outF);
        fl_color(FL_WHITE); fl_font(FL_HELVETICA_BOLD, 18);
        fl_draw("Exact 4-point FFT in Q(i)", x()+20, y()+35);
        fl_font(FL_HELVETICA, 14); int ty = y() + 70;
        fl_draw("Input signal (Editable with Keys 1,2,3,4 + Up/Down):", x()+20, ty); ty+=30;
        for(int i=0; i<4; ++i) {
            fl_color(FL_WHITE); fl_draw((std::string("x[") + std::to_string(i) + "]: " + signalQ[i].str()).c_str(), x()+40, ty); ty+=20;
        }
        ty += 20; fl_draw("Output Spectrum:", x()+20, ty); ty+=30;
        for(int i=0; i<4; ++i) {
            fl_color(FL_CYAN); fl_draw((std::string("X[") + std::to_string(i) + "] Exact: " + outQ[i].str()).c_str(), x()+40, ty); ty+=20;
            fl_color(FL_GRAY); std::stringstream ss; ss << std::fixed << std::setprecision(15) << "      Float: " << outF[i].real() << " + " << outF[i].imag() << "i";
            fl_draw(ss.str().c_str(), x()+40, ty); ty+=25;
        }
        fl_pop_clip();
    }
    int handle(int e) override {
        static int idx = 0;
        if(e == FL_PUSH) { Fl::focus(this); return 1; }
        if(e == FL_KEYDOWN) {
            int k = Fl::event_key();
            if(k >= '1' && k <= '4') idx = k - '1';
            if(k == FL_Up) signalQ[idx].a = signalQ[idx].a + rat(1);
            if(k == FL_Down) signalQ[idx].a = signalQ[idx].a - rat(1);
            redraw(); return 1;
        }
        return Fl_Widget::handle(e);
    }
};

int main() {
    Fl_Double_Window* win = new Fl_Double_Window(800, 600, "Exact Algebraic FFT");
    new FFTWidget(0, 0, 800, 600); win->resizable(win); win->show();
    return Fl::run();
}
