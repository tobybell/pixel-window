#pragma once

#include "canvas.hh"

namespace PW {

struct Line {
  enum { edge_type = 0 };
  Point start;
  float slope;
};

struct LeftArc {
  enum { edge_type = 1 };
  Point center;
  float radius2;
};

struct RightArc {
  enum { edge_type = 2 };
  Point center;
  float radius2;
};

struct EdgeLimit {
  u32 edge;
  u32 i;
};

struct FillData {
  float words[5];
};

struct EdgeData {
  float words[3];
};

struct AllEdges {
  enum { max_fills = 16, max_edges = 128 };
  FillData fill_data[max_fills];
  u8 fill_type[max_fills];
  u32 fill_count {};
  EdgeData edge_data[max_edges];
  u8 type[max_edges];
  u32 fill[max_edges];
  u32 count {};
  EdgeLimit lim[2 * max_edges];
};

u32 push_fill(AllEdges& edges, u8 fill_type, FillData const& fill);

template <class T>
u32 push_fill(AllEdges& edges, T const& fill) {
  static_assert(sizeof(T) <= sizeof(FillData));
  static_assert(alignof(T) <= alignof(FillData));
  return push_fill(edges, T::fill_type, reinterpret_cast<FillData const&>(fill));
}

void push_edge(AllEdges& edges, float y0, float y1, u32 fill_right, EdgeData const& edge, u8 edge_type);

template <class T>
void push_edge(AllEdges& edges, float y0, float y1, u32 fill_right, T const& edge) {
  static_assert(sizeof(T) <= sizeof(EdgeData));
  static_assert(alignof(T) <= alignof(EdgeData));
  return push_edge(edges, y0, y1, fill_right, reinterpret_cast<EdgeData const&>(edge), T::edge_type);
}

void render(Canvas& canvas, AllEdges& edges);

}