#pragma once

#include <stdint.h>

struct RGBColor {
  float r;
  float g;
  float b;

  RGBColor()
    : r(0)
    , g(0)
    , b(0) {}

  RGBColor(float r_, float g_, float b_)
    : r(r_)
    , g(g_)
    , b(b_) {}

  RGBColor(int r_, int g_, int b_)
    : r(r_ / 255.f)
    , g(g_ / 255.f)
    , b(b_ / 255.f) {}

  RGBColor(uint32_t rgb)
    : r(((rgb >> 16) & 0xff) / 255.f)
    , g(((rgb >> 8) & 0xff) / 255.f)
    , b((rgb & 0xff) / 255.f) {}
};

struct Config {
  /** Create a config using the defaults. */
  Config();

  // [geometry]

  int height;
  int pad;
  float corner_radius;
  int label_width;
  enum class Alignment {
    Left,
    Center,
    Right,
  } alignment;
  bool bottom;

  // [colors]

  RGBColor background_color = RGBColor(255, 255, 255);
  RGBColor highlight_color;
  RGBColor inactive_text_color = highlight_color;
  RGBColor active_text_color = background_color;

  // [keybinds]

  bool window_switch = false;
  bool window_switch_wrap = false;
};

int config_entry_handler(void* config, const char* section, const char* name,
                         const char* value);
