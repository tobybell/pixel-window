#include "canvas.hh"

#include <cmath>
#include <cstdio>
#include <utility>

void blit_pie(Canvas& canvas, Point center, float radius, Dir dir0, Dir dir1, RadialGradient const&);

namespace {

constexpr Pixel color {255, 0, 0, 0};

constexpr float thickness = 1.f;

auto dir(Point p) -> Dir {
  auto d = p / sqrt(abs2(p));
  return {d.x, d.y};
}

struct Section {
  Point p[3];
};

}

void bezier(Canvas& canvas, Point p0, Point p1, Point p2, Point p3) {
  auto P0 = p0;
  auto P1 = (p1 - p0) * 3.f;
  auto P2 = (p0 - p1 * 2.f + p2) * 3.f;
  auto P3 = (p1 - p2) * 3.f + p3 - p0;

  auto eval = [&](float T) -> Section {
    auto T2 = T * T;
    auto T3 = T2 * T;

    auto center = P0 + P1 * T + P2 * T2 + P3 * T3;
    auto velocity = P1 + P2 * (2.f * T) + P3 * (3.f * T2);
    auto heading = dir(velocity);
    auto normal = Point {-heading.y, heading.x};

    return {center + normal * thickness, center, center - normal * thickness};
  };

  auto last = eval(0.f);
  auto dt = pow(.5f, 6.f);
  for (auto T = dt; T <= 1.f; T += dt) {
    auto next = eval(T);
    auto avg = Section {
      (last.p[0] + next.p[0]) * .5f,
      (last.p[1] + next.p[1]) * .5f,
      (last.p[2] + next.p[2]) * .5f};
    auto d0 = avg.p[0] - avg.p[1];
    auto d1 = avg.p[2] - avg.p[1];
    auto gradient0 = LinearGradient {color, avg.p[1], d0 / abs2(d0)};
    auto gradient1 = LinearGradient {color, avg.p[1], d1 / abs2(d1)};
    blit_triangle(canvas, last.p[0], last.p[1], next.p[0], gradient0);
    blit_triangle(canvas, next.p[1], last.p[1], next.p[0], gradient0);
    blit_triangle(canvas, last.p[1], last.p[2], next.p[1], gradient1);
    blit_triangle(canvas, next.p[2], last.p[2], next.p[1], gradient1);
    last = next;
  }

  {
    auto heading = dir(p0 - p1);
    auto normal = Dir {-heading.y, heading.x};
    blit_pie(canvas, p0, thickness, -normal, normal, {color, p0, thickness, -thickness});
  }

  {
    auto heading = dir(p3 - p2);
    auto normal = Dir {-heading.y, heading.x};
    blit_pie(canvas, p3, thickness, -normal, normal, {color, p3, thickness, -thickness});
  }
}
