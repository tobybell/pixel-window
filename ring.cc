#include "canvas.hh"

#include <cmath>
#include <cstdio>
#include <algorithm>

namespace PW {

namespace {

// TODO: dedup
void setrow(Canvas& canvas, u32 i, u32 j1, u32 j2, Pixel color) {
  for (u32 j = j1; j < j2; ++j)
    canvas.data[i * canvas.stride + j] = color;
}

auto sqr(float value) -> float { return value * value; }

struct LeftArc {
  Point center;
  float radius2;
  float eval(float y) const { return center.x - sqrt(radius2 - sqr(y - center.y)); }
};

struct RightArc {
  Point center;
  float radius2;
  float eval(float y) const { return center.x + sqrt(radius2 - sqr(y - center.y)); }
};

struct Line {
  Point start;
  float slope;
  float eval(float y) const { return start.x + slope * (y - start.y); }
};

  // loop over points, know if they start a fragment or stop a fragment. If they
  // start a fragment, add to the current fragments list. Go to next point,
  // blit all rows using current fragments

  // Each point must end a fragment and start a new one, or start 2 fragments, or end 2 fragments.

// void debug_edge(Canvas& canvas, RightArc const& arc) {
//   for (u32 i = 0; i < 32; ++i) {
//     auto theta = i / 16.f * static_cast<float>(M_PI);
//     auto pt = arc.center + Dir {cos(theta), sin(theta)} * arc.radius;
//     point(canvas, pt);
//   }
// }

// void debug_edge(Canvas& canvas, Line const& line) {
//   for (u32 i = 0; i < 32; ++i) {
//     auto pt = line.start + Point {2.f * line.slope * i, 2.f * i};
//     point(canvas, pt);
//   }
// }

template <class T>
struct EdgeType;

template <> struct EdgeType<Line> { enum { value = 0 }; };
template <> struct EdgeType<LeftArc> { enum { value = 1 }; };
template <> struct EdgeType<RightArc> { enum { value = 2 }; };

template <class T>
float do_eval(float const (&data)[3], float y) {
  return reinterpret_cast<T const&>(data).eval(y);
}

constexpr float (*evals[3])(float const (&data)[3], float y) {
  do_eval<Line>, do_eval<LeftArc>, do_eval<RightArc>
};

float eval_edge(u8 type, float const (&data)[3], float y) {
  return evals[type](data, y);
}

int to_pixel(float coord) {
  return static_cast<int>(coord + .5f);
}

constexpr Pixel debug_color {255, 255, 0, 255};

void check(bool condition) {
  if (!condition)
    abort();
}

struct EdgeLimit {
  u32 edge;
  u32 i;
  bool operator<(EdgeLimit const& rhs) const { return i < rhs.i; }
};

struct AllEdges {
  float edge_data[8][3];
  EdgeLimit lim[16];
  u8 type[8];
  u32 count {};

  template <class T>
  void push(T const& edge, float y0, float y1) {
    auto index = count++;
    static_assert(sizeof(T) == 12);
    static_assert(alignof(T) == 4);
    reinterpret_cast<T&>(edge_data[index]) = edge;
    type[index] = EdgeType<T>::value;
    lim[2 * index] = {index, static_cast<u32>(y0 + .5f)};
    lim[2 * index + 1] = {index, static_cast<u32>(y1 + .5f)};
  }
};

struct Edges {
  AllEdges const& edge_info;
  u8 edges[8];
  u32 active[8] {};
  u32 edge_count {};

  void update(u32 edge0, u32 edge1) {
    auto active0 = !!active[edge0];
    auto active1 = !!active[edge1];
    if (active0) {
      if (active1) {
        auto i0 = active[edge0] - 1;
        auto i1 = active[edge1] - 1;
        active[edge0] = 0;
        active[edge1] = 0;
        if (i1 < i0)
          std::swap(i0, i1);
        --i1;
        edge_count -= 2;
        for (auto i = i0; i < i1; ++i) {
          auto e = edges[i + 1];
          active[e] -= 1;
          edges[i] = e;
        }
        for (auto i = i1; i < edge_count; ++i) {
          auto e = edges[i + 2];
          active[e] -= 2;
          edges[i] = e;
        }
      } else {
        auto i = active[edge0] - 1;
        edges[i] = edge1;
        active[edge0] = 0;
        active[edge1] = i + 1;
      }
    } else {
      if (active1) {
        auto i = active[edge1] - 1;
        edges[i] = edge0;
        active[edge0] = i + 1;
        active[edge1] = 0;
      } else {
        edges[edge_count] = edge0;
        edges[edge_count + 1] = edge1;
        active[edge0] = edge_count + 1;
        active[edge1] = edge_count + 2;
        edge_count += 2;
      }
    }
  }

  void blit(Canvas& canvas, u32 i0, u32 i1) {
    for (auto i = i0; i < i1; ++i) {
      auto y = i + .5f;

      u32 js[8];
      for (auto k = 0; k < edge_count; ++k) {
        auto e = edges[k];
        js[k] = to_pixel(eval_edge(edge_info.type[e], edge_info.edge_data[e], y));
      }
      std::sort(&js[0], &js[edge_count]);

      for (auto k = 0; k < edge_count; k += 2) {
        setrow(canvas, i, js[k], js[k + 1], debug_color);
      }
    }
  }
};

}

void blit_ring(Canvas& canvas, Point center, float inner_radius, float outer_radius, Dir begin, Dir end) {
  // auto color = Pixel {255, 0, 100, 150};

  // what are all the combinations we could have?

  // if (begin.x < 0.f && end.x < 0.f) {

  // }

  AllEdges all_edges;

  // we could have 2 left pieces, 2 right pieces
  auto inner_right = RightArc {center, sqr(inner_radius)};
  auto outer_right = RightArc {center, sqr(outer_radius)};
  auto inner_left = LeftArc {center, sqr(inner_radius)};
  auto outer_left = LeftArc {center, sqr(outer_radius)};

  // auto neg0 = begin.x < 0.f;
  // auto neg1 = end.x < 0.f;
  // if (neg0 && neg1) {
  //   all_edges.push(LeftArc {center, sqr(outer_radius)});
  //   all_edges.push(LeftArc {center, sqr(inner_radius)});
  // } else {
  auto y0 = center.y + begin.y * inner_radius;
  auto y1 = center.y + begin.y * outer_radius;
  auto y2 = center.y + end.y * inner_radius;
  auto y3 = center.y + end.y * outer_radius;

  auto neg0 = begin.x < 0.f;
  auto neg1 = end.x < 0.f;
  all_edges.push(Line {center + begin * inner_radius, begin.x / begin.y}, y0, y1);
  all_edges.push(Line {center + end * inner_radius, end.x / end.y}, y2, y3);
  if (!neg0 && neg1) {
    all_edges.push(outer_left, center.y + outer_radius, y3);
    all_edges.push(inner_left, center.y + inner_radius, y2);
    all_edges.push(inner_right, center.y + inner_radius, y0);
    all_edges.push(outer_right, center.y + outer_radius, y1);
  } else if (neg0 && !neg1) {
    all_edges.push(outer_left, center.y - outer_radius, y1);
    all_edges.push(inner_left, center.y - inner_radius, y0);
    all_edges.push(inner_right, center.y - inner_radius, y2);
    all_edges.push(outer_right, center.y - outer_radius, y3);
  } else if (neg0 && neg1 && end.y < begin.y) {
    all_edges.push(outer_left, y1, y3);
    all_edges.push(inner_left, y0, y2);
  } else {
    all_edges.push(inner_right, y0, y2);
    all_edges.push(outer_right, y1, y3);
  }
  {
    auto begin = &all_edges.lim[0];
    auto end = &all_edges.lim[2 * all_edges.count];
    std::sort(begin, end);

    Edges edges {all_edges};
    edges.update(begin[0].edge, begin[1].edge);
    for (auto it = begin + 2; it != end; it += 2) {
      edges.blit(canvas, it[-1].i, it[0].i);
      check(it[0].i == it[1].i);
      edges.update(it[0].edge, it[1].edge);
    }
  }
}

}
