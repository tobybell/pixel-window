#include "canvas.hh"
#include "edges.hh"

#include <cmath>

void blit_rectangle(Canvas& canvas, Point corner, Size size, Dir dir, Pixel color);
void blit_rectangle(Canvas& canvas, Point corner, Size size, Dir dir, LinearGradient const& gradient);
void blit_pie(Canvas& canvas, Point center, float radius, Dir start, Dir end, Pixel color);

namespace PW {

void push_ring(AllEdges&, Point center, float inner_radius, float outer_radius, Dir begin, Dir end, Pixel color);

constexpr float rounded = 9.5f;
constexpr float width = 50.f;
constexpr float height = 30.f;

void check(bool condition) {
  if (!condition)
    abort();
}

void roundRect(Canvas& canvas, Point position) {
  // position.x += .5f;
  // position.y += .5f;
  static float t = -.0625f;
  t += .0625f;
  auto horizontal = Dir {cos(t), sin(t)};
  auto vertical = Dir {-sin(t), cos(t)};
  auto color = Pixel {255, 255, 0, 0};

  auto transform_y = [&](Point p) {
    return position.y + horizontal.y * p.x + vertical.y * p.y;
  };
  auto transform_x = [&](Point p) {
    return position.x + horizontal.x * p.x + vertical.x * p.y;
  };
  auto transform = [&](Point p) {
    return Point {transform_x(p), transform_y(p)};
  };
  auto lt = transform({.5f, rounded});
  auto lto = transform({-.5f, rounded});
  auto lb = transform({.5f, height - rounded});
  auto lbo = transform({-.5f, height - rounded});
  auto rt = transform({width - .5f, rounded});
  auto rto = transform({width + .5f, rounded});
  auto rb = transform({width - .5f, height - rounded});
  auto rbo = transform({width + .5f, height - rounded});
  auto tl = transform({rounded, .5f});
  auto tlo = transform({rounded, -.5f});
  auto tr = transform({width - rounded, .5f});
  auto tro = transform({width - rounded, -.5f});
  auto bl = transform({rounded, height - .5f});
  auto blo = transform({rounded, height + .5f});
  auto br = transform({width - rounded, height - .5f});
  auto bro = transform({width - rounded, height + .5f});

  AllEdges edges;

  auto ctl = transform({rounded, rounded});
  auto ctr = transform({width - rounded, rounded});
  auto cbl = transform({rounded, height - rounded});
  auto cbr = transform({width - rounded, height - rounded});

  auto interior = push_fill(edges, Solid {color});
  auto left = push_fill(edges, LinearGradient {color, lt, -horizontal * 1.f});
  auto right = push_fill(edges, LinearGradient {color, rb, horizontal * 1.f});
  auto top = push_fill(edges, LinearGradient {color, tr, -vertical * 1.f});
  auto bottom = push_fill(edges, LinearGradient {color, bl, vertical * 1.f});
  auto top_left = push_fill(edges, RadialGradient {color, ctl, rounded + .5f, -1.f});
  auto top_right = push_fill(edges, RadialGradient {color, ctr, rounded + .5f, -1.f});
  auto bottom_left = push_fill(edges, RadialGradient {color, cbl, rounded + .5f, -1.f});
  auto bottom_right = push_fill(edges, RadialGradient {color, cbr, rounded + .5f, -1.f});

  auto push_line_from_to = [&](Point from, Point to, u32 fill_right, u32 fill_left) {
    auto slope = (to.x - from.x) / (to.y - from.y);
    if (to.y < from.y)
      return push_edge(edges, to.y, from.y, fill_right, Line {from, slope});
    push_edge(edges, from.y, to.y, fill_left, Line {from, slope});
  };

  auto push_arc = [&](Point center, float radius, Dir start, Dir end, u32 fill_inner, u32 fill_outer) {
    auto r2 = radius * radius;
    auto left = LeftArc {center, r2};
    auto right = RightArc {center, r2};
    if (start.x < 0.f && end.x > 0.f) {
      push_edge(edges, center.y - radius, center.y + start.y * radius, fill_inner, left);
      push_edge(edges, center.y - radius, center.y + end.y * radius, fill_outer, right);
    } else if (start.x <= 0.f && end.x <= 0.f) {
      check(start.y > end.y);
      push_edge(edges, center.y + end.y * radius, center.y + start.y * radius, fill_inner, left);
    } else if (start.x > 0.f && end.x < 0.f) {
      push_edge(edges, center.y + end.y * radius, center.y + radius, fill_inner, left);
      push_edge(edges, center.y + start.y * radius, center.y + radius, fill_outer, right);
    } else if (start.x >= 0.f && end.x >= 0.f) {
      check(end.y > start.y);
      push_edge(edges, center.y + start.y * radius, center.y + end.y * radius, fill_outer, right);
    } else {
      abort();  // not a bug, this has unhandled cases for >180deg
    }
  };

  push_line_from_to(lb, lbo, left, bottom_left);
  push_line_from_to(lbo, lto, left, 0);
  push_line_from_to(lto, lt, left, top_left);

  push_line_from_to(rt, rto, right, top_right);
  push_line_from_to(rto, rbo, right, 0);
  push_line_from_to(rbo, rb, right, bottom_right);

  push_line_from_to(tl, tlo, top, top_left);
  push_line_from_to(tlo, tro, top, 0);
  push_line_from_to(tro, tr, top, top_right);

  push_line_from_to(br, bro, bottom, bottom_right);
  push_line_from_to(bro, blo, bottom, 0);
  push_line_from_to(blo, bl, bottom, bottom_left);

  push_line_from_to(tl, tr, interior, top);
  push_line_from_to(rt, rb, interior, right);
  push_line_from_to(br, bl, interior, bottom);
  push_line_from_to(lb, lt, interior, left);
  push_arc(ctl, rounded - .5f, -horizontal, -vertical, interior, top_left);
  push_arc(ctl, rounded + .5f, -horizontal, -vertical, top_left, 0);
  push_arc(ctr, rounded - .5f, -vertical, horizontal, interior, top_right);
  push_arc(ctr, rounded + .5f, -vertical, horizontal, top_right, 0);
  push_arc(cbr, rounded - .5f, horizontal, vertical, interior, bottom_right);
  push_arc(cbr, rounded + .5f, horizontal, vertical, bottom_right, 0);
  push_arc(cbl, rounded - .5f, vertical, -horizontal, interior, bottom_left);
  push_arc(cbl, rounded + .5f, vertical, -horizontal, bottom_left, 0);

  render(canvas, edges);
  
}

}
