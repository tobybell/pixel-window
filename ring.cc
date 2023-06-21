#include "canvas.hh"

#include <cmath>
#include <cstdio>

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


struct AllEdges {
  float edge_data[4][3];
  u8 type[4];
  template <class T>
  void set(u32 index, T const& edge) {
    static_assert(sizeof(T) == 12);
    static_assert(alignof(T) == 4);
    reinterpret_cast<T&>(edge_data[index]) = edge;
    type[index] = EdgeType<T>::value;
  }
};

struct Edges {
  AllEdges const& edge_info;
  u8 edges[4];
  u32 edge_count {};

  void blit(Canvas& canvas, u32 i0, u32 i1) {
    for (auto i = i0; i < i1; ++i) {
      auto y = i + .5f;
      for (auto k = 0; k < edge_count; k += 2) {
        auto e0 = edges[k];
        auto e1 = edges[k + 1];
        auto j0 = to_pixel(eval_edge(edge_info.type[e0], edge_info.edge_data[e0], y));
        auto j1 = to_pixel(eval_edge(edge_info.type[e1], edge_info.edge_data[e1], y));
        setrow(canvas, i, j0, j1, debug_color);
      }
    }
  }
};

struct Poin {
  int i;
  u8 type;
  u8 first_edge;
  u8 second_edge;
};

void apply_poin(Edges& edges, Poin const& poin) {
  switch (poin.type) {
    case 0:
      edges.edges[edges.edge_count++] = poin.first_edge;
      edges.edges[edges.edge_count++] = poin.second_edge;
      return;
    case 1:
      edges.edges[poin.first_edge] = poin.second_edge;
      return;
    case 2:
      edges.edge_count -= 2;  // TODO: shift everything down
      return;
  }
}

}

void blit_ring(Canvas& canvas, Point center, float inner_radius, float outer_radius, Dir begin, Dir end) {
  // auto color = Pixel {255, 0, 100, 150};

  // what are all the combinations we could have?

  auto inner_arc = RightArc {center, sqr(inner_radius)};
  auto outer_arc = RightArc {center, sqr(outer_radius)};
  auto begin_cap = Line {center + begin * inner_radius, begin.x / begin.y};
  auto end_cap = Line {center + end * inner_radius, end.x / end.y};
  // debug_edge(canvas, inner_arc);
  // debug_edge(canvas, outer_arc);
  // debug_edge(canvas, begin_cap);
  // debug_edge(canvas, end_cap);

  AllEdges all_edges;
  all_edges.set(0, inner_arc);
  all_edges.set(1, outer_arc);
  all_edges.set(2, begin_cap);
  all_edges.set(3, end_cap);

  // for the arc, we may want to split into 2. But for now let's assume we know
  // we don't.

  auto pt_begin_inner = center + begin * inner_radius;
  auto pt_begin_outer = center + begin * outer_radius;
  auto pt_end_inner = center + end * inner_radius;
  auto pt_end_outer = center + end * outer_radius;

  Edges edges {all_edges};

  Poin P[4] {
    {to_pixel(pt_begin_inner.y), 0, 0, 2},
    {to_pixel(pt_begin_outer.y), 1, 1, 1},
    {to_pixel(pt_end_inner.y), 1, 0, 3},
    {to_pixel(pt_end_outer.y), 2, 0, 1}};
  // u8 type[4] {0, 1, 1, 2};  // 0 = create, 1 = replace, 2 = end

  auto i_curr = P[0].i;
  apply_poin(edges, P[0]);
  
  for (u32 i = 1; i < 4; ++i) {
    auto i_next = P[i].i;
    edges.blit(canvas, i_curr, i_next);
    apply_poin(edges, P[i]);
    i_curr = i_next;
  }

  // Now, make list of y coordinates, and which edges they join. If both edges
  // go down, then it's a "start point". If both edges go up, then it's an "end
  // point". If one goes up and one goes down, it's a "change point".
  // float y_coords[4] = {
  // };

  // enumerate all the fragments, in such a way that they never duplicate y
  // values.
  // Go from top to bottom. Blit.
}

}
