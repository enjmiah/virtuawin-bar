#pragma once

namespace vwtiling {

struct RGBColor {
  double r;
  double g;
  double b;

  RGBColor(double r_, double g_, double b_)
    : r(r_)
    , g(g_)
    , b(b_) {}

  RGBColor(int r_, int g_, int b_)
    : r(r_ / 255.0)
    , g(g_ / 255.0)
    , b(b_ / 255.0) {}
};

struct Config {
  int height = 30;
  int pad = 26;
  double corner_radius = 0; // TODO: Transparency is janky

  int label_width = height;

  RGBColor background_color = RGBColor(208, 222, 207);
  RGBColor highlight_color = RGBColor(117, 190, 195);
  RGBColor inactive_text_color = highlight_color;
  RGBColor active_text_color = background_color;

  int outer_gap = 16;
};

} // namespace vwtiling
