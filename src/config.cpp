#include "config.h"

#include <dwmapi.h>

namespace vwtiling {

Config::Config()
  : corner_radius(0) // TODO: Transparency is janky.
  , height(26)
  , pad(10)
  , label_width(height)
  , outer_gap(16)
  , alignment(Alignment::Center)
  , wraparound(false) {
  DWORD active_caption; // AARRGGBB
  BOOL opaque_blend;
  if (DwmGetColorizationColor(&active_caption, &opaque_blend) == S_OK) {
    highlight_color.r = ((active_caption >> 16) & 0xFF) / 255.f;
    highlight_color.g = ((active_caption >> 8) & 0xFF) / 255.f;
    highlight_color.b = (active_caption & 0xFF) / 255.f;
  }
  inactive_text_color = highlight_color;
}

} // namespace vwtiling
