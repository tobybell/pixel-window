#include "canvas.hh"

#include <cmath>

void blit_rectangle(Canvas& canvas, Point corner, Size size, Dir dir, Pixel color);
void blit_rectangle(Canvas& canvas, Point corner, Size size, Dir dir, LinearGradient const& gradient);
void blit_pie(Canvas& canvas, Point center, float radius, Dir start, Dir end, Pixel color);

constexpr float rounded = 10.f;
constexpr float width = 50.f;
constexpr float height = 30.f;

namespace {

float t = 0.f;

}

void roundRect(Canvas& canvas, Point position) {
  t += 0.1f;
  auto horizontal = Dir {cos(t), sin(t)};
  auto vertical = Dir {-sin(t), cos(t)};
  auto straight_width = width - 2.f * rounded;
  auto straight_height = height - 2.f * rounded;
  auto color = Pixel {255, 255, 0, 0};
  blit_rectangle(canvas, position + horizontal * rounded, {straight_width, rounded}, horizontal, color);
  blit_rectangle(canvas, position + vertical * rounded, {width, straight_height}, horizontal, color);
  blit_rectangle(canvas, position + horizontal * rounded + vertical * (height - rounded), {straight_width, rounded}, horizontal, color);
  blit_pie(canvas, position + (horizontal + vertical) * rounded, rounded, -horizontal, -vertical, color);
  blit_pie(canvas,
    position + horizontal * (width - rounded) + vertical * rounded,
    rounded, -vertical, horizontal, color);
  blit_pie(canvas,
    position + horizontal * rounded + vertical * (height - rounded), rounded,
    vertical, -horizontal, color);
  blit_pie(canvas,
    position + horizontal * (width - rounded) + vertical * (height - rounded),
    rounded, horizontal, vertical, color);
  blit_rectangle(canvas, position + horizontal * rounded, Size {1.f, straight_width}, -vertical, LinearGradient {
    color,
    position,
    -vertical * 1.f,
  });
  blit_rectangle(canvas, position + horizontal * rounded + vertical * height, Size {straight_width, 1.f}, horizontal, LinearGradient {
    color,
    position + vertical * height,
    vertical * 1.f,
  });
  blit_rectangle(canvas, position + vertical * rounded, Size {straight_height, 1.f}, vertical, LinearGradient {
    color,
    position + vertical * rounded,
    -horizontal * 1.f,
  });
  blit_rectangle(canvas, position + horizontal * width + vertical * rounded, Size {1.f, straight_height}, horizontal, LinearGradient {
    color,
    position + horizontal * width,
    horizontal * 1.f,
  });

}
