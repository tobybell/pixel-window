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

unsigned to_pixel(float coord, unsigned limit) {
  auto grid = static_cast<int>(ceil(coord - .5));
  return grid < 0 ? 0 : grid > limit ? limit : grid;
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
  int H = canvas.height;
  int W = canvas.width;
  auto actual_radius = radius + .5;
  auto inner_radius = radius - .5;

  auto i1 = to_pixel(cy - actual_radius, H);
  auto i2 = to_pixel(cy - inner_radius, H);
  auto i3 = to_pixel(cy + inner_radius, H);
  auto i4 = to_pixel(cy + actual_radius, H);

  auto ir2 = inner_radius * inner_radius;
  auto r2 = actual_radius * actual_radius;

  for (auto i = i1; i < i2; ++i) {
    auto y = i + .5f;
    auto v2 = sqr(y - cy);
    auto row_radius = sqrt(r2 - v2);
    auto j1 = to_pixel(cx - row_radius, W);
    auto j2 = to_pixel(cx + row_radius, W);
    for (auto j = j1; j < j2; ++j) {
      auto x = j + .5f;
      auto h2 = sqr(x - cx);
      auto point_radius = sqrt(h2 + v2);
      auto t = inner_radius - point_radius + 1;
      auto& pixel = canvas.data[i * canvas.stride + j];
      pixel = lerp(pixel, color, t);
    }
  }
  for (auto i = i2; i < i3; ++i) {
    auto y = i + .5f;
    auto v2 = sqr(y - cy);
    auto row_radius = sqrt(r2 - v2);
    auto row_inner = sqrt(ir2 - v2);
    auto j1 = to_pixel(cx - row_radius, W);
    auto j2 = to_pixel(cx + row_radius, W);
    auto jj1 = to_pixel(cx - row_inner, W);
    auto jj2 = to_pixel(cx + row_inner, W);

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
  for (auto i = i3; i < i4; ++i) {
    auto y = i + .5f;
    auto v2 = sqr(y - cy);
    auto row_radius = sqrt(r2 - v2);
    auto j1 = to_pixel(cx - row_radius, W);
    auto j2 = to_pixel(cx + row_radius, W);
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

unsigned long last = clock();
float t = 0.;

// SquirrelNoise5 by Squirrel Eiserloh
constexpr unsigned int noise(int positionX, unsigned int seed) {
	constexpr unsigned int SQ5_BIT_NOISE1 = 0xd2a80a3f;
	constexpr unsigned int SQ5_BIT_NOISE2 = 0xa884f197;
	constexpr unsigned int SQ5_BIT_NOISE3 = 0x6C736F4B;
	constexpr unsigned int SQ5_BIT_NOISE4 = 0xB79F3ABB;
	constexpr unsigned int SQ5_BIT_NOISE5 = 0x1b56c4f5;

	unsigned int mangledBits = (unsigned int) positionX;
	mangledBits *= SQ5_BIT_NOISE1;
	mangledBits += seed;
	mangledBits ^= (mangledBits >> 9);
	mangledBits += SQ5_BIT_NOISE2;
	mangledBits ^= (mangledBits >> 11);
	mangledBits *= SQ5_BIT_NOISE3;
	mangledBits ^= (mangledBits >> 13);
	mangledBits += SQ5_BIT_NOISE4;
	mangledBits ^= (mangledBits >> 15);
	mangledBits *= SQ5_BIT_NOISE5;
	mangledBits ^= (mangledBits >> 17);
	return mangledBits;
}

Pixel colorNoise(int position, unsigned int seed) {
  return {255, static_cast<u8>(noise(3 * position, seed) % 255u),
               static_cast<u8>(noise(3 * position + 1, seed) % 255u),
               static_cast<u8>(noise(3 * position + 2, seed) % 255u)};
}

void paint(unsigned* data, unsigned width, unsigned height, unsigned row) {
  unsigned long start = clock();
  Canvas canvas {reinterpret_cast<Pixel*>(data), width, height, row};
  clear(canvas);
  // randomSquare(canvas);

  t += (start - last) / 200000.f;
  last = start;

  for (auto i = 0u; i < 100; ++i) {
    auto radius = (noise(i, 0) % 100u + 20u) / 5.f;
    auto phase = (noise(i, 1) % 628) / 100.f;
    auto speed = (noise(i, 2) % 200) / 100.f;
    auto center_x = noise(i, 3) % width + 10.f * sinf(speed * t + phase);
    auto center_y = noise(i, 4) % height + 10.f * cosf(speed * t + phase);
    auto color = colorNoise(i, 5);
    circle(canvas, center_x, center_y, radius, color);
  }
  printf("rendered %lu\n", clock() - start);
}
