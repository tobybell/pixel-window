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

struct AllEdges {
  enum { max_fills = 8, max_edges = 40 };
  u32 fill_data[max_fills][5];
  u8 fill_type[max_fills];
  u32 fill_count {};
  float edge_data[max_edges][3];
  u8 type[max_edges];
  u32 fill[max_edges];
  u32 count {};
  EdgeLimit lim[2 * max_edges];
};

void render(Canvas& canvas, AllEdges& edges);

}