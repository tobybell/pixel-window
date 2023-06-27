#pragma once

using u8 = unsigned char;
using u32 = unsigned int;

struct Pixel {
  u8 alpha;
  u8 red;
  u8 green;
  u8 blue;
  u8 const& operator[](u32 i) const { return (&alpha)[i]; }
  u8& operator[](u32 i) { return (&alpha)[i]; }
};

struct Canvas {
  Pixel* data;
  u32 width;
  u32 height;
  u32 stride;
};

struct Point {
  float x, y;
  Point operator+(Point const& rhs) const { return {x + rhs.x, y + rhs.y}; }
  Point operator-(Point const& rhs) const { return {x - rhs.x, y - rhs.y}; }
  Point operator*(float scale) const { return {x * scale, y * scale}; }
  Point operator/(float scale) const { return *this * (1.f / scale); }
  friend float dot(Point const& a, Point const& b) {
    return a.x * b.x + a.y * b.y;
  }
  friend float abs2(Point const& p) { return dot(p, p); }
};

struct LinearGradient {
  enum { fill_type = 2 };
  Pixel color;
  Point position;
  Point direction;
};

struct RadialGradient {
  enum { fill_type = 0 };
  Pixel color;
  Point position;
  float start_radius;
  float thickness;
};

struct Solid {
  enum { fill_type = 1 };
  Pixel color;
};

struct Dir {
  float x, y;
  Point operator*(float scale) { return {x * scale, y * scale}; }
  Point operator*(Point const& rhs) {
    return {x * rhs.x - y * rhs.y, x * rhs.y + y * rhs.x};
  }
  Dir operator-() const { return {-x, -y}; }
  Point operator+(Dir const& rhs) const { return {x + rhs.x, y + rhs.y}; }
  Point operator-(Dir const& rhs) const { return {x - rhs.x, y - rhs.y}; }
  friend float dot(Point const& a, Dir const& b) {
    return a.x * b.x + a.y * b.y;
  }
};

struct Size {
  float x, y;
  Point operator*(Point const& rhs) const { return {x * rhs.x, y * rhs.y}; }
  friend Point operator/(Point const& lhs, Size const& rhs) {
    return {lhs.x / rhs.x, lhs.y / rhs.y};
  }
  Size operator/(Size const& rhs) const { return {x / rhs.x, y / rhs.y}; }
};

void blit_triangle(Canvas& canvas, Point a, Point b, Point c, Pixel color);
void blit_triangle(Canvas& canvas, Point a, Point b, Point c, LinearGradient const& fill);

template <class T>
T const& max(T const& a, T const& b) { return (a < b) ? b : a; }

template <class T>
T const& min(T const& a, T const& b) { return (a < b) ? a : b; }

namespace PW {

void point(Canvas& canvas, Point point);

inline auto cross(Dir a, Dir b) { return a.x * b.y - a.y * b.x; }

}
