#include "canvas.hh"
#include "math.hh"
#include "edges.hh"

#include <cmath>
#include <cstdio>
#include <algorithm>

namespace PW {

namespace {

auto sqr(float value) -> float { return value * value; }

}

void push_ring(AllEdges& edges, Point center, float inner_radius, float outer_radius, Dir begin, Dir end, Pixel color) {

  auto inner_right = RightArc {center, sqr(inner_radius)};
  auto outer_right = RightArc {center, sqr(outer_radius)};
  auto inner_left = LeftArc {center, sqr(inner_radius)};
  auto outer_left = LeftArc {center, sqr(outer_radius)};

  auto neg0 = begin.x < 0.f;
  auto neg1 = end.x < 0.f;

  auto interior = push_fill(edges,
    RadialGradient {color, center, outer_radius, inner_radius - outer_radius});

  if (begin.y != 0.f)
    push_edge(edges,
      center.y + begin.y * inner_radius,
      center.y + begin.y * outer_radius,
      begin.y < 0.f ? interior : 0,
      Line {center + begin * inner_radius, begin.x / begin.y});
  if (end.y != 0.f)
    push_edge(edges,
      center.y + end.y * inner_radius,
      center.y + end.y * outer_radius,
      end.y > 0.f ? interior : 0,
      Line {center + end * inner_radius, end.x / end.y});

  if (neg0 && (!neg1 || begin.y < end.y)) {
    push_edge(edges, center.y - outer_radius, center.y + begin.y * outer_radius, interior, outer_left);
    push_edge(edges, center.y - inner_radius, center.y + begin.y * inner_radius, 0, inner_left);
  } else if (!neg0 && (neg1 || end.y < begin.y)) {
    push_edge(edges, center.y + inner_radius, center.y + begin.y * inner_radius, interior, inner_right);
    push_edge(edges, center.y + outer_radius, center.y + begin.y * outer_radius, 0, outer_right);
  }
  if (neg1 && (!neg0 || begin.y < end.y)) {
    push_edge(edges, center.y + outer_radius, center.y + end.y * outer_radius, interior, outer_left);
    push_edge(edges, center.y + inner_radius, center.y + end.y * inner_radius, 0, inner_left);
  } else if (!neg1 && (neg0 || end.y < begin.y)) {
    push_edge(edges, center.y - inner_radius, center.y + end.y * inner_radius, interior, inner_right);
    push_edge(edges, center.y - outer_radius, center.y + end.y * outer_radius, 0, outer_right);
  }
  if (neg0 && neg1) {
    if (begin.y < end.y) {
      push_edge(edges, center.y - inner_radius, center.y + inner_radius, interior, inner_right);
      push_edge(edges, center.y - outer_radius, center.y + outer_radius, 0, outer_right);
    } else {
      push_edge(edges, center.y + begin.y * outer_radius, center.y + end.y * outer_radius, interior, outer_left);
      push_edge(edges, center.y + begin.y * inner_radius, center.y + end.y * inner_radius, 0, inner_left);
    }
  } else if (!neg0 && !neg1) {
    if (end.y < begin.y) {
      push_edge(edges, center.y - outer_radius, center.y + outer_radius, interior, outer_left);
      push_edge(edges, center.y - inner_radius, center.y + inner_radius, 0, inner_left);
    } else {
      push_edge(edges, center.y + begin.y * inner_radius, center.y + end.y * inner_radius, interior, inner_right);
      push_edge(edges, center.y + begin.y * outer_radius, center.y + end.y * outer_radius, 0, outer_right);
    }
  }
}

}
