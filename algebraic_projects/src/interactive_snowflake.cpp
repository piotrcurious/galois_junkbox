#include "../include/algebraic_field.hpp"
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/fl_draw.H>
#include <vector>
#include <cmath>
#include <iostream>

struct Vec2Q { Q3 x, y; };

class SnowflakeWidget : public Fl_Widget {
    double zoom;
    double offset_x, offset_y;
    int depth;
public:
    SnowflakeWidget(int X, int Y, int W, int H) : Fl_Widget(X, Y, W, H), zoom(1.0), offset_x(0), offset_y(0), depth(4) {}

    void draw() override {
        fl_push_clip(x(), y(), w(), h());
        fl_color(FL_BLACK);
        fl_rectf(x(), y(), w(), h());

        double cx = x() + w() / 2.0 + offset_x;
        double cy = y() + h() / 2.0 + offset_y;

        const double side_len = 600.0;
        // Side 600 means height is 300*sqrt(3).
        // Center the triangle vertically: y from -100*sqrt(3) to 200*sqrt(3)
        // or just use 150*sqrt(3) as half-height.
        // Q3(rat(0), rat(150)) represents 0 + 150*sqrt(3).

        Vec2Q p1 = { Q3(rat(-300)), Q3(rat(0), rat(150))  };
        Vec2Q p2 = { Q3(rat(300)),  Q3(rat(0), rat(150))  };
        Vec2Q p3 = { Q3(rat(0)),    Q3(rat(0), rat(-150)) };

        std::vector<Vec2Q> pts = { p1, p3, p2, p1 };

        auto step = [](std::vector<Vec2Q>& points) {
            std::vector<Vec2Q> next;
            Q3 one3 = Q3(rat(1, 3));
            Q3 half = Q3(rat(1, 2));
            Q3 s3_6 = Q3(rat(0), rat(1, 6));

            for (size_t i = 0; i < points.size() - 1; i++) {
                Vec2Q p1 = points[i];
                Vec2Q p2 = points[i+1];
                Q3 dx = p2.x - p1.x;
                Q3 dy = p2.y - p1.y;

                Vec2Q a = { p1.x + one3 * dx, p1.y + one3 * dy };
                Vec2Q c = { p1.x + Q3(rat(2, 3)) * dx, p1.y + Q3(rat(2, 3)) * dy };
                Vec2Q b = {
                    half * (p1.x + p2.x) + s3_6 * dy,
                    half * (p1.y + p2.y) - s3_6 * dx
                };
                next.push_back(p1);
                next.push_back(a);
                next.push_back(b);
                next.push_back(c);
            }
            next.push_back(points.back());
            points = next;
        };

        for (int i = 0; i < depth; i++) step(pts);

        fl_color(FL_CYAN);
        for (size_t i = 0; i < pts.size() - 1; i++) {
            fl_line(cx + pts[i].x.approx() * zoom, cy + pts[i].y.approx() * zoom,
                    cx + pts[i+1].x.approx() * zoom, cy + pts[i+1].y.approx() * zoom);
        }

        fl_color(FL_WHITE);
        fl_font(FL_HELVETICA, 14);
        fl_draw(("Zoom: " + std::to_string(zoom)).c_str(), x() + 10, y() + 20);
        fl_draw("Scroll to zoom, Drag to pan. Depth: 4", x() + 10, y() + 40);

        fl_pop_clip();
    }

    int handle(int event) override {
        static int last_x, last_y;
        switch (event) {
            case FL_PUSH:
                last_x = Fl::event_x();
                last_y = Fl::event_y();
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
        }
        return Fl_Widget::handle(event);
    }
};

int main() {
    Fl_Double_Window* win = new Fl_Double_Window(800, 600, "Interactive Exact Snowflake");
    new SnowflakeWidget(0, 0, 800, 600);
    win->resizable(win);
    win->show();
    return Fl::run();
}
