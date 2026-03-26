#include "../include/algebraic_field.hpp"
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Tabs.H>
#include <FL/Fl_Group.H>
#include <FL/fl_draw.H>
#include <vector>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <complex>

// 1. Snowflake
class SnowflakeWidget : public Fl_Widget {
    double zoom = 1.0, ox = 0, oy = 0, azf = 1.0;
    int depth = 4;
    bool az = false;
public:
    SnowflakeWidget(int X, int Y, int W, int H) : Fl_Widget(X, Y, W, H) {}
    void draw() override {
        fl_push_clip(x(), y(), w(), h());
        fl_color(FL_BLACK); fl_rectf(x(), y(), w(), h());
        double cur_z = zoom * azf;
        auto stepQ = [](std::vector<std::pair<Q3, Q3>>& pts) {
            std::vector<std::pair<Q3, Q3>> next;
            Q3 one3(rat(1, 3)), half(rat(1, 2)), s3_6(rat(0), rat(1, 6));
            for (size_t i = 0; i < pts.size() - 1; i++) {
                auto p1 = pts[i], p2 = pts[i+1];
                Q3 dx = p2.first - p1.first, dy = p2.second - p1.second;
                auto a = std::make_pair(p1.first + one3*dx, p1.second + one3*dy);
                auto c = std::make_pair(p1.first + Q3(rat(2,3))*dx, p1.second + Q3(rat(2,3))*dy);
                auto b = std::make_pair(half*(p1.first + p2.first) + s3_6*dy, half*(p1.second + p2.second) - s3_6*dx);
                next.push_back(p1); next.push_back(a); next.push_back(b); next.push_back(c);
            }
            next.push_back(pts.back()); pts = next;
        };
        std::vector<std::pair<Q3, Q3>> pts = { {Q3(rat(-200)), Q3(rat(0), rat(100))}, {Q3(rat(0)), Q3(rat(0), rat(-100))}, {Q3(rat(200)), Q3(rat(0), rat(100))}, {Q3(rat(-200)), Q3(rat(0), rat(100))} };
        for(int i=0; i<depth; ++i) stepQ(pts);
        double cx = x() + w()/2.0 + ox, cy = y() + h()/2.0 + oy;
        if(az) { cx = x()+w()/2.0 - pts[pts.size()/3].first.approx()*cur_z; cy = y()+h()/2.0 - pts[pts.size()/3].second.approx()*cur_z; }
        fl_color(FL_CYAN);
        for(size_t i = 0; i < pts.size() - 1; i++) fl_line(cx + pts[i].first.approx()*cur_z, cy + pts[i].second.approx()*cur_z, cx + pts[i+1].first.approx()*cur_z, cy + pts[i+1].second.approx()*cur_z);
        fl_color(FL_WHITE); fl_draw("Up/Down: Depth, A: Auto-Zoom, Scroll: Zoom", x()+10, y()+20);
        fl_pop_clip();
    }
    int handle(int e) override {
        if(e == FL_PUSH) { Fl::focus(this); return 1; }
        if(e == FL_KEYDOWN) {
            if(Fl::event_key() == FL_Up && depth < 6) depth++;
            if(Fl::event_key() == FL_Down && depth > 0) depth--;
            if(Fl::event_key() == 'a') { az = !az; if(az) Fl::add_timeout(0.05, timeout, this); else azf=1.0; }
            redraw(); return 1;
        }
        return Fl_Widget::handle(e);
    }
    static void timeout(void* d) { SnowflakeWidget* w = (SnowflakeWidget*)d; if(w->az) { w->azf *= 1.1; if(w->azf > 1e15) w->azf = 1.0; w->redraw(); Fl::repeat_timeout(0.05, timeout, d); } }
};

// 2. Pell Identity
class PellWidget : public Fl_Widget {
    int n = 1;
public:
    PellWidget(int X, int Y, int W, int H) : Fl_Widget(X, Y, W, H) {}
    void draw() override {
        fl_push_clip(x(), y(), w(), h());
        fl_color(FL_BLACK); fl_rectf(x(), y(), w(), h());
        Q2 unit(rat(1), rat(1)), cur(rat(1), rat(0));
        long double fa = 1.0, fb = 0.0;
        for(int i=0; i<n; ++i) { cur = cur * unit; long double na = fa + 2.0*fb, nb = fa + fb; fa = na; fb = nb; }
        fl_color(FL_WHITE); fl_font(FL_HELVETICA, 16);
        int ty = y() + 40;
        fl_draw(("Sequence of units in Z[sqrt(2)]: (1 + sqrt(2))^" + std::to_string(n)).c_str(), x()+20, ty); ty+=40;
        fl_color(FL_CYAN); fl_draw(("Exact a: " + cur.a.str()).c_str(), x()+20, ty); ty+=25;
        fl_draw(("Exact b: " + cur.b.str()).c_str(), x()+20, ty); ty+=40;
        Q norm = cur.norm();
        fl_color(FL_GREEN); fl_draw(("Exact Norm (a^2 - 2b^2) = " + norm.str()).c_str(), x()+20, ty); ty+=40;
        long double fnorm = fa*fa - 2.0*fb*fb;
        fl_color(FL_RED); fl_draw(("Float Norm = " + std::to_string((double)fnorm)).c_str(), x()+20, ty); ty+=25;
        fl_draw(("Float Error = " + std::to_string(std::abs((double)fnorm - (double)norm.approx()))).c_str(), x()+20, ty);
        fl_color(FL_WHITE); fl_draw("Up/Down: Increase n", x()+20, h()-20);
        fl_pop_clip();
    }
    int handle(int e) override {
        if(e == FL_PUSH) { Fl::focus(this); return 1; }
        if(e == FL_KEYDOWN) { if(Fl::event_key() == FL_Up && n < 60) n++; if(Fl::event_key() == FL_Down && n > 0) n--; redraw(); return 1; }
        return Fl_Widget::handle(e);
    }
};

// 3. Rotation Drift
class RotationWidget : public Fl_Widget {
    int iter = 1000;
public:
    RotationWidget(int X, int Y, int W, int H) : Fl_Widget(X, Y, W, H) {}
    void draw() override {
        fl_push_clip(x(), y(), w(), h());
        fl_color(FL_BLACK); fl_rectf(x(), y(), w(), h());
        Q3 cos30(rat(0), rat(1,2)), sin30(rat(1,2), rat(0));
        const double fcos = std::sqrt(3.0)/2.0, fsin = 0.5;
        std::pair<Q3, Q3> pQ = {Q3(rat(1)), Q3(rat(0))};
        std::pair<double, double> pF = {1.0, 0.0};
        double zoom = 200.0, cx = x()+w()/2.0, cy = y()+h()/2.0;
        for(int i=0; i<iter; ++i) {
            auto nQ = std::make_pair(pQ.first*cos30 - pQ.second*sin30, pQ.first*sin30 + pQ.second*cos30);
            pQ = nQ;
            double nFre = pF.first*fcos - pF.second*fsin, nFim = pF.first*fsin + pF.second*fcos;
            pF = {nFre, nFim};
            if(i%10 == 0 || i == iter-1) {
                fl_color(FL_GREEN); fl_pie(cx + pQ.first.approx()*zoom-2, cy + pQ.second.approx()*zoom-2, 4, 4, 0, 360);
                fl_color(FL_RED); fl_pie(cx + pF.first*zoom-1, cy + pF.second*zoom-1, 2, 2, 0, 360);
            }
        }
        fl_color(FL_WHITE); fl_draw(("Iterations: " + std::to_string(iter) + " (Up/Down)").c_str(), x()+20, y()+30);
        double err = std::sqrt(std::pow(pQ.first.approx()-pF.first, 2) + std::pow(pQ.second.approx()-pF.second, 2));
        fl_draw(("Tip Drift: " + std::to_string(err)).c_str(), x()+20, y()+50);
        fl_pop_clip();
    }
    int handle(int e) override {
        if(e == FL_PUSH) { Fl::focus(this); return 1; }
        if(e == FL_KEYDOWN) { if(Fl::event_key() == FL_Up) iter+=500; if(Fl::event_key() == FL_Down && iter > 500) iter-=500; redraw(); return 1; }
        return Fl_Widget::handle(e);
    }
};

// 4. Golden Decomposition
class GoldenWidget : public Fl_Widget {
    int depth = 5;
public:
    GoldenWidget(int X, int Y, int W, int H) : Fl_Widget(X, Y, W, H) {}
    void draw() override {
        fl_push_clip(x(), y(), w(), h());
        fl_color(FL_BLACK); fl_rectf(x(), y(), w(), h());
        Q5 phi(rat(1,2), rat(1,2));
        double phia = phi.approx();
        double rw = 700.0, rh = rw / phia;
        double rx = x() + (w()-rw)/2.0, ry = y() + (h()-rh)/2.0;
        auto draw_rec = [&](auto self, double x, double y, double w, double h, int d, bool hor) -> void {
            if(d == 0) return;
            fl_color(FL_CYAN); fl_rect(x, y, w, h);
            if(hor) {
                fl_color(FL_YELLOW); fl_rect(x, y, h, h);
                self(self, x+h, y, w-h, h, d-1, false);
            } else {
                fl_color(FL_YELLOW); fl_rect(x, y, w, w);
                self(self, x, y+w, w, h-w, d-1, true);
            }
        };
        draw_rec(draw_rec, rx, ry, rw, rh, depth, true);
        fl_color(FL_WHITE); fl_draw(("Depth: " + std::to_string(depth) + " (Up/Down)").c_str(), x()+20, y()+30);
        fl_pop_clip();
    }
    int handle(int e) override {
        if(e == FL_PUSH) { Fl::focus(this); return 1; }
        if(e == FL_KEYDOWN) { if(Fl::event_key() == FL_Up && depth < 15) depth++; if(Fl::event_key() == FL_Down && depth > 1) depth--; redraw(); return 1; }
        return Fl_Widget::handle(e);
    }
};

int main() {
    Fl_Double_Window* win = new Fl_Double_Window(1000, 800, "Algebraic Field Demonstration Dashboard");
    Fl_Tabs* tabs = new Fl_Tabs(0, 0, 1000, 800);
    {
        Fl_Group* g = new Fl_Group(0, 25, 1000, 775, "Snowflake");
        new SnowflakeWidget(10, 40, 980, 740);
        g->end();
    }
    {
        Fl_Group* g = new Fl_Group(0, 25, 1000, 775, "Pell Identity");
        new PellWidget(10, 40, 980, 740);
        g->end();
    }
    {
        Fl_Group* g = new Fl_Group(0, 25, 1000, 775, "Rotation Drift");
        new RotationWidget(10, 40, 980, 740);
        g->end();
    }
    {
        Fl_Group* g = new Fl_Group(0, 25, 1000, 775, "Golden Rect");
        new GoldenWidget(10, 40, 980, 740);
        g->end();
    }
    tabs->end();
    win->resizable(tabs);
    win->show();
    return Fl::run();
}
