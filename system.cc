extern "C" {
#include "header.h"
}

#include "canvas.hh"

#include <ctime>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <algorithm>
#include <new>

void triangle(Canvas& canvas, float t, Pixel color);
void quad(Canvas&, Point const (&points)[4], Pixel color);

namespace {

constexpr unsigned square_size = 32;

using std::exchange;

struct Blob {
  Blob(): data() {}
  Blob(unsigned size);
  Blob(Blob const&) = delete;
  Blob(Blob&& other): data(exchange(other.data, nullptr)) {}
  Blob(Blob&& other, unsigned size);
  ~Blob();
  void operator=(Blob&& other) { data = exchange(other.data, nullptr); }
  char& operator[](unsigned i) { return data[i]; }
  char const& operator[](unsigned i) const { return data[i]; }

private:
  char* data;
};

Blob::Blob(unsigned size): data((char*) malloc(size)) {}

Blob::Blob(Blob&& other, unsigned size):
  data((char*) realloc(exchange(other.data, nullptr), size)) {}

Blob::~Blob() { free(exchange(data, nullptr)); }

template <class T>
constexpr bool is_trivial = std::is_trivial_v<T>;

template <class T>
struct List {
  static_assert(is_trivial<T>);
  void push(T const& x) {
    expand(count + 1);
    begin()[count++] = x;
  }
  T const& operator[](u32 index) const { return begin()[index]; }
  T* begin() { return reinterpret_cast<T*>(&data[0]); }
  T const* begin() const { return reinterpret_cast<T const*>(&data[0]); }
  T* end() { return begin() + count; }
  T const* end() const { return begin() + count; }
  friend u32 len(List const& list) { return list.count; }
private:
  Blob data;
  u32 count {};
  u32 capacity {};
  T* at(u32 index) { return reinterpret_cast<T*>(&data[0]) + index; }
  void expand(u32 needed) {
    if (capacity >= needed)
      return;
    if (!capacity) {
      capacity = needed;
    } else {
      while (capacity < needed)
        capacity *= 2;
    }
    data = Blob(std::move(data), capacity * sizeof(T));
  }
};

void clear(Canvas& canvas) {
  for (unsigned i = 0; i < canvas.height; ++i)
    for (unsigned j = 0; j < canvas.width; ++j)
      canvas.data[i * canvas.stride + j] = {255, 255, 255, 255};
}

[[maybe_unused]] void randomSquare(Canvas& canvas) {
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

Pixel lerp(Pixel const& a, Pixel const& b, float t) {
  Pixel c;
  for (auto i = 0u; i < 4u; ++i)
    c[i] = a[i] + (b[i] - a[i]) * t;
  return c;
}

void circle(Canvas& canvas, float cx, float cy, float radius, Pixel color) {
  int H = canvas.height;
  int W = canvas.width;
  auto outer_radius = radius + .5;
  auto inner_radius = radius - .5;
  auto outer_radius_squared = sqr(outer_radius);
  auto inner_radius_squared = sqr(inner_radius);

  auto edgeRange = [&](u32 i, u32 j1, u32 j2, float y2) {
    for (auto j = j1; j < j2; ++j) {
      auto x2 = sqr(j + .5f - cx);
      auto point_radius = sqrt(x2 + y2);
      auto t = point_radius - inner_radius;
      auto& pixel = canvas.data[i * canvas.stride + j];
      pixel = lerp(color, pixel, t);
    }
  };
  auto edgeRow = [&](u32 i) {
    auto y2 = sqr(i + .5f - cy);
    auto width = sqrt(outer_radius_squared - y2);
    auto j1 = to_pixel(cx - width, W);
    auto j2 = to_pixel(cx + width, W);
    edgeRange(i, j1, j2, y2);
  };

  auto i1 = to_pixel(cy - outer_radius, H);
  auto i2 = to_pixel(cy - inner_radius, H);
  auto i3 = to_pixel(cy + inner_radius, H);
  auto i4 = to_pixel(cy + outer_radius, H);

  for (auto i = i1; i < i2; ++i)
    edgeRow(i);
  for (auto i = i2; i < i3; ++i) {
    auto y2 = sqr(i + .5f - cy);
    auto outer_width = sqrt(outer_radius_squared - y2);
    auto inner_width = sqrt(inner_radius_squared - y2);
    auto j1 = to_pixel(cx - outer_width, W);
    auto j2 = to_pixel(cx - inner_width, W);
    auto j3 = to_pixel(cx + inner_width, W);
    auto j4 = to_pixel(cx + outer_width, W);
    edgeRange(i, j1, j2, y2);
    for (auto j = j2; j < j3; ++j)
      canvas.data[i * canvas.stride + j] = color;
    edgeRange(i, j3, j4, y2);
  }
  for (auto i = i3; i < i4; ++i)
    edgeRow(i);
}

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

constexpr Pixel red {255, 255, 0, 0};

struct System {
  unsigned long last = clock();
  float t = 0.;

  struct Circle {
    float x;
    float y;
  };
  List<Circle> circles;

  void paint(unsigned* data, unsigned width, unsigned height, unsigned row) {
    unsigned long start = clock();
    Canvas canvas {reinterpret_cast<Pixel*>(data), width, height, row};
    clear(canvas);
    // randomSquare(canvas);

    t += (start - last) / 200000.f;
    last = start;

    triangle(canvas, t, red);
    for (auto i = 0u; i < len(circles); ++i) {
      auto& parameters = circles[i];
      auto radius = (noise(i, 0) % 100u + 20u) / 5.f;
      auto phase = (noise(i, 1) % 628) / 100.f;
      auto speed = (noise(i, 2) % 200) / 100.f;
      auto center_x = parameters.x + 10.f * sinf(speed * t + phase);
      auto center_y = parameters.y + 10.f * cosf(speed * t + phase);
      auto color = colorNoise(i, 5);
      circle(canvas, center_x, center_y, radius, color);
    }

//    triangle(canvas, t + 10.f);
//    printf("rendered %lu\n", clock() - start);
  }
  void mouseDown(float x, float y) {
    printf("sys mousedown %f %f\n", x, y);
    circles.push({x, y});
  }
};

System* cast(void* pointer) { return reinterpret_cast<System*>(pointer); }

}

void* sysInit() {
  auto sys = malloc(sizeof(System));
  new (cast(sys)) System {};
  return sys;
}
void sysKill(void* sys) {
  cast(sys)->~System();
  free(sys);
}
void sysPaint(void* sys, unsigned* data, unsigned width, unsigned height, unsigned stride) {
  return cast(sys)->paint(data, width, height, stride);
}
void sysMouseDown(void* sys, float x, float y) {
  return cast(sys)->mouseDown(x, y);
}
