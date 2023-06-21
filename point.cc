#include "canvas.hh"

#include <cmath>

namespace PW {

namespace {

void lerp(Pixel& a, Pixel const& b, float t) {
  for (auto i = 0u; i < 4u; ++i)
    a[i] += (b[i] - a[i]) * t;
}

template <class T>
T const& max(T const& a, T const& b) {
  return (a < b) ? b : a;
}

auto sqr(float value) -> float { return value * value; }

}

void point(Canvas& canvas, Point point) {
  auto x = point.x - .5f;
  auto y = point.y - .5f;
  auto i0 = static_cast<int>(floor(y));
  auto i1 = i0 + 1;
  auto j0 = static_cast<int>(floor(x));
  auto j1 = j0 + 1;
  auto color = Pixel {255, 255, 0, 255};
  lerp(canvas.data[i0 * canvas.stride + j0], color, max(0.f, 1.f - sqrt(sqr(j0 - x) + sqr(i0 - y))));
  lerp(canvas.data[i0 * canvas.stride + j1], color, max(0.f, 1.f - sqrt(sqr(j1 - x) + sqr(i0 - y))));
  lerp(canvas.data[i1 * canvas.stride + j0], color, max(0.f, 1.f - sqrt(sqr(j0 - x) + sqr(i1 - y))));
  lerp(canvas.data[i1 * canvas.stride + j1], color, max(0.f, 1.f - sqrt(sqr(j1 - x) + sqr(i1 - y))));
}

}
