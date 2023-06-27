#pragma once

#include "canvas.hh"

#include <cmath>

namespace PW {

inline auto len(Point const& p) -> float { return sqrt(p.x * p.x + p.y * p.y); }

inline auto make_dir(float theta) -> Dir { return {cos(theta), sin(theta)}; }

inline auto dir(Point const& p) -> Dir {
  auto d = 1.f / sqrt(p.x * p.x + p.y * p.y);
  return {p.x * d, p.y * d};
}

inline auto dir_from_to(Point a, Point b) -> Dir { return dir(b - a); }

}
