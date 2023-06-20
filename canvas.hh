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
  Point operator+(Point const& rhs) { return {x + rhs.x, y + rhs.y}; }
  Point operator-(Point const& rhs) { return {x - rhs.x, y - rhs.y}; }
  Point operator*(float scale) { return {x * scale, y * scale}; }
  Point operator/(float scale) { return *this * (1.f / scale); }
  friend float dot(Point a, Point b) { return a.x * b.x + a.y * b.y; }
};

struct LinearGradient {
  Pixel color;
  Point position;
  Point direction;
};

struct Dir {
  float x, y;
  Point operator*(float scale) { return {x * scale, y * scale}; }
  Point operator*(Point const& rhs) {
    return {x * rhs.x - y * rhs.y, x * rhs.y + y * rhs.x};
  }
  Dir operator-() const { return {-x, -y}; }
  Point operator+(Dir const& rhs) { return {x + rhs.x, y + rhs.y}; }
  Point operator-(Dir const& rhs) { return {x - rhs.x, y - rhs.y}; }
};

void blit_triangle(Canvas& canvas, Point a, Point b, Point c, LinearGradient const& fill);
