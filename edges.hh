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
  float edge_data[8][3];
  EdgeLimit lim[16];
  u8 type[8];
  u32 count {};
};

void render(Canvas& canvas, AllEdges& edges, RadialGradient const& radial);

}