#include "config.h"

#include <dwmapi.h>

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
  , alignment(Alignment::Center)
  , bottom(false) {

  get_accent_color(&highlight_color);
  inactive_text_color = highlight_color;
}

static bool handle_bool(const char* value, bool* attr) {
  if (strcmp(value, "true") == 0) {
    *attr = true;
  } else if (strcmp(value, "false") == 0) {
    *attr = false;
  } else {
    return false; /* Error: unrecognized. */
  }
  return true;
}

static bool handle_color(const char* value, RGBColor* attr) {
  if (strlen(value) == 7 && value[0] == '#') {
    char* endptr = nullptr;
    errno = 0;
    RGBColor color = RGBColor(strtol(value + 1, &endptr, 16));
    if (*endptr == '\0' && errno == 0) {
      *attr = color;
      return true;
    }
    errno = 0;
  }
  return false;
}

static bool handle_positive_int(const char* value, int* attr) {
  char* endptr = nullptr;
  errno = 0;
  int x = strtol(value, &endptr, 10);
  if (*endptr == '\0' && errno == 0 && x > 0) {
    *attr = x;
    return true;
  }
  errno = 0;
  return false;
}

static bool handle_non_negative_int(const char* value, int* attr) {
  char* endptr = nullptr;
  errno = 0;
  int x = strtol(value, &endptr, 10);
  if (*endptr == '\0' && errno == 0 && x >= 0) {
    *attr = x;
    return true;
  }
  errno = 0;
  return false;
}

int config_entry_handler(void* user, const char* section, const char* name,
                         const char* value) {
  Config* config = (Config*)user;

#define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0

  if (MATCH("geometry", "height")) {
    if (!handle_positive_int(value, &config->height)) {
      return 0;
    }

  } else if (MATCH("geometry", "pad")) {
    if (!handle_non_negative_int(value, &config->pad)) {
      return 0;
    }

  } else if (MATCH("geometry", "label_width")) {
    if (!handle_positive_int(value, &config->label_width)) {
      return 0;
    }

  } else if (MATCH("geometry", "alignment")) {
    if (strcmp(value, "left") == 0) {
      config->alignment = Config::Alignment::Left;
    } else if (strcmp(value, "center") == 0) {
      config->alignment = Config::Alignment::Center;
    } else if (strcmp(value, "right") == 0) {
      config->alignment = Config::Alignment::Right;
    } else {
      return 0; // Error: unrecognized.
    }

  } else if (MATCH("geometry", "bottom")) {
    handle_bool(value, &config->bottom);

  } else if (MATCH("colors", "background")) {
    if (!handle_color(value, &config->background_color)) {
      return 0; // Error: invalid colour.
    }

  } else if (MATCH("colors", "highlight")) {
    if (strcmp(value, "auto") == 0) {
      get_accent_color(&config->highlight_color);
    } else if (!handle_color(value, &config->highlight_color)) {
      return 0; // Error: invalid colour.
    }

  } else if (MATCH("colors", "active")) {
    if (!handle_color(value, &config->active_text_color)) {
      return 0; // Error: invalid colour.
    }

  } else if (MATCH("colors", "inactive")) {
    if (strcmp(value, "auto") == 0) {
      get_accent_color(&config->inactive_text_color);
    } else if (!handle_color(value, &config->inactive_text_color)) {
      return 0; // Error: invalid colour.
    }

  } else if (MATCH("keybinds", "window_switch")) {
    if (!handle_bool(value, &config->window_switch)) {
      return 0;
    }

  } else if (MATCH("keybinds", "wraparound")) {
    if (!handle_bool(value, &config->window_switch_wrap)) {
      return 0;
    }

  } else {
    return 0; /* unknown section/name, error */
  }
  return 1;
}
