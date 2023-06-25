#include "edges.hh"
#include "math.hh"

#include <cmath>
#include <algorithm>

namespace PW {

static bool operator<(EdgeLimit const& lhs, EdgeLimit const& rhs) {
  return lhs.i < rhs.i;
}

namespace {

void check(bool condition) {
  if (!condition)
    abort();
}

auto sqr(float value) -> float { return value * value; }

float eval(LeftArc const& e, float y) {
  return e.center.x - sqrt(e.radius2 - sqr(y - e.center.y));
}

float eval(RightArc const& e, float y) {
  return e.center.x + sqrt(e.radius2 - sqr(y - e.center.y));
}

float eval(Line const& e, float y) {
  return e.start.x + e.slope * (y - e.start.y);
}

template <class T>
float do_eval(EdgeData const& data, float y) {
  return eval(reinterpret_cast<T const&>(data), y);
}

constexpr float (*evals[3])(EdgeData const& data, float y) {
  do_eval<Line>, do_eval<LeftArc>, do_eval<RightArc>
};

float eval_edge(u8 type, EdgeData const& data, float y) {
  return evals[type](data, y);
}

int to_pixel(float coord) {
  return static_cast<int>(coord + .5f);
}

// dedup
Pixel lerp(Pixel const& a, Pixel const& b, float t) {
  Pixel c;
  for (auto i = 0u; i < 4u; ++i)
    c[i] = a[i] + (b[i] - a[i]) * t;
  return c;
}

void setrow(Canvas& canvas, u32 i, u32 j0, u32 j1, RadialGradient const& radial) {
  for (u32 j = j0; j < j1; ++j) {
    auto p = Point {j + .5f, i + .5f};
    auto t = max(0.f, min(1.f, (len(p - radial.position) - radial.start_radius) / radial.thickness));
    auto& curr = canvas.data[i * canvas.stride + j];
    curr = lerp(curr, radial.color, t);
  }
}

void setrow(Canvas& canvas, u32 i, u32 j1, u32 j2, Solid const& solid) {
  for (u32 j = j1; j < j2; ++j)
    canvas.data[i * canvas.stride + j] = solid.color;
}

void setrow(Canvas& canvas, u32 i, u32 j0, u32 j1, LinearGradient const& gradient) {
  for (u32 j = j0; j < j1; ++j) {
    auto p = Point {j + .5f, i + .5f};
    auto t = max(0.f, min(1.f, dot(p - gradient.position, gradient.direction)));
    auto& curr = canvas.data[i * canvas.stride + j];
    curr = lerp(gradient.color, curr, t);
  }
}

struct Edges {
  AllEdges const& edge_info;
  u8 edges[AllEdges::max_edges];
  u32 active[AllEdges::max_edges] {};
  u32 edge_count {};

  void update(u32 edge) {
    auto i_edge = active[edge];
    if (i_edge) {
      --i_edge;
      active[edge] = 0;
      edge_count -= 1;
      for (auto i = i_edge; i < edge_count; ++i) {
        auto e = edges[i + 1];
        active[e] -= 1;
        edges[i] = e;
      }
    } else {
      edges[edge_count] = edge;
      active[edge] = edge_count + 1;
      ++edge_count;
    }
  }

  struct Checkpoint {
    float x;
    u32 fill;
    bool operator<(Checkpoint const& rhs) const { return x < rhs.x; }
  };

  void blit(Canvas& canvas, u32 i0, u32 i1) {
    for (auto i = i0; i < i1; ++i) {
      auto y = i + .5f;

      Checkpoint js[8];
      for (auto k = 0; k < edge_count; ++k) {
        auto e = edges[k];
        js[k] = {
          eval_edge(edge_info.type[e], edge_info.edge_data[e], y),
          edge_info.fill[e]};
      }
      std::sort(&js[0], &js[edge_count]);

      auto cur_j = to_pixel(js[0].x);
      auto cur_fill = js[0].fill;
      for (auto k = 1; k < edge_count; ++k) {
        auto const& next = js[k];
        auto next_j = to_pixel(next.x);
        if (next_j > cur_j && cur_fill) {
          switch (edge_info.fill_type[cur_fill - 1]) {
            case 0:
              setrow(canvas, i, cur_j, next_j, reinterpret_cast<RadialGradient const&>(edge_info.fill_data[cur_fill - 1]));
              break;
            case 1:
              setrow(canvas, i, cur_j, next_j, reinterpret_cast<Solid const&>(edge_info.fill_data[cur_fill - 1]));
              break;
            case 2:
              setrow(canvas, i, cur_j, next_j, reinterpret_cast<LinearGradient const&>(edge_info.fill_data[cur_fill - 1]));
              break;
          }
        }
        cur_j = next_j;
        cur_fill = next.fill;
      }
    }
  }
};

}  // namespace

u32 push_fill(AllEdges& edges, u8 fill_type, FillData const& fill) {
  check(edges.fill_count < AllEdges::max_fills);
  auto index = edges.fill_count++;
  edges.fill_data[index] = fill;
  edges.fill_type[index] = fill_type;
  return index + 1;
}

void push_edge(AllEdges& edges, float y0, float y1, u32 fill_right, EdgeData const& edge, u8 edge_type) {
  auto i0 = static_cast<u32>(y0 + .5f);
  auto i1 = static_cast<u32>(y1 + .5f);
  if (i0 == i1)
    return;
  check(edges.count < AllEdges::max_edges);
  auto index = edges.count++;
  edges.edge_data[index] = edge;
  edges.type[index] = edge_type;
  edges.fill[index] = fill_right;
  edges.lim[2 * index] = {index, i0};
  edges.lim[2 * index + 1] = {index, i1};
}

void render(Canvas& canvas, AllEdges& edges) {
  if (!edges.count)
    return;

  auto begin = &edges.lim[0];
  auto end = &edges.lim[2 * edges.count];
  std::sort(begin, end);

  Edges renderer {edges};
  renderer.update(begin[0].edge);
  for (auto it = begin + 1; it != end; ++it) {
    renderer.blit(canvas, it[-1].i, it[0].i);
    renderer.update(it[0].edge);
  }
}

}
