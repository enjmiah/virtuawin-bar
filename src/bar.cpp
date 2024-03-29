#include "bar.h"

#include "handler.h"

#include "VirtuaWin/defines.h"

#include <cairo-win32.h>
#include <tchar.h>

#include <stdint.h>
#include <stdio.h>

/** Return the number of bits set in v. */
static int popcount(uint32_t v) {
  int c = 0; // c accumulates the total bits set in v.
  for (; v; c++) {
    v &= v - 1; // Clear the least significant bit set.
  }
  return c;
}

static void draw_bar(cairo_t* const cr) {
  constexpr auto pi = 3.14159265359;
  constexpr auto degrees = pi / 180.0;
  const auto& state = get_state();
  const auto& config = state.config;
  const auto radius = config.corner_radius;
  const auto n_desktops = popcount(state.desktops);
  const auto width = n_desktops * config.label_width;

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
    cairo_set_font_size(
      cr,
      // Let's not include <algorithm> just for the sake of `min`...
      0.45 * (config.height < config.label_width ? config.height : config.label_width));
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

/** Handle WM_PAINT. */
static void paint(const HWND hwnd) {
  PAINTSTRUCT ps;
  auto* const hdc = BeginPaint(hwnd, &ps);

  // Create the cairo surface and context.
  auto* const surface = cairo_win32_surface_create(hdc);
  auto* const cr = cairo_create(surface);

  // Draw on the cairo context.
  draw_bar(cr);

  // Cleanup.
  cairo_destroy(cr);
  cairo_surface_destroy(surface);

  EndPaint(hwnd, &ps);
}

/** Handle window messages to the bar. */
static LRESULT wnd_proc(const HWND hwnd, const UINT msg, const WPARAM wParam,
                        const LPARAM lParam) {
  switch (msg) {
    case WM_PAINT:
      paint(hwnd);
      break;

    case WM_CLOSE:
      break;

    default:
      return DefWindowProc(hwnd, msg, wParam, lParam);
  }
  return 0;
}

void init_bar(State& state, const HINSTANCE instance, const HWND parent) {
  constexpr auto module_name = TEXT("vwbar");
  WNDCLASS wc = {0};
  wc.lpszClassName = module_name;
  wc.hInstance = instance;
  wc.lpfnWndProc = wnd_proc;
  wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
  RegisterClass(&wc);

  LONG screen_height;
  get_screen_resolution(nullptr, &screen_height);
  auto* const hwnd =
    CreateWindowEx(WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE, wc.lpszClassName, module_name,
                   WS_POPUP | WS_VISIBLE, 0, 0, 0, 0, parent, nullptr, instance, nullptr);
  state.bar_hwnd = hwnd;
  SetWindowLong(hwnd, GWL_STYLE, 0); // Remove title bar and border.
  resize_client(state);
  if (state.bar_topmost) {
    SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
  }

  ShowWindow(hwnd, SW_SHOWNA);
}

void destroy_bar(State& state) {
  DestroyWindow(state.bar_hwnd);
  state.bar_hwnd = nullptr;
}

void resize_client(const State& state) {
  static int old_width = -1;
  auto* const hwnd = state.bar_hwnd;
  if (hwnd && IsWindow(hwnd)) {
    const auto n_desktops = popcount(state.desktops);
    const auto width = n_desktops * (LONG)state.config.label_width;
    if (width != old_width) { // FIXME: Also needed if screen resolution changes!
      LONG screen_width;
      LONG screen_height;
      get_screen_resolution(&screen_width, &screen_height);
      const auto ypos =
        (state.config.bottom ? screen_height - state.config.pad - state.config.height
                             : state.config.pad);
      switch (state.config.alignment) {
        case Config::Alignment::Left:
          SetWindowPos(hwnd, nullptr, state.config.pad, ypos, width, state.config.height,
                       SWP_NOZORDER);
          break;
        case Config::Alignment::Center:
          SetWindowPos(hwnd, nullptr, (screen_width - width) / 2, ypos, width,
                       state.config.height, SWP_NOZORDER);
          break;
        case Config::Alignment::Right:
          SetWindowPos(hwnd, nullptr, screen_width - width - state.config.pad, ypos,
                       width, state.config.height, SWP_NOZORDER);
          break;
      }
      old_width = width;
    }
  }
}
