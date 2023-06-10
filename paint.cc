extern "C" {
#include "header.h"
}

#include <ctime>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <algorithm>

constexpr unsigned square_size = 32;

using u8 = unsigned char;

struct Pixel {
  u8 alpha;
  u8 red;
  u8 green;
  u8 blue;
  u8 const& operator[](unsigned i) const { return (&alpha)[i]; }
  u8& operator[](unsigned i) { return (&alpha)[i]; }
};

struct Canvas {
  Pixel* data;
  unsigned width;
  unsigned height;
  unsigned stride;
};

void clear(Canvas& canvas) {
  for (unsigned i = 0; i < canvas.height; ++i)
    for (unsigned j = 0; j < canvas.width; ++j)
      canvas.data[i * canvas.stride + j] = {255, 255, 255, 255};
}

void randomSquare(Canvas& canvas) {
  unsigned i0 = rand() % (canvas.height - square_size);
  unsigned j0 = rand() % (canvas.width - square_size);
  for (unsigned i = i0; i < i0 + square_size; ++i)
    for (unsigned j = j0; j < j0 + square_size; ++j)
      canvas.data[i * canvas.stride + j] = {255, (unsigned char)(rand() % 255)};
}

unsigned to_pixel(float coord) {
  return static_cast<unsigned>(ceil(coord - .5));
}

unsigned min_pixel(float coord) {
  auto pixel = static_cast<unsigned>(floor(coord + .5));
  return std::max(pixel, 0u);
}

unsigned max_pixel(float coord, unsigned limit) {
  auto pixel = static_cast<unsigned>(ceil(coord - .5));
  return std::min(pixel, limit);
}

float sqr(float value) {
  return value * value;
}

Pixel operator+(Pixel const& lhs, Pixel const& rhs) {
  return {static_cast<unsigned char>(std::min(lhs.alpha + lhs.alpha, 255)),
          static_cast<unsigned char>(std::min(lhs.red + lhs.red, 255)),
          static_cast<unsigned char>(std::min(lhs.green + lhs.green, 255)),
          static_cast<unsigned char>(std::min(lhs.blue + lhs.blue, 255))};
}

Pixel lerp(Pixel const& a, Pixel const& b, float t) {
  Pixel c;
  for (auto i = 0u; i < 4u; ++i)
    c[i] = a[i] + (b[i] - a[i]) * t;
  return c;
}

void circle(Canvas& canvas, float cx, float cy, float radius, Pixel color) {
  auto H = canvas.height;
  auto W = canvas.width;
  auto actual_radius = radius + .5;
  auto inner_radius = radius - .5;

  auto i1 = to_pixel(cy - actual_radius);
  auto i2 = to_pixel(cy + actual_radius);

  auto ir2 = inner_radius * inner_radius;
  auto r2 = actual_radius * actual_radius;

  auto i = i1;
  {
    // top row
    auto y = i + .5f;
    auto v2 = sqr(y - cy);
    auto row_radius = sqrt(r2 - v2);
    auto j1 = to_pixel(cx - row_radius);
    auto j2 = to_pixel(cx + row_radius);
    for (auto j = j1; j < j2; ++j) {
      auto x = j + .5f;
      auto h2 = sqr(x - cx);
      auto point_radius = sqrt(h2 + v2);
      auto t = inner_radius - point_radius + 1;
      auto& pixel = canvas.data[i * canvas.stride + j];
      pixel = lerp(pixel, color, t);
    }
  }

  ++i;
  for (; i + 1 < i2; ++i) {
    auto y = i + .5f;
    auto v2 = sqr(y - cy);
    auto row_radius = sqrt(r2 - v2);
    auto row_inner = sqrt(ir2 - v2);
    auto j1 = to_pixel(cx - row_radius);
    auto j2 = to_pixel(cx + row_radius);
    auto jj1 = to_pixel(cx - row_inner);
    auto jj2 = to_pixel(cx + row_inner);

    for (auto j = j1; j < jj1; ++j) {
      auto x = j + .5f;
      auto h2 = sqr(x - cx);
      auto point_radius = sqrt(h2 + v2);
      auto t = inner_radius - point_radius + 1;
      auto& pixel = canvas.data[i * canvas.stride + j];
      pixel = lerp(pixel, color, t);
    }
    for (auto j = jj1; j < jj2; ++j) {
      auto& pixel = canvas.data[i * canvas.stride + j];
      pixel = color;
    }
    for (auto j = jj2; j < j2; ++j) {
      auto x = j + .5f;
      auto h2 = sqr(x - cx);
      auto point_radius = sqrt(h2 + v2);
      auto t = inner_radius - point_radius + 1;
      auto& pixel = canvas.data[i * canvas.stride + j];
      pixel = lerp(pixel, color, t);
    }
  }

  {
    // bottom row
    auto y = i + .5f;
    auto v2 = sqr(y - cy);
    auto row_radius = sqrt(r2 - v2);
    auto j1 = to_pixel(cx - row_radius);
    auto j2 = to_pixel(cx + row_radius);
    for (auto j = j1; j < j2; ++j) {
      auto x = j + .5f;
      auto h2 = sqr(x - cx);
      auto point_radius = sqrt(h2 + v2);
      auto t = inner_radius - point_radius + 1;
      auto& pixel = canvas.data[i * canvas.stride + j];
      pixel = lerp(pixel, color, t);
    }
  }
}

Pixel randomColor() {
  return {255, static_cast<u8>(arc4random() % 255u), static_cast<u8>(arc4random() % 255u), static_cast<u8>(arc4random() % 255u)};
}

void paint(unsigned* data, unsigned width, unsigned height, unsigned row) {
  unsigned long start = clock();
  Canvas canvas {reinterpret_cast<Pixel*>(data), width, height, row};
  clear(canvas);
  randomSquare(canvas);
  for (auto i = 0u; i < 100; ++i) {
    auto radius = (arc4random() % 100u + 20u) / 5.f;
    auto center_x = width / 4 + arc4random() % width / 2.f;
    auto center_y = height / 4 + arc4random() % height / 2.f;
    circle(canvas, center_x, center_y, radius, randomColor());
  }
  printf("rendered %lu\n", clock() - start);
}
