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
  float x, y;
  Point operator+(Point const& rhs) { return {x + rhs.x, y + rhs.y}; }
  Point operator-(Point const& rhs) { return {x - rhs.x, y - rhs.y}; }
  friend float dot(Point a, Point b) { return a.x * b.x + a.y * b.y; }
};

struct Dir2 {
  float x, y;
  Point operator*(float scale) { return {x * scale, y * scale}; }
  Point operator*(Point const& rhs) {
    return {x * rhs.x - y * rhs.y, x * rhs.y + y * rhs.x};
  }
  Dir2 operator-() const { return {-x, -y}; }
};

struct LinearGradient {
  Pixel color;
  Point position;
  Point direction;
};

// dedup
Pixel lerp(Pixel const& a, Pixel const& b, float t) {
  Pixel c;
  for (auto i = 0u; i < 4u; ++i)
    c[i] = a[i] + (b[i] - a[i]) * t;
  return c;
}

void setrow(Canvas& canvas, u32 i, u32 j0, u32 j1, LinearGradient const& gradient) {
  for (u32 j = j0; j < j1; ++j) {
    auto p = Point {j + .5f, i + .5f};
    auto t = dot(p - gradient.position, gradient.direction);
    auto& curr = canvas.data[i * canvas.stride + j];
    curr = lerp(gradient.color, curr, t);
  }
}

// constexpr Pixel color {255, 255, 0, 0};
// constexpr Pixel red {255, 255, 0, 0};
// constexpr Pixel green {255, 0, 255, 0};

auto dir(float x, float y) -> Dir2 {
  auto d = 1.f / sqrt(x * x + y * y);
  return {d * x, d * y};
}

// dedup
float sqr(float value) { return value * value; }

int tmp_to_pixel(float coord) {
  return static_cast<int>(ceil(coord - .5f));
}

Dir2 make_dir(float t) {
  return {cos(t), sin(t)};
}

void blit_top_rectangle(Canvas& canvas, float x0, float y0, float dx, float dy, LinearGradient const& gradient, float s0, float s1) {
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
    auto y = i + .5f;
    auto dy = y - y0;
    auto j0 = tmp_to_pixel(x0 + dy * slope1);
    auto j1 = tmp_to_pixel(x0 + dy * slope0);
    setrow(canvas, i, j0, j1, gradient);
  }

  auto yref = ia < ib ? y1 : y2;
  auto x0ref = ia < ib ? x0 + slope1 * (s0 * dy) : x2;
  auto x1ref = ia < ib ? x1 : x0 + slope0 * (s1 * dx);
  auto slope = ia < ib ? slope1 : slope0;
  for (auto i = i1; i < i2; ++i) {
    auto y = i + .5f;
    auto dy = y - yref;
    auto j0 = tmp_to_pixel(x0ref + dy * slope);
    auto j1 = tmp_to_pixel(x1ref + dy * slope);
    setrow(canvas, i, j0, j1, gradient);
  }
  for (auto i = i2; i < i3; ++i) {
    auto y = i + .5f;
    auto dy = y - y3;
    auto j0 = tmp_to_pixel(x3 + dy * slope0);
    auto j1 = tmp_to_pixel(x3 + dy * slope1);
    setrow(canvas, i, j0, j1, gradient);
  }
}

auto p90(Dir2 d) -> Dir2 { return {-d.y, d.x}; }
auto m90(Dir2 d) -> Dir2 { return {d.y, -d.x}; }

void blit_rectangle(Canvas& canvas, Point corner, float w, float h, Dir2 dir, Pixel color) {
  auto x = corner.x;
  auto y = corner.y;
  auto color_direction = p90(dir) * (1.f / h);
  auto gradient = LinearGradient {color, corner, color_direction};
  if (dir.x < 0 && dir.y < 0)
    return blit_top_rectangle(canvas, x + w * dir.x - h * dir.y, y + w * dir.y + h * dir.x, -dir.x, -dir.y, gradient, w, h);
  if (dir.y < 0)
    return blit_top_rectangle(canvas, x + w * dir.x, y + w * dir.y, -dir.y, dir.x, gradient, h, w);
  if (dir.x < 0)
    return blit_top_rectangle(canvas, x - h * dir.y, y + h * dir.x, dir.y, -dir.x, gradient, h, w);
  return blit_top_rectangle(canvas, x, y, dir.x, dir.y, gradient, w, h);
}

void blit_pie_slice(Canvas& canvas, Point c, float r, Pixel color, Dir2 dir0, Dir2 dir1) {
  auto cx = c.x;
  auto cy = c.y;
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

void blit_triangle_fragment(Canvas& canvas, Point anchor, float left_slope, float right_slope, u32 i0, u32 i1, Pixel color) {
  for (auto i = i0; i < i1; ++i) {
    auto y = i + .5f - anchor.y;
    auto j1 = tmp_to_pixel(anchor.x + y * left_slope);
    auto j2 = tmp_to_pixel(anchor.x + y * right_slope);
    setrow(canvas, i, j1, j2, color);
  }
}

void blit_top_triangle(Canvas& canvas, Point a, Point b, Point c, Pixel color) {
  auto i0 = tmp_to_pixel(a.y);
  auto i1 = tmp_to_pixel(b.y);
  auto i2 = tmp_to_pixel(c.y);
  auto ab = (b.x - a.x) / (b.y - a.y);
  auto ac = (c.x - a.x) / (c.y - a.y);
  auto bc = (c.x - b.x) / (c.y - b.y);
  if (ab < ac) {
    blit_triangle_fragment(canvas, a, ab, ac, i0, i1, color);
    blit_triangle_fragment(canvas, c, bc, ac, i1, i2, color);
  } else {
    blit_triangle_fragment(canvas, a, ac, ab, i0, i1, color);
    blit_triangle_fragment(canvas, c, ac, bc, i1, i2, color);
  }
}

void blit_triangle(Canvas& canvas, Point a, Point b, Point c, Pixel color) {
  if (a.y < b.y) {
    if (b.y < c.y)
      return blit_top_triangle(canvas, a, b, c, color);
    else if (a.y < c.y)
      return blit_top_triangle(canvas, a, c, b, color);
    return blit_top_triangle(canvas, c, a, b, color);
  }
  if (a.y < c.y)
    return blit_top_triangle(canvas, b, a, c, color);
  if (b.y < c.y)
    return blit_top_triangle(canvas, b, c, a, color);
  return blit_top_triangle(canvas, c, b, a, color);
}

auto dir_from_to(Point a, Point b) -> Dir2 {
  return dir(b.x - a.x, b.y - a.y);
}

auto len(Point p) -> float {
  return sqrt(p.x * p.x + p.y * p.y);
}

}

void triangle(Canvas& canvas, float t) {
  auto tt = .1f * t;
  auto th1 = tt;
  auto dir0 = make_dir(th1);

  auto one_third = Dir2 {-.5f, .5f * sqrt(3.f)};

  auto center = Point {.5f * canvas.width, .5f * canvas.height};
  auto color = Pixel {255, 255, 0, 0};

  auto blur = 6.f + 5.f * sin(t);
  auto triangle_radius = .35f * canvas.width;
  auto offset = dir0 * triangle_radius;
  auto a0 = center + offset;
  auto b0 = center + one_third * offset;
  auto c0 = center + one_third * (one_third * offset);

  auto a = a0 + make_dir(.5f * t) * 5.f;
  auto b = b0 + make_dir(.8f * t + 1.f) * 5.f;
  auto c = c0 + make_dir(.6f * t + 2.f) * 5.f;

  blit_triangle(canvas, c, a, b, color);

  auto vab = b - a;
  auto vac = c - a;
  auto vbc = c - b;

  auto ab = dir_from_to(a, b);
  auto ac = dir_from_to(a, c);
  auto bc = dir_from_to(b, c);

  blit_pie_slice(canvas, a, blur, color, p90(ac), m90(ab));
  blit_pie_slice(canvas, b, blur, color, p90(-ab), m90(bc));
  blit_pie_slice(canvas, c, blur, color, p90(-bc), m90(-ac));

  blit_rectangle(canvas, a, len(vac), blur, ac, color);
  blit_rectangle(canvas, c, len(vbc), blur, -bc, color);
  blit_rectangle(canvas, b, len(vab), blur, -ab, color);
}
