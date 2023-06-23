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
float do_eval(float const (&data)[3], float y) {
  return eval(reinterpret_cast<T const&>(data), y);
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

  void blit(Canvas& canvas, u32 i0, u32 i1, RadialGradient const& radial) {
    for (auto i = i0; i < i1; ++i) {
      auto y = i + .5f;

      u32 js[8];
      for (auto k = 0; k < edge_count; ++k) {
        auto e = edges[k];
        js[k] = to_pixel(eval_edge(edge_info.type[e], edge_info.edge_data[e], y));
      }
      std::sort(&js[0], &js[edge_count]);

      for (auto k = 0; k < edge_count; k += 2)
        setrow(canvas, i, js[k], js[k + 1], radial);
    }
  }
};

}

void render(Canvas& canvas, AllEdges& edges, RadialGradient const& radial) {
  auto begin = &edges.lim[0];
  auto end = &edges.lim[2 * edges.count];
  std::sort(begin, end);

  Edges renderer {edges};
  renderer.update(begin[0].edge, begin[1].edge);
  for (auto it = begin + 2; it != end; it += 2) {
    renderer.blit(canvas, it[-1].i, it[0].i, radial);
    check(it[0].i == it[1].i);
    renderer.update(it[0].edge, it[1].edge);
  }
}

}
