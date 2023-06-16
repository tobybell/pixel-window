#include "canvas.hh"

#include <cmath>
#include <cstdio>
#include <utility>

namespace {

using std::exchange;

void check(bool condition) {
  if (!condition)
    abort();
}

template <class T>
T const& max(T const& a, T const& b) {
  return (a < b) ? b : a;
}

template <class T>
T const& min(T const& a, T const& b) {
  return (a < b) ? a : b;
}

void setrow(Canvas& canvas, u32 i, u32 j1, u32 j2, Pixel color) {
  for (u32 j = j1; j < j2; ++j)
    canvas.data[i * canvas.stride + j] = color;
}

struct Point {
  float x;
  float y;
};

constexpr Point top {50.f, 10.f};
constexpr Point left {10.f, 60.f};
constexpr Point right {80.f, 90.f};

constexpr Pixel color {255, 255, 0, 0};
// constexpr Pixel red {255, 255, 0, 0};
// constexpr Pixel green {255, 0, 255, 0};

struct Dir2 {
  float x, y;
};

auto dir(float x, float y) -> Dir2 {
  auto d = 1.f / sqrt(x * x + y * y);
  return {d * x, d * y};
}

// dedup
float sqr(float value) { return value * value; }

int tmp_to_pixel(float coord) {
  return static_cast<int>(ceil(coord - .5f));
}

// dedup
Pixel lerp(Pixel const& a, Pixel const& b, float t) {
  Pixel c;
  for (auto i = 0u; i < 4u; ++i)
    c[i] = a[i] + (b[i] - a[i]) * t;
  return c;
}

Dir2 make_slope(float t) {
  return {cos(t), sin(t)};
}

void blit_top_rectangle(Canvas& canvas, float x0, float y0, float dx, float dy, float s0, float s1) {
  check(dx >= 0.f);
  check(dy >= 0.f);

  auto dx0 = s0 * dx;
  auto dy0 = s0 * dy;
  auto dx1 = -s1 * dy;
  auto dy1 = s1 * dx;

  auto slope0 = dx / dy;
  auto slope1 = -dy / dx;

  auto y1 = y0 + dy0;
  auto y2 = y0 + dy1;
  auto y3 = y2 + dy0;
  auto x1 = x0 + dx0;
  auto x2 = x0 + dx1;
  auto x3 = x1 + dx1;

  auto i0 = tmp_to_pixel(y0);
  auto ia = tmp_to_pixel(y1);
  auto ib = tmp_to_pixel(y2);
  auto i3 = tmp_to_pixel(y3);

  auto i1 = min(ia, ib);
  auto i2 = max(ia, ib);

  for (auto i = i0; i < i1; ++i) {
    auto y = i + .5f - y0;
    auto j0 = tmp_to_pixel(x0 + y * slope1);
    auto j1 = tmp_to_pixel(x0 + y * slope0);
    for (auto j = j0; j < j1; ++j) {
      canvas.data[i * canvas.stride + j] = color;
    }
  }

  auto yref = ia < ib ? y1 : y2;
  auto x0ref = ia < ib ? x0 + slope1 * (s0 * dy) : x2;
  auto x1ref = ia < ib ? x1 : x0 + slope0 * (s1 * dx);
  auto slope = ia < ib ? slope1 : slope0;
  for (auto i = i1; i < i2; ++i) {
    auto y = i + .5f - yref;
    auto j0 = tmp_to_pixel(x0ref + y * slope);
    auto j1 = tmp_to_pixel(x1ref + y * slope);
    for (auto j = j0; j < j1; ++j) {
      canvas.data[i * canvas.stride + j] = color;
    }
  }
  for (auto i = i2; i < i3; ++i) {
    auto y = i + .5f - y3;
    auto j0 = tmp_to_pixel(x3 + y * slope0);
    auto j1 = tmp_to_pixel(x3 + y * slope1);
    for (auto j = j0; j < j1; ++j) {
      canvas.data[i * canvas.stride + j] = color;
    }
    
  }
}

void blit_rectangle(Canvas& canvas, Dir2 dir) {
  // 3 regime

  auto s0 = 30.f;
  auto s1 = 20.f;

  auto x0 = 45.f;
  auto y0 = 45.f;

  if (dir.x < 0 && dir.y < 0)
    return blit_top_rectangle(canvas, x0 + s0 * dir.x - s1 * dir.y, y0 + s0 * dir.y + s1 * dir.x, -dir.x, -dir.y, s0, s1);
  if (dir.y < 0)
    return blit_top_rectangle(canvas, x0 + s0 * dir.x, y0 + s0 * dir.y, -dir.y, dir.x, s1, s0);
  if (dir.x < 0)
    return blit_top_rectangle(canvas, x0 - s1 * dir.y, y0 + s1 * dir.x, dir.y, -dir.x, s1, s0);
  return blit_top_rectangle(canvas, x0, y0, dir.x, dir.y, s0, s1);
}

void blit_pie_slice(Canvas& canvas, float cx, float cy, float r, Pixel color, Dir2 dir0, Dir2 dir1) {
  auto r2 = sqr(r);
  auto i_min = tmp_to_pixel(cy - r);
  auto i_mid = tmp_to_pixel(cy);
  auto i_max = tmp_to_pixel(cy + r);

  auto i0 = tmp_to_pixel(cy + r * dir0.y);
  auto i1 = tmp_to_pixel(cy + r * dir1.y);

  auto edge0 = [&, slope0 = dir0.x / dir0.y](float y) { return cx + y * slope0; };
  auto edge1 = [&, slope1 = dir1.x / dir1.y](float y) { return cx + y * slope1; };
  auto arc0 = [&](float y) { return cx - sqrt(r2 - y * y); };
  auto arc1 = [&](float y) { return cx + sqrt(r2 - y * y); };

  auto row = [&canvas, &cx, &r, &color](u32 i, float y2, float x0, float x1) {
    auto j0 = tmp_to_pixel(x0);
    auto j1 = tmp_to_pixel(x1);
    for (auto j = j0; j < j1; ++j) {
      auto x2 = sqr(j + .5f - cx);
      auto d = sqrt(x2 + y2) / r;
      auto& curr = canvas.data[i * canvas.stride + j];
      curr = lerp(color, curr, d);
    }
  };

  auto ream = [&](u32 i0, u32 i1, auto& left, auto& right) {
    for (auto i = i0; i < i1; ++i) {
      auto y = i + .5f - cy;
      row(i, y * y, left(y), right(y));
    }
  };

  ream(dir1.x < dir0.x ? max(i_mid, i0) : i_mid, i1, edge1, arc1);
  ream(max(i0, i1), i_mid, edge0, edge1);
  ream(i_mid, min(i0, i1), edge1, edge0);
  if (dir0.x < 0 ? (dir1.x < 0 && dir0.y < dir1.y) : (dir1.x < 0 || dir1.y < dir0.y))
    ream(max(i_mid, max(i1, i0)), i_max, arc0, arc1);
  if (dir0.x < 0 ? (dir1.x > 0 || dir1.y > dir0.y) : (dir1.x > 0 && dir1.y < dir0.y))
    ream(i_min, min(i_mid, min(i1, i0)), arc0, arc1);
  ream(dir1.x < dir0.x ? max(i1, i_mid) : i_mid, i0, arc0, edge0);
  ream(min(i1, i_mid), dir0.x < dir1.x ? min(i_mid, i0) : i_mid, arc0, edge1);
  ream(min(i0, i_mid), dir0.x < dir1.x ? min(i1, i_mid) : i_mid, edge0, arc1);
}

void blit_triangle(Canvas& canvas) {
  auto i1 = static_cast<int>(ceil(top.y - .5f));
  auto mid = static_cast<int>(ceil(left.y - .5f));
  auto bottom = static_cast<int>(ceil(right.y - .5f));

  auto left_slope = (left.x - top.x) / (left.y - top.y);
  auto right_slope = (right.x - top.x) / (right.y - top.y);
  auto bottom_slope = (left.x - right.x) / (left.y - right.y);
  for (u32 i = i1; i < mid; ++i) {
    auto y = i + .5f - top.y;
    auto x1 = top.x + y * left_slope;
    auto x2 = top.x + y * right_slope;
    auto j1 = static_cast<int>(ceil(x1 - .5f));
    auto j2 = static_cast<int>(ceil(x2 - .5f));
    setrow(canvas, i, j1, j2, color);
  }
  for (u32 i = mid; i < bottom; ++i) {
    auto y = i + .5f - right.y;
    auto x1 = right.x + y * bottom_slope;
    auto x2 = right.x + y * right_slope;
    auto j1 = static_cast<int>(ceil(x1 - .5f));
    auto j2 = static_cast<int>(ceil(x2 - .5f));
    setrow(canvas, i, j1, j2, color);
  }
}

float t = 0.f;

}

void triangle(Canvas& canvas) {
  t += .01f;
  auto th1 = t;
  auto th2 = 1.3f * t + .5f;
  auto dir0 = make_slope(th1);
  auto dir1 = make_slope(th2);

  // blit_triangle(canvas);
  // blit_pie_slice(canvas, 45., 45., 30., color, dir0, dir1);
  blit_rectangle(canvas, dir0);
}
