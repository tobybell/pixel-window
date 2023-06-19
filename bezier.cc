#include "canvas.hh"

#include <cmath>
#include <cstdio>
#include <utility>

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

}

void bezier(Canvas& canvas, float t) {
  auto p0 = Point {canvas.width / 3.f, canvas.height / 3.f} + ofs(.1f * t);
  auto p1 = Point {2.f * canvas.width / 3.f, canvas.height / 3.f} + ofs(.2f * t);
  auto p2 = Point {canvas.width / 3.f, 2.f * canvas.height / 3.f} + ofs(.3f * t);
  auto p3 = Point {2.f * canvas.width / 3.f, 2.f * canvas.height / 3.f} + ofs(.4f * t);
  point(canvas, p0);
  point(canvas, p1);
  point(canvas, p2);
  point(canvas, p3);

  auto dT = pow(.5f, 5.f);
  for (auto T = 0.f; T <= 1.f; T += dT) {
    auto q0 = lerp(p0, p1, T);
    auto q1 = lerp(p1, p2, T);
    auto q2 = lerp(p2, p3, T);
    auto r0 = lerp(q0, q1, T);
    auto r1 = lerp(q1, q2, T);
    auto s0 = lerp(r0, r1, T);
    point(canvas, s0);
  }

}
