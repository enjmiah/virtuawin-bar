#include "config.h"

#include <dwmapi.h>

namespace vwbar {

static bool get_accent_color(RGBColor* color) {
  DWORD active_caption; // AARRGGBB
  BOOL opaque_blend;
  if (DwmGetColorizationColor(&active_caption, &opaque_blend) == S_OK) {
    *color = RGBColor(active_caption & 0xFFFFFF);
    return true;
  }
  return false;
}

Config::Config()
  : height(26)
  , pad(10)
  , corner_radius(0) // TODO: Transparency is janky.
  , label_width(height)
  , outer_gap(16)
  , alignment(Alignment::Center) {

  get_accent_color(&highlight_color);
  inactive_text_color = highlight_color;
}

int config_entry_handler(void* user, const char* section, const char* name,
                         const char* value) {
  Config* config = (Config*)user;

#define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
  if (MATCH("geometry", "height")) {
    config->height = atoi(value);
  } else if (MATCH("geometry", "pad")) {
    config->pad = atoi(value);
  } else if (MATCH("geometry", "label_width")) {
    config->label_width = atoi(value);
  } else if (MATCH("geometry", "outer_gap")) {
    config->outer_gap = atoi(value);
  } else if (MATCH("geometry", "alignment")) {
    if (strcmp(value, "left") == 0) {
      config->alignment = Config::Alignment::Left;
    } else if (strcmp(value, "center") == 0) {
      config->alignment = Config::Alignment::Center;
    } else if (strcmp(value, "right") == 0) {
      // TODO:
    } else {
      return 0; // Error: unrecognized.
    }
  } else if (MATCH("colors", "background")) {
    if (strlen(value) == 7 && value[0] == '#') {
      config->background_color = strtol(value + 1, nullptr, 16);
    } else {
      return 0; // Error: invalid colour.
    }
  } else if (MATCH("colors", "highlight")) {
    if (strlen(value) == 7 && value[0] == '#') {
      config->highlight_color = strtol(value + 1, nullptr, 16);
    } else if (strcmp(value, "auto") == 0) {
      get_accent_color(&config->highlight_color);
    } else {
      return 0; // Error: invalid colour.
    }
  } else if (MATCH("colors", "active")) {
    if (strlen(value) == 7 && value[0] == '#') {
      config->active_text_color = strtol(value + 1, nullptr, 16);
    } else {
      return 0; // Error: invalid colour.
    }
  } else if (MATCH("colors", "inactive")) {
    if (strlen(value) == 7 && value[0] == '#') {
      config->inactive_text_color = strtol(value + 1, nullptr, 16);
    } else if (strcmp(value, "auto") == 0) {
      get_accent_color(&config->inactive_text_color);
    } else {
      return 0; // Error: invalid colour.
    }
  } else {
    return 0; /* unknown section/name, error */
  }
  return 1;
}

} // namespace vwbar
