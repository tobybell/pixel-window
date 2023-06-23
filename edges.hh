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
  u32 fill_data[8][5];
  u8 fill_type[8];
  u32 fill_count {};
  float edge_data[8][3];
  u8 type[8];
  u32 fill[8];
  u32 count {};
  EdgeLimit lim[16];
};

void render(Canvas& canvas, AllEdges& edges);

}