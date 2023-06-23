#pragma once

#include "canvas.hh"

#include <cmath>

namespace PW {

inline float len(Point const& p) { return sqrt(p.x * p.x + p.y * p.y); }

}
