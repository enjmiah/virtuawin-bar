#pragma once

namespace vwtiling {

struct RGBColor {
  float r;
  float g;
  float b;

  RGBColor(float r_, float g_, float b_)
    : r(r_)
    , g(g_)
    , b(b_) {}

  RGBColor(int r_, int g_, int b_)
    : r(r_ / 255.f)
    , g(g_ / 255.f)
    , b(b_ / 255.f) {}
};

struct Config {
  Config();

  int height;
  int pad;
  float corner_radius;
  int label_width;
  int outer_gap;
  enum class Alignment {
    Left,
    Center,
  } alignment;

  RGBColor background_color = RGBColor(208, 222, 207);
  RGBColor highlight_color = RGBColor(0, 0, 0);
  RGBColor inactive_text_color = highlight_color;
  RGBColor active_text_color = background_color;
};

} // namespace vwtiling
