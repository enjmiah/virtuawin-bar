#include "bar.h"

#include "handler.h"

#include "VirtuaWin/defines.h"

#include <cairo-win32.h>

#include <cstdint>
#include <cstdio>

using namespace ::std;

namespace {

/// Return the number of bits set in v.
int popcount(std::uint32_t v) {
  int c; // c accumulates the total bits set in v
  for (c = 0; v; c++) {
    v &= v - 1; // clear the least significant bit set
  }
  return c;
}

} // namespace

namespace vwbar {

void resize_client(const State& state) {
  static int old_width = -1;
  auto* const hwnd = state.bar_hwnd;
  if (hwnd && IsWindow(hwnd)) {
    const auto style = DWORD(GetWindowLongPtr(hwnd, GWL_STYLE));
    const auto ex_style = DWORD(GetWindowLongPtr(hwnd, GWL_EXSTYLE));
    const HMENU menu = GetMenu(hwnd);
    const auto width = popcount(state.desktops) * (LONG)state.config.label_width;
    if (width != old_width) {
      RECT rc = {0, 0, width, state.config.height};
      AdjustWindowRectEx(&rc, style, menu ? TRUE : FALSE, ex_style);
      SetWindowPos(hwnd, nullptr, 0, 0, rc.right - rc.left, rc.bottom - rc.top,
                   SWP_NOZORDER | SWP_NOMOVE);
    }
  }
}

void draw_bar(cairo_t* const cr, const State& state) {
  const auto& config = state.config;
  constexpr auto pi = 3.14159265359;
  constexpr auto degrees = pi / 180.0;
  const auto radius = state.config.corner_radius;
  const auto n_desktops = popcount(state.desktops);
  const auto width = n_desktops * config.height;

  cairo_new_sub_path(cr);
  cairo_arc(cr, width - radius, radius, radius, -90 * degrees, 0 * degrees);
  cairo_arc(cr, width - radius, config.height - radius, radius, 0 * degrees,
            90 * degrees);
  cairo_arc(cr, radius, config.height - radius, radius, 90 * degrees, 180 * degrees);
  cairo_arc(cr, radius, radius, radius, 180 * degrees, 270 * degrees);
  cairo_close_path(cr);

  cairo_set_source_rgb(cr, config.background_color.r, config.background_color.g,
                       config.background_color.b);
  cairo_fill(cr);

  // Convert desktops to an array of ids.
  uint8_t desk_ids[vwDESKTOP_MAX];
  uint8_t desk_ids_size = 0;
  for (auto i = uint8_t(1); i < vwDESKTOP_MAX + 1; ++i) {
    if (state.desktops & (1 << i)) {
      desk_ids[desk_ids_size] = i;
      desk_ids_size++;
    }
  }

  for (auto i = uint8_t(0); i < desk_ids_size; ++i) {
    const auto desktop = desk_ids[i];
    const auto& text_color =
      (desktop == state.active_desktop ? config.active_text_color
                                       : config.inactive_text_color);
    if (desktop == state.active_desktop) {
      // Draw active.
      cairo_set_source_rgb(cr, config.highlight_color.r, config.highlight_color.g,
                           config.highlight_color.b);
      cairo_rectangle(cr, double(i * config.label_width), 0, config.label_width,
                      config.height);
      cairo_fill(cr);
    }
    cairo_set_source_rgb(cr, text_color.r, text_color.g, text_color.b);
    cairo_set_font_size(cr, 0.45 * config.height);
    cairo_select_font_face(cr, "Segoe UI", CAIRO_FONT_SLANT_NORMAL,
                           CAIRO_FONT_WEIGHT_BOLD);
    char label_text[16];
    snprintf(label_text, 16, "%d", (int)desktop);
    cairo_text_extents_t te;
    cairo_text_extents(cr, label_text, &te);
    cairo_move_to(cr, (i + 0.5) * config.label_width - 0.5 * te.width - te.x_bearing,
                  0.5 * (config.height - te.height) - te.y_bearing);
    cairo_show_text(cr, label_text);
  }
}

void paint(const HWND hwnd, const State& state) {
  PAINTSTRUCT ps;
  auto* const hdc = BeginPaint(hwnd, &ps);

  // Create the cairo surface and context.
  auto* const surface = cairo_win32_surface_create(hdc);
  auto* const cr = cairo_create(surface);

  // Draw on the cairo context.
  draw_bar(cr, state);

  // Cleanup.
  cairo_destroy(cr);
  cairo_surface_destroy(surface);

  EndPaint(hwnd, &ps);
}

} // namespace vwbar
