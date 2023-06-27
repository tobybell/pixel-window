#include "canvas.hh"
#include "math.hh"

void blit_rectangle(Canvas& canvas, Point corner, Size size, Dir dir, Pixel color);
void blit_rectangle(Canvas& canvas, Point corner, Size size, Dir dir, LinearGradient const& gradient);
void blit_pie(Canvas& canvas, Point center, float radius, Dir start, Dir end, RadialGradient const& radial);

namespace PW {

namespace {

Dir operator*(Dir const& lhs, Dir const& rhs) {
  return {lhs.x * rhs.x - lhs.y * rhs.y, lhs.x * rhs.y + lhs.y * rhs.x};
}

Point offset_corner(Point const& point, Dir const& dir0, Dir const& dir1, float amount) {
  auto n = amount / abs(cross(dir0, dir1));
  return point + (dir0 + dir1) * n;
}

auto m90(Dir d) -> Dir { return {d.y, -d.x}; }
auto p90(Dir d) -> Dir { return {-d.y, d.x}; }

}

void star(Canvas& canvas, Point center, float outer_radius, float inner_radius, Dir top) {
  constexpr auto blur = 1.f;
  constexpr auto half = .5f * blur;
  constexpr auto color = Pixel {255, 220, 215, 0};

  Point o[10];
  o[0] = center + top * make_dir(-90.f / 180.f * M_PI) * outer_radius;
  o[1] = center + top * make_dir(-54.f / 180.f * M_PI) * inner_radius;
  o[2] = center + top * make_dir(-18.f / 180.f * M_PI) * outer_radius;
  o[3] = center + top * make_dir(18.f / 180.f * M_PI) * inner_radius;
  o[4] = center + top * make_dir(54.f / 180.f * M_PI) * outer_radius;
  o[5] = center + top * make_dir(90.f / 180.f * M_PI) * inner_radius;
  o[6] = center + top * make_dir(126.f / 180.f * M_PI) * outer_radius;
  o[7] = center + top * make_dir(162.f / 180.f * M_PI) * inner_radius;
  o[8] = center + top * make_dir(198.f / 180.f * M_PI) * outer_radius;
  o[9] = center + top * make_dir(234.f / 180.f * M_PI) * inner_radius;

  Dir dir[10];
  for (u32 i = 0; i + 1 < 10; ++i)
    dir[i] = dir_from_to(o[i], o[i + 1]);
  dir[9] = dir_from_to(o[9], o[0]);

  Point ii[5];
  for (u32 i = 0; i < 5; ++i)
    ii[i] = offset_corner(o[2 * i + 1], -dir[2 * i], dir[2 * i + 1], -half);

  o[0] = offset_corner(o[0], -dir[9], dir[0], half);
  for (u32 i = 1; i < 10; ++i)
    o[i] = offset_corner(o[i], -dir[i - 1], dir[i], half);

  auto rect_length = dot(o[1] - o[0], dir[0]);
  for (u32 i = 0; i < 10; i += 2)
    blit_rectangle(canvas, o[i], {blur, rect_length}, m90(dir[i]), LinearGradient {color, o[i], m90(dir[i]) * (1.f / blur)});
  for (u32 i = 1; i < 10; i += 2)
    blit_rectangle(canvas, o[i], {rect_length, blur}, dir[i], LinearGradient {color, o[i] + p90(dir[i]) * blur, m90(dir[i]) * (1.f / blur)});

  blit_pie(canvas, o[0], blur, m90(dir[9]), m90(dir[0]), RadialGradient {color, o[0], blur, -blur});
  for (u32 i = 2; i < 10; i += 2)
    blit_pie(canvas, o[i], blur, m90(dir[i - 1]), m90(dir[i]), RadialGradient {color, o[i], blur, -blur});
  for (u32 i = 1; i < 10; i += 2)
    blit_pie(canvas, o[i], blur, p90(dir[i]), p90(dir[i - 1]), RadialGradient {color, o[i], 0, blur});

  for (u32 i = 0; i < 5; ++i)
    blit_triangle(canvas, center, ii[i], o[2 * i], color);
  for (u32 i = 0; i + 1 < 5; ++i)
    blit_triangle(canvas, center, ii[i], o[2 * (i + 1)], color);
  blit_triangle(canvas, center, ii[4], o[0], color);
}

}
