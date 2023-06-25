#include "canvas.hh"
#include "edges.hh"

#include <cmath>

void blit_rectangle(Canvas& canvas, Point corner, Size size, Dir dir, Pixel color);
void blit_rectangle(Canvas& canvas, Point corner, Size size, Dir dir, LinearGradient const& gradient);
void blit_pie(Canvas& canvas, Point center, float radius, Dir start, Dir end, Pixel color);

namespace PW {

void blit_ring(AllEdges&, Point center, float inner_radius, float outer_radius, Dir begin, Dir end, Pixel color);

constexpr float rounded = 9.5f;
constexpr float inner_width = 49.f;
constexpr float inner_height = 29.f;

void roundRect(Canvas& canvas, Point position) {
  position.x += .5f;
  position.y += .5f;
  static float t = -.0625f;
  t += .0625f;
  auto horizontal = Dir {cos(t), sin(t)};
  auto vertical = Dir {-sin(t), cos(t)};
  auto straight_width = inner_width - 2.f * rounded;
  auto straight_height = inner_height - 2.f * rounded;
  auto corner_width = inner_width - rounded;
  auto corner_height = inner_height - rounded;
  auto color = Pixel {255, 255, 0, 0};
  blit_rectangle(canvas, position + horizontal * rounded, {straight_width, rounded}, horizontal, color);
  blit_rectangle(canvas, position + vertical * rounded, {inner_width, straight_height}, horizontal, color);
  blit_rectangle(canvas, position + horizontal * rounded + vertical * (inner_height - rounded), {straight_width, rounded}, horizontal, color);
  blit_pie(canvas, position + (horizontal + vertical) * rounded, rounded, -horizontal, -vertical, color);
  blit_pie(canvas,
    position + horizontal * (inner_width - rounded) + vertical * rounded,
    rounded, -vertical, horizontal, color);
  blit_pie(canvas,
    position + horizontal * rounded + vertical * (inner_height - rounded), rounded,
    vertical, -horizontal, color);
  blit_pie(canvas,
    position + horizontal * (inner_width - rounded) + vertical * (inner_height - rounded),
    rounded, horizontal, vertical, color);
  blit_rectangle(canvas, position + horizontal * rounded, Size {1.f, straight_width}, -vertical, LinearGradient {
    color,
    position,
    -vertical * 1.f,
  });
  blit_rectangle(canvas, position + horizontal * rounded + vertical * inner_height, Size {straight_width, 1.f}, horizontal, LinearGradient {
    color,
    position + vertical * inner_height,
    vertical * 1.f,
  });
  blit_rectangle(canvas, position + vertical * rounded, Size {straight_height, 1.f}, vertical, LinearGradient {
    color,
    position + vertical * rounded,
    -horizontal * 1.f,
  });
  blit_rectangle(canvas, position + horizontal * inner_width + vertical * rounded, Size {1.f, straight_height}, horizontal, LinearGradient {
    color,
    position + horizontal * inner_width,
    horizontal * 1.f,
  });

  AllEdges edges;
  blit_ring(edges, position + horizontal * rounded + vertical * rounded,
    rounded, rounded + 1.f, -horizontal, -vertical, color);
  blit_ring(edges, position + horizontal * corner_width + vertical * rounded,
    rounded, rounded + 1.f, -vertical, horizontal, color);
  blit_ring(edges, position + horizontal * rounded + vertical * corner_height,
    rounded, rounded + 1.f, vertical, -horizontal, color);
  blit_ring(edges, position + horizontal * corner_width + vertical * corner_height,
    rounded, rounded + 1.f, horizontal, vertical, color);
  render(canvas, edges);
  
}

}
