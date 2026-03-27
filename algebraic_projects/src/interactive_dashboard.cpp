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

enum PrecisionType { PRE_FLOAT, PRE_DOUBLE };
PrecisionType GLOBAL_PRECISION = PRE_DOUBLE;

// 1. Theodorus
class TheodorusWidget : public Fl_Widget {
    double zoom = 40.0, ox = 0, oy = 0; int steps = 45; bool az = false, show_g = false; int mx, my;
public:
    TheodorusWidget(int X, int Y, int W, int H) : Fl_Widget(X, Y, W, H) {}
    template<typename T> void get_f_p(std::vector<std::pair<double, double>>& p) {
        T fx=1, fy=0; p.push_back({0,0}); p.push_back({1,0});
        for(int n=1; n<=steps; ++n) { T r=std::sqrt(fx*fx+fy*fy), nx=-fy/r, ny=fx/r; fx+=nx; fy+=ny; p.push_back({(double)fx, (double)fy}); }
    }
    void draw_paths(double cx, double cy, double z, const std::vector<std::pair<double,double>>& ap, const std::vector<std::pair<double,double>>& fp) {
        fl_color(FL_RED); for(size_t i=0; i<fp.size()-1; ++i) fl_line(cx+fp[i].first*z, cy+fp[i].second*z, cx+fp[i+1].first*z, cy+fp[i+1].second*z);
        fl_color(FL_GREEN); fl_line_style(FL_SOLID, 2); for(size_t i=0; i<ap.size()-1; ++i) fl_line(cx+ap[i].first*z, cy+ap[i].second*z, cx+ap[i+1].first*z, cy+ap[i+1].second*z);
        fl_line_style(0);
    }
    void draw() override {
        fl_push_clip(x(), y(), w(), h()); fl_color(FL_BLACK); fl_rectf(x(), y(), w(), h());
        std::vector<std::pair<double, double>> ap = {{0,0}, {1,0}}, fp; double ang=0;
        for(int n=1; n<=steps; ++n) { ang += std::atan(1.0/std::sqrt((double)n)); double r=std::sqrt((double)n+1); ap.push_back({r*std::cos(ang), -r*std::sin(ang)}); }
        if(GLOBAL_PRECISION == PRE_FLOAT) get_f_p<float>(fp); else get_f_p<double>(fp);
        double cx = x()+w()/2.0+ox, cy = y()+h()/2.0+oy;
        if(az) { cx = x()+w()/2.0 - ap.back().first*zoom; cy = y()+h()/2.0 - ap.back().second*zoom; }
        draw_paths(cx, cy, zoom, ap, fp);
        if(show_g) {
            int gr=120; fl_push_clip(mx-gr, my-gr, 2*gr, 2*gr); fl_color(FL_BLACK); fl_rectf(mx-gr, my-gr, 2*gr, 2*gr);
            double gz = zoom*500.0; double mx_model = (mx - cx) / zoom; double my_model = (my - cy) / zoom;
            double gcx = mx - mx_model * gz, gcy = my - my_model * gz;
            draw_paths(gcx, gcy, gz, ap, fp); fl_pop_clip(); fl_color(FL_WHITE); fl_arc(mx-gr, my-gr, 2*gr, 2*gr, 0, 360);
        }
        fl_color(FL_WHITE); fl_draw(("Steps: "+std::to_string(steps)+", P: "+(GLOBAL_PRECISION==PRE_FLOAT?"float32":"float64")+", G: Glass, A: Follow").c_str(), x()+10, y()+20);
        fl_pop_clip();
    }
    int handle(int e) override {
        if(e==FL_PUSH){ Fl::focus(this); mx=Fl::event_x(); my=Fl::event_y(); return 1; }
        if(e==FL_MOVE || e==FL_DRAG){ if(e==FL_DRAG){ ox+=Fl::event_x()-mx; oy+=Fl::event_y()-my; } mx=Fl::event_x(); my=Fl::event_y(); redraw(); return 1; }
        if(e==FL_MOUSEWHEEL){ zoom *= (Fl::event_dy()<0?1.1:0.9); redraw(); return 1; }
        if(e==FL_KEYDOWN){
            int k=Fl::event_key(); if(k==FL_Up) steps+=5; else if(k==FL_Down && steps>5) steps-=5;
            else if(k=='g') show_g=!show_g; else if(k=='a') az=!az; else if(k=='p') GLOBAL_PRECISION=(GLOBAL_PRECISION==PRE_FLOAT?PRE_DOUBLE:PRE_FLOAT);
            redraw(); return 1;
        }
        return Fl_Widget::handle(e);
    }
};

// 2. Snowflake
class SnowflakeWidget : public Fl_Widget {
    double zoom=1, ox=0, oy=0, azf=1; int depth=4; bool az=false, ss=false;
public:
    SnowflakeWidget(int X, int Y, int W, int H) : Fl_Widget(X, Y, W, H) {}
    template<typename T> void step(std::vector<std::pair<T,T>>& p) {
        std::vector<std::pair<T,T>> n; T i3=1.0/3.0, s36=std::sqrt(3.0)/6.0;
        for(size_t i=0; i<p.size()-1; ++i) {
            auto p1=p[i], p2=p[i+1]; T dx=p2.first-p1.first, dy=p2.second-p1.second;
            n.push_back(p1); n.push_back({p1.first+i3*dx, p1.second+i3*dy});
            n.push_back({(T)0.5*(p1.first+p2.first)+s36*dy, (T)0.5*(p1.second+p2.second)-s36*dx});
            n.push_back({p1.first+(T)(2.0/3.0)*dx, p1.second+(T)(2.0/3.0)*dy});
        }
        n.push_back(p.back()); p=n;
    }
    void draw_snowflake(double cx, double cy, double z, bool exact, Fl_Color col) {
        if(exact) {
            std::vector<std::pair<Q3,Q3>> p = {{Q3(rat(-200)), Q3(rat(0),rat(100))}, {Q3(rat(0)), Q3(rat(0),rat(-100))}, {Q3(rat(200)), Q3(rat(0),rat(100))}, {Q3(rat(-200)), Q3(rat(0),rat(100))}};
            auto stepQ = [](std::vector<std::pair<Q3,Q3>>& pts) {
                std::vector<std::pair<Q3,Q3>> n; Q3 i3(rat(1,3)), h(rat(1,2)), s(rat(0),rat(1,6));
                for(size_t i=0; i<pts.size()-1; ++i) {
                    auto p1=pts[i], p2=pts[i+1]; Q3 dx=p2.first-p1.first, dy=p2.second-p1.second;
                    n.push_back(p1); n.push_back({p1.first+i3*dx, p1.second+i3*dy});
                    n.push_back({h*(p1.first+p2.first)+s*dy, h*(p1.second+p2.second)-s*dx});
                    n.push_back({p1.first+Q3(rat(2,3))*dx, p1.second+Q3(rat(2,3))*dy});
                }
                n.push_back(pts.back()); pts=n;
            };
            for(int i=0; i<depth; ++i) stepQ(p);
            if(az) { cx-=p[p.size()/3].first.approx()*z; cy-=p[p.size()/3].second.approx()*z; }
            fl_color(col); for(size_t i=0; i<p.size()-1; ++i) fl_line(cx+p[i].first.approx()*z, cy+p[i].second.approx()*z, cx+p[i+1].first.approx()*z, cy+p[i+1].second.approx()*z);
        } else {
            if(GLOBAL_PRECISION == PRE_FLOAT) {
                std::vector<std::pair<float,float>> p = {{-200, 100*sqrtf(3)}, {0, -100*sqrtf(3)}, {200, 100*sqrtf(3)}, {-200, 100*sqrtf(3)}};
                for(int i=0; i<depth; ++i) step<float>(p);
                fl_color(col); for(size_t i=0; i<p.size()-1; ++i) fl_line(cx+p[i].first*z, cy+p[i].second*z, cx+p[i+1].first*z, cy+p[i+1].second*z);
            } else {
                std::vector<std::pair<double,double>> p = {{-200, 100*sqrt(3)}, {0, -100*sqrt(3)}, {200, 100*sqrt(3)}, {-200, 100*sqrt(3)}};
                for(int i=0; i<depth; ++i) step<double>(p);
                fl_color(col); for(size_t i=0; i<p.size()-1; ++i) fl_line(cx+p[i].first*z, cy+p[i].second*z, cx+p[i+1].first*z, cy+p[i+1].second*z);
            }
        }
    }
    void draw() override {
        fl_push_clip(x(), y(), w(), h()); fl_color(FL_BLACK); fl_rectf(x(), y(), w(), h());
        double cz = zoom*azf;
        if(!ss) { draw_snowflake(x()+w()/2.0+ox, y()+h()/2.0+oy, cz, true, FL_CYAN); }
        else {
            fl_push_clip(x(), y(), w()/2, h()); draw_snowflake(x()+w()/4.0+ox, y()+h()/2.0+oy, cz, true, FL_CYAN); fl_pop_clip();
            fl_push_clip(x() + w()/2, y(), w()/2, h()); draw_snowflake(x()+3*w()/4.0+ox, y()+h()/2.0+oy, cz, false, FL_RED); fl_pop_clip();
        }
        fl_color(FL_WHITE); fl_draw(("Depth: "+std::to_string(depth)+", P: "+(GLOBAL_PRECISION==PRE_FLOAT?"float32":"float64")+", S: Split, A: AutoZoom").c_str(), x()+10, y()+20);
        fl_pop_clip();
    }
    int handle(int e) override {
        if(e==FL_PUSH) { Fl::focus(this); return 1; }
        if(e==FL_KEYDOWN) {
            int k=Fl::event_key(); if(k==FL_Up && depth<6) depth++; else if(k==FL_Down && depth>0) depth--;
            else if(k=='p') GLOBAL_PRECISION=(GLOBAL_PRECISION==PRE_FLOAT?PRE_DOUBLE:PRE_FLOAT); else if(k=='s') ss=!ss;
            else if(k=='a') { az=!az; if(az) Fl::add_timeout(0.05, timeout, this); else azf=1; }
            redraw(); return 1;
        }
        return Fl_Widget::handle(e);
    }
    static void timeout(void* d) { SnowflakeWidget* w=(SnowflakeWidget*)d; if(w->az){ w->azf*=1.05; if(w->azf>1e15)w->azf=1; w->redraw(); Fl::repeat_timeout(0.05, timeout, d); } }
};

// 3. Pell sequence
class PellWidget : public Fl_Widget {
    int n = 1, D = 2;
public:
    PellWidget(int X, int Y, int W, int H) : Fl_Widget(X, Y, W, H) {}
    void draw() override {
        fl_push_clip(x(), y(), w(), h()); fl_color(FL_BLACK); fl_rectf(x(), y(), w(), h());
        long double fa = 1.0, fb = 0.0; std::string res_a, res_b, res_norm;
        if(D==2) { Q2 u(rat(1), rat(1)), c(rat(1), rat(0)); for(int i=0; i<n; ++i) { c = c*u; if(GLOBAL_PRECISION==PRE_FLOAT){float na=(float)fa+2.0f*fb, nb=fa+fb; fa=na; fb=nb;}else{double na=fa+2.0*fb, nb=fa+fb; fa=na; fb=nb;} } res_a=c.a.str(); res_b=c.b.str(); res_norm=c.norm().str(); }
        else if(D==3) { Q3 u(rat(2), rat(1)), c(rat(1), rat(0)); for(int i=0; i<n; ++i) { c = c*u; if(GLOBAL_PRECISION==PRE_FLOAT){float na=2.0f*fa+3.0f*fb, nb=fa+2.0f*fb; fa=na; fb=nb;}else{double na=2.0*fa+3.0*fb, nb=fa+2.0*fb; fa=na; fb=nb;} } res_a=c.a.str(); res_b=c.b.str(); res_norm=c.norm().str(); }
        else { Q5 u(rat(9), rat(4)), c(rat(1), rat(0)); for(int i=0; i<n; ++i) { c = c*u; if(GLOBAL_PRECISION==PRE_FLOAT){float na=9.0f*fa+20.0f*fb, nb=4.0f*fa+9.0f*fb; fa=na; fb=nb;}else{double na=9.0*fa+20.0*fb, nb=4.0*fa+9.0*fb; fa=na; fb=nb;} } res_a=c.a.str(); res_b=c.b.str(); res_norm=c.norm().str(); }
        fl_color(FL_WHITE); fl_font(FL_HELVETICA, 16); int ty = y() + 40;
        fl_draw(("Pell sequence level " + std::to_string(n)).c_str(), x()+20, ty); ty+=40;
        fl_color(FL_CYAN); fl_draw(("Exact a: " + res_a).c_str(), x()+20, ty); ty+=30;
        fl_draw(("Exact b: " + res_b).c_str(), x()+20, ty); ty+=40;
        fl_color(FL_GREEN); fl_draw(("Exact Norm = " + res_norm).c_str(), x()+20, ty); ty+=40;
        long double fnorm = fa*fa - (long double)D*fb*fb;
        fl_color(FL_RED); fl_draw(("Float Norm (" + std::string(GLOBAL_PRECISION==PRE_FLOAT?"32":"64") + " bit) = " + std::to_string((double)fnorm)).c_str(), x()+20, ty); ty+=30;
        fl_draw(("Error = " + std::to_string(std::abs((double)fnorm - 1.0))).c_str(), x()+20, ty);
        fl_color(FL_WHITE); fl_draw("Up/Down: n, P: Precision, D: Switch field (2,3,5)", x()+20, h()-20);
        fl_pop_clip();
    }
    int handle(int e) override {
        if(e==FL_PUSH){ Fl::focus(this); return 1; }
        if(e==FL_KEYDOWN){ int k=Fl::event_key(); if(k==FL_Up && n<60) n++; else if(k==FL_Down && n>0) n--;
            else if(k=='p') GLOBAL_PRECISION=(GLOBAL_PRECISION==PRE_FLOAT?PRE_DOUBLE:PRE_FLOAT);
            else if(k=='d') { if(D==2) D=3; else if(D==3) D=5; else D=2; }
            redraw(); return 1;
        }
        return Fl_Widget::handle(e);
    }
};

// 4. Rotation
class RotationWidget : public Fl_Widget {
    int iter = 1000; int type = 0;
public:
    RotationWidget(int X, int Y, int W, int H) : Fl_Widget(X, Y, W, H) {}
    template<typename T> void get_path(std::vector<std::pair<double,double>>& p) {
        T fcos, fsin, fx=1, fy=0;
        if(type==0) { fcos = std::sqrt(3.0)/2.0; fsin = 0.5; } else { fcos = 0.5; fsin = std::sqrt(3.0)/2.0; }
        for(int i=0; i<iter; ++i) { T nfx=fx*fcos-fy*fsin, nfy=fx*fsin+fy*fcos; fx=nfx; fy=nfy; p.push_back({(double)fx,(double)fy}); }
    }
    void draw() override {
        fl_push_clip(x(), y(), w(), h()); fl_color(FL_BLACK); fl_rectf(x(), y(), w(), h());
        std::vector<std::pair<double,double>> ap, fp;
        Q3 c3, s3; if(type==0){ c3={rat(0),rat(1,2)}; s3={rat(1,2),rat(0)}; } else { c3={rat(1,2),rat(0)}; s3={rat(0),rat(1,2)}; }
        std::pair<Q3,Q3> pQ={Q3(rat(1)),Q3(rat(0))};
        for(int i=0; i<iter; ++i) { auto nQ=std::make_pair(pQ.first*c3-pQ.second*s3, pQ.first*s3+pQ.second*c3); pQ=nQ; ap.push_back({pQ.first.approx(), pQ.second.approx()}); }
        if(GLOBAL_PRECISION==PRE_FLOAT) get_path<float>(fp); else get_path<double>(fp);
        double cx=x()+w()/2.0, cy=y()+h()/2.0, z=250;
        fl_color(FL_RED); for(size_t i=0; i<fp.size()-1; i+=2) fl_line(cx+fp[i].first*z, cy+fp[i].second*z, cx+fp[i+1].first*z, cy+fp[i+1].second*z);
        fl_color(FL_GREEN); for(size_t i=0; i<ap.size()-1; i+=2) fl_line(cx+ap[i].first*z, cy+ap[i].second*z, cx+ap[i+1].first*z, cy+ap[i+1].second*z);
        fl_color(FL_WHITE); fl_draw(("Iter: "+std::to_string(iter)+", P: "+(GLOBAL_PRECISION==PRE_FLOAT?"float32":"float64")+", T: Switch Angle").c_str(), x()+20, y()+30);
        fl_pop_clip();
    }
    int handle(int e) override {
        if(e==FL_PUSH) { Fl::focus(this); return 1; }
        if(e==FL_KEYDOWN) {
            int k=Fl::event_key(); if(k==FL_Up) iter+=500; else if(k==FL_Down && iter>500) iter-=500;
            else if(k=='p') GLOBAL_PRECISION=(GLOBAL_PRECISION==PRE_FLOAT?PRE_DOUBLE:PRE_FLOAT);
            else if(k=='t') type = 1-type;
            redraw(); return 1;
        }
        return Fl_Widget::handle(e);
    }
};

// 5. Cubic
class CubicWidget : public Fl_Widget {
    Qcbrt2 alpha = {rat(1), rat(1), rat(0)}, beta = {rat(0), rat(1), rat(1)}; int mode = 0, idx = 0;
public:
    CubicWidget(int X, int Y, int W, int H) : Fl_Widget(X, Y, W, H) {}
    void draw() override {
        fl_push_clip(x(), y(), w(), h()); fl_color(FL_BLACK); fl_rectf(x(), y(), w(), h());
        auto draw_m = [&](int x, int y, Qcbrt2 v, const char* l, bool act) {
            fl_color(act?FL_CYAN:FL_WHITE); fl_draw(l, x, y);
            auto s = [](Q q) { return q.str(); };
            std::string m[3][3] = {{s(v.a), s(rat(2)*v.c), s(rat(2)*v.b)}, {s(v.b), s(v.a), s(rat(2)*v.c)}, {s(v.c), s(v.b), s(v.a)}};
            for(int i=0; i<3; ++i) for(int j=0; j<3; ++j) { fl_color(FL_WHITE); fl_rect(x+j*80, y+10+i*25, 80, 25); fl_draw(m[i][j].c_str(), x+j*80+5, y+10+i*25+18); }
        };
        draw_m(20, 40, alpha, "Alpha", mode==0); draw_m(20, 160, beta, "Beta", mode==1); draw_m(20, 280, alpha*beta, "Alpha*Beta", false);
        fl_color(FL_GREEN); fl_draw(("Exact Identity: " + std::string(alpha.norm()*beta.norm()==(alpha*beta).norm()?"Verified":"Failed")).c_str(), 20, 400);
        fl_color(FL_WHITE); fl_draw("TAB: Switch, 1/2/3: Coeff, Up/Down: Change", 20, 430);
        fl_pop_clip();
    }
    int handle(int e) override {
        if(e==FL_PUSH) { Fl::focus(this); return 1; }
        if(e==FL_KEYDOWN) {
            int k=Fl::event_key(); if(k==FL_Tab) mode=1-mode; if(k>='1'&&k<='3') idx=k-'1';
            if(k==FL_Up || k==FL_Down) { Qcbrt2 &t=(mode==0?alpha:beta); Q &c=(idx==0?t.a:(idx==1?t.b:t.c)); c=c+(k==FL_Up?rat(1):rat(-1)); }
            redraw(); return 1;
        }
        return Fl_Widget::handle(e);
    }
};

// 6. Polygons
class PolygonWidget : public Fl_Widget {
    int sides = 4; int rots = 0;
public:
    PolygonWidget(int X, int Y, int W, int H) : Fl_Widget(X, Y, W, H) {}
    void draw() override {
        fl_push_clip(x(), y(), w(), h()); fl_color(FL_BLACK); fl_rectf(x(), y(), w(), h());
        double cx=x()+w()/2.0, cy=y()+h()/2.0, z=200;
        if(sides==4) {
            Qi pQ(rat(1),rat(0)), rQ(rat(0),rat(1));
            if(GLOBAL_PRECISION == PRE_FLOAT) {
                float fre=1, fim=0; for(int i=0; i<rots; ++i) { float nr=0*fre-1*fim, ni=1*fre+0*fim; fre=nr; fim=ni; }
                Qi pq2(rat(1),rat(0)); for(int i=0; i<rots%4; ++i) pq2=pq2*rQ; auto pc=pq2.approx_complex();
                fl_color(FL_CYAN); fl_pie(cx+pc.first*z-5, cy-pc.second*z-5, 10, 10, 0, 360); fl_color(FL_RED); fl_pie(cx+fre*z-3, cy-fim*z-3, 6, 6, 0, 360);
            } else {
                double fre=1, fim=0; for(int i=0; i<rots; ++i) { double nr=0*fre-1*fim, ni=1*fre+0*fim; fre=nr; fim=ni; }
                Qi pq2(rat(1),rat(0)); for(int i=0; i<rots%4; ++i) pq2=pq2*rQ; auto pc=pq2.approx_complex();
                fl_color(FL_CYAN); fl_pie(cx+pc.first*z-5, cy-pc.second*z-5, 10, 10, 0, 360); fl_color(FL_RED); fl_pie(cx+fre*z-3, cy-fim*z-3, 6, 6, 0, 360);
            }
        } else {
            Qomega pQ(rat(1),rat(0)), rQ(rat(0),rat(1));
            if(GLOBAL_PRECISION == PRE_FLOAT) {
                float fre=1, fim=0, fc=-0.5f, fs=std::sqrt(3.0f)/2.0f;
                for(int i=0; i<rots; ++i) { float nr=fre*fc-fim*fs, ni=fre*fs+fim*fc; fre=nr; fim=ni; }
                Qomega pq2(rat(1),rat(0)); for(int i=0; i<rots%3; ++i) pq2=pq2*rQ; auto pc=pq2.approx();
                fl_color(FL_CYAN); fl_pie(cx+pc.first*z-5, cy-pc.second*z-5, 10, 10, 0, 360); fl_color(FL_RED); fl_pie(cx+fre*z-3, cy-fim*z-3, 6, 6, 0, 360);
            } else {
                double fre=1, fim=0, fc=-0.5, fs=std::sqrt(3.0)/2.0;
                for(int i=0; i<rots; ++i) { double nr=fre*fc-fim*fs, ni=fre*fs+fim*fc; fre=nr; fim=ni; }
                Qomega pq2(rat(1),rat(0)); for(int i=0; i<rots%3; ++i) pq2=pq2*rQ; auto pc=pq2.approx();
                fl_color(FL_CYAN); fl_pie(cx+pc.first*z-5, cy-pc.second*z-5, 10, 10, 0, 360); fl_color(FL_RED); fl_pie(cx+fre*z-3, cy-fim*z-3, 6, 6, 0, 360);
            }
        }
        fl_color(FL_WHITE); fl_draw(("Sides: "+std::to_string(sides)+" (3/4), Rots: "+std::to_string(rots)+", P: "+(GLOBAL_PRECISION==PRE_FLOAT?"32":"64")).c_str(), x()+20, y()+30);
        fl_pop_clip();
    }
    int handle(int e) override {
        if(e==FL_PUSH) { Fl::focus(this); return 1; }
        if(e==FL_KEYDOWN) { int k=Fl::event_key(); if(k=='3') sides=3; else if(k=='4') sides=4; else if(k==FL_Up) rots+=1000; else if(k==FL_Down&&rots>=1000) rots-=1000; else if(k=='p') GLOBAL_PRECISION=(GLOBAL_PRECISION==PRE_FLOAT?PRE_DOUBLE:PRE_FLOAT); redraw(); return 1; }
        return Fl_Widget::handle(e);
    }
};

// 7. Lattice
class LatticeWidget : public Fl_Widget {
    int type = 0; double scale = 50.0; bool sel = false; std::pair<long long, long long> sp = {0,0};
public:
    LatticeWidget(int X, int Y, int W, int H) : Fl_Widget(X, Y, W, H) {}
    void draw() override {
        fl_push_clip(x(), y(), w(), h()); fl_color(FL_BLACK); fl_rectf(x(), y(), w(), h());
        double cx=x()+w()/2.0, cy=y()+h()/2.0; fl_color(FL_GRAY);
        for(int i=-15; i<=15; ++i) for(int j=-15; j<=15; ++j) {
            double px, py; if(type==0){ px=cx+i*scale; py=cy-j*scale; } else { px=cx+(i-0.5*j)*scale; py=cy-(0.5*std::sqrt(3.0)*j)*scale; }
            fl_pie(px-2, py-2, 4, 4, 0, 360);
        }
        if(sel) {
            double px, py; if(type==0){ px=cx+sp.first*scale; py=cy-sp.second*scale; } else { px=cx+(sp.first-0.5*sp.second)*scale; py=cy-(0.5*std::sqrt(3.0)*sp.second)*scale; }
            fl_color(FL_YELLOW); fl_pie(px-4, py-4, 8, 8, 0, 360);
            std::string s; if(type==0){ Qi v(rat(sp.first), rat(sp.second)); s="Selected Qi: "+v.str()+" Norm: "+(v.a*v.a+v.b*v.b).str(); }
            else { Qomega v(rat(sp.first), rat(sp.second)); s="Selected Qw: "+v.str()+" Norm: "+(v.a*v.a+v.b*v.b-v.a*v.b).str(); }
            fl_color(FL_WHITE); fl_draw(s.c_str(), x()+20, y()+h()-20);
        }
        fl_color(FL_WHITE); fl_draw("1: Qi, 2: Qw, Click to select", x()+20, y()+30);
        fl_pop_clip();
    }
    int handle(int e) override {
        if(e==FL_PUSH) {
            Fl::focus(this); double cx=x()+w()/2.0, cy=y()+h()/2.0, dx=(Fl::event_x()-cx)/scale, dy=(cy-Fl::event_y())/scale;
            if(type==0){ sp={(long long)std::round(dx), (long long)std::round(dy)}; }
            else { double j=dy/(0.5*std::sqrt(3.0)), i=dx+0.5*j; sp={(long long)std::round(i), (long long)std::round(j)}; }
            sel=true; redraw(); return 1;
        }
        if(e==FL_KEYDOWN) { int k=Fl::event_key(); if(k=='1') type=0; if(k=='2') type=1; redraw(); return 1; }
        return Fl_Widget::handle(e);
    }
};

int main() {
    Fl_Double_Window* win = new Fl_Double_Window(1024, 768, "Ultimate Algebraic Demonstration Dashboard");
    Fl_Tabs* tabs = new Fl_Tabs(0, 0, 1024, 768);
    { Fl_Group* g = new Fl_Group(0, 25, 1024, 743, "Theodorus"); new TheodorusWidget(10, 40, 1004, 708); g->end(); }
    { Fl_Group* g = new Fl_Group(0, 25, 1024, 743, "Snowflake"); new SnowflakeWidget(10, 40, 1004, 708); g->end(); }
    { Fl_Group* g = new Fl_Group(0, 25, 1024, 743, "Pell"); new PellWidget(10, 40, 1004, 708); g->end(); }
    { Fl_Group* g = new Fl_Group(0, 25, 1024, 743, "Rotation"); new RotationWidget(10, 40, 1004, 708); g->end(); }
    { Fl_Group* g = new Fl_Group(0, 25, 1024, 743, "Cubic"); new CubicWidget(10, 40, 1004, 708); g->end(); }
    { Fl_Group* g = new Fl_Group(0, 25, 1024, 743, "Polygons"); new PolygonWidget(10, 40, 1004, 708); g->end(); }
    { Fl_Group* g = new Fl_Group(0, 25, 1024, 743, "Lattice"); new LatticeWidget(10, 40, 1004, 708); g->end(); }
    tabs->end(); win->resizable(tabs); win->show();
    return Fl::run();
}
