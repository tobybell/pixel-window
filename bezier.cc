#include "canvas.hh"

#include <cmath>
#include <cstdio>
#include <utility>

void blit_pie_slice(Canvas& canvas, Point center, float radius, Dir dir0, Dir dir1, Pixel color);

namespace {

constexpr Pixel color {255, 0, 0, 0};

void lerp(Pixel& a, Pixel const& b, float t) {
  for (auto i = 0u; i < 4u; ++i)
    a[i] += (b[i] - a[i]) * t;
}

auto lerp(Point a, Point b, float t) -> Point {
  return {a.x + t * (b.x - a.x), a.y + t * (b.y - a.y)};
}

int to_pixel(float coord) {
  return static_cast<int>(ceil(coord - .5f));
}

auto sqr(float value) -> float { return value * value; }

template <class T>
T const& max(T const& a, T const& b) {
  return (a < b) ? b : a;
}

void point(Canvas& canvas, Point point) {
  auto x = point.x - .5f;
  auto y = point.y - .5f;
  auto i0 = static_cast<int>(floor(y));
  auto i1 = i0 + 1;
  auto j0 = static_cast<int>(floor(x));
  auto j1 = j0 + 1;
  lerp(canvas.data[i0 * canvas.stride + j0], color, max(0.f, 1.f - sqrt(sqr(j0 - x) + sqr(i0 - y))));
  lerp(canvas.data[i0 * canvas.stride + j1], color, max(0.f, 1.f - sqrt(sqr(j1 - x) + sqr(i0 - y))));
  lerp(canvas.data[i1 * canvas.stride + j0], color, max(0.f, 1.f - sqrt(sqr(j0 - x) + sqr(i1 - y))));
  lerp(canvas.data[i1 * canvas.stride + j1], color, max(0.f, 1.f - sqrt(sqr(j1 - x) + sqr(i1 - y))));
}

auto ofs(float theta) -> Point {
  return {10.f * cos(theta), 10.f * sin(theta)};
}

auto len(Point p) -> float {
  return sqrt(p.x * p.x + p.y * p.y);
}

constexpr float thickness = 1.f;

auto abs2(Point p) -> float { return p.x * p.x + p.y * p.y; }

auto dir(Point p) -> Dir {
  auto d = p / sqrt(abs2(p));
  return {d.x, d.y};
}

struct Section {
  Point p[3];
};

}

void bezier(Canvas& canvas, float t) {
  auto p0 = Point {canvas.width / 3.f, canvas.height / 3.f} + ofs(.1f * t);
  auto p1 = Point {2.f * canvas.width / 3.f, canvas.height / 3.f} + ofs(.2f * t);
  auto p2 = Point {canvas.width / 3.f, 2.f * canvas.height / 3.f} + ofs(.3f * t);
  auto p3 = Point {2.f * canvas.width / 3.f, 2.f * canvas.height / 3.f} + ofs(.4f * t);

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
  for (auto T = .02f; T <= 1.f; T += .02f) {
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
    blit_pie_slice(canvas, p0, thickness, -normal, normal, color);
  }

  {
    auto heading = dir(p3 - p2);
    auto normal = Dir {-heading.y, heading.x};
    blit_pie_slice(canvas, p3, thickness, -normal, normal, color);
  }
}
