#include "canvas.hh"
#include "math.hh"
#include "edges.hh"

#include <cmath>
#include <cstdio>
#include <algorithm>

namespace PW {

namespace {

auto sqr(float value) -> float { return value * value; }

template <class T>
u32 push_fill(AllEdges& edges, T const& fill) {
  auto index = edges.fill_count++;
  static_assert(sizeof(T) <= sizeof(edges.fill_data[0]));
  static_assert(alignof(T) <= alignof(u32));
  reinterpret_cast<T&>(edges.fill_data[index]) = fill;
  edges.fill_type[index] = T::fill_type;
  return index + 1;
}

template <class T>
void push(AllEdges& edges, T const& edge, float y0, float y1, u32 fill_right) {
  auto i0 = static_cast<u32>(y0 + .5f);
  auto i1 = static_cast<u32>(y1 + .5f);
  if (i0 == i1)
    return;
  auto index = edges.count++;
  static_assert(sizeof(T) == 12);
  static_assert(alignof(T) == 4);
  reinterpret_cast<T&>(edges.edge_data[index]) = edge;
  edges.type[index] = T::edge_type;
  edges.fill[index] = fill_right;
  edges.lim[2 * index] = {index, i0};
  edges.lim[2 * index + 1] = {index, i1};
}

}

void blit_ring(Canvas& canvas, Point center, float inner_radius, float outer_radius, Dir begin, Dir end, Pixel color) {
  AllEdges all_edges;

  auto inner_right = RightArc {center, sqr(inner_radius)};
  auto outer_right = RightArc {center, sqr(outer_radius)};
  auto inner_left = LeftArc {center, sqr(inner_radius)};
  auto outer_left = LeftArc {center, sqr(outer_radius)};

  auto neg0 = begin.x < 0.f;
  auto neg1 = end.x < 0.f;

  auto interior = push_fill(all_edges,
    RadialGradient {color, center, outer_radius, inner_radius - outer_radius});

  if (begin.y != 0.f)
    push(all_edges,
      Line {center + begin * inner_radius, begin.x / begin.y},
      center.y + begin.y * inner_radius,
      center.y + begin.y * outer_radius,
      begin.y < 0.f ? interior : 0);
  if (end.y != 0.f)
    push(all_edges,
      Line {center + end * inner_radius, end.x / end.y},
      center.y + end.y * inner_radius,
      center.y + end.y * outer_radius,
      end.y > 0.f ? interior : 0);

  if (neg0 && (!neg1 || begin.y < end.y)) {
    push(all_edges, outer_left, center.y - outer_radius, center.y + begin.y * outer_radius, interior);
    push(all_edges, inner_left, center.y - inner_radius, center.y + begin.y * inner_radius, 0);
  } else if (!neg0 && (neg1 || end.y < begin.y)) {
    push(all_edges, inner_right, center.y + inner_radius, center.y + begin.y * inner_radius, interior);
    push(all_edges, outer_right, center.y + outer_radius, center.y + begin.y * outer_radius, 0);
  }
  if (neg1 && (!neg0 || begin.y < end.y)) {
    push(all_edges, outer_left, center.y + outer_radius, center.y + end.y * outer_radius, interior);
    push(all_edges, inner_left, center.y + inner_radius, center.y + end.y * inner_radius, 0);
  } else if (!neg1 && (neg0 || end.y < begin.y)) {
    push(all_edges, inner_right, center.y - inner_radius, center.y + end.y * inner_radius, interior);
    push(all_edges, outer_right, center.y - outer_radius, center.y + end.y * outer_radius, 0);
  }
  if (neg0 && neg1) {
    if (begin.y < end.y) {
      push(all_edges, inner_right, center.y - inner_radius, center.y + inner_radius, interior);
      push(all_edges, outer_right, center.y - outer_radius, center.y + outer_radius, 0);
    } else {
      push(all_edges, outer_left, center.y + begin.y * outer_radius, center.y + end.y * outer_radius, interior);
      push(all_edges, inner_left, center.y + begin.y * inner_radius, center.y + end.y * inner_radius, 0);
    }
  } else if (!neg0 && !neg1) {
    if (end.y < begin.y) {
      push(all_edges, outer_left, center.y - outer_radius, center.y + outer_radius, interior);
      push(all_edges, inner_left, center.y - inner_radius, center.y + inner_radius, 0);
    } else {
      push(all_edges, inner_right, center.y + begin.y * inner_radius, center.y + end.y * inner_radius, interior);
      push(all_edges, outer_right, center.y + begin.y * outer_radius, center.y + end.y * outer_radius, 0);
    }
  }

  if (!all_edges.count)
    return;

  render(canvas, all_edges);
}

}
