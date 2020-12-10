#include "VirtuaWin/defines.h"
#include "VirtuaWin/messages.h"
#include "config.h"

#include <Windows.h>
#include <cairo-win32.h>
#include <tchar.h>

#include <cstdint>
#include <cstdio>

using namespace virtuawinbar;
using namespace std;

namespace {

constexpr auto module_name = _T("virtuawin-bar");
constexpr auto module_name_exe = _T("virtuawin-bar.exe");

/////////////
// Globals //
/////////////

struct State {
  Config config;
  // Flags.
  uint8_t initialized = 0;
  // Handle to VirtuaWin.
  HWND vw_handle = nullptr;
  // User's config path.
  TCHAR user_app_path[MAX_PATH];

  // Desktop i has at least one window if and only if desktops & (1 << i) != 0.
  uint32_t desktops = 0;
  static_assert(vwDESKTOP_MAX < 32,
                "cannot store desktop set in an int32, use int64 or bigger instead");
  // Index of currently active desktop.
  uint8_t active_desktop = 0;
};

struct Module {
  State state;
};

Module mod;

///////////////
// Functions //
///////////////

/// Return the number of bits set in v.
int popcount(uint32_t v) {
  int c; // c accumulates the total bits set in v
  for (c = 0; v; c++) {
    v &= v - 1; // clear the least significant bit set
  }
  return c;
}

/// Change the dimensions of the bar.
void resize_client(const HWND hwnd) {
  const auto& state = mod.state;
  static int old_width = -1;
  if (IsWindow(hwnd)) {
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

__inline BOOL CALLBACK enum_windows_proc(const HWND hwnd, const LPARAM desk_count) {
  auto& state = mod.state;
  if (state.vw_handle) {
    const auto style = GetWindowLong(hwnd, GWL_STYLE);
    if (!(style & WS_CHILD)) {
      const auto flag = SendMessage(state.vw_handle, VW_WINGETINFO, (WPARAM)hwnd, 0);
      if (flag && (flag & vwWINFLAGS_MANAGED)) {
        const auto desk = vwWindowGetInfoDesk(flag);
        if (desk != state.active_desktop && desk > desk_count) {
          return TRUE; // On a hidden desktop, so skip.
        }
        state.desktops |= 1 << desk;
      }
    }
  }
  return TRUE;
}

void update_desktop_set() {
  mod.state.desktops = 0; // Clear.
  const auto desk_count = SendMessage(mod.state.vw_handle, VW_DESKX, 0, 0) *
                          SendMessage(mod.state.vw_handle, VW_DESKY, 0, 0);
  EnumWindows(enum_windows_proc, desk_count);
}

void draw_rounded_background(cairo_t* const cr) {
  auto& state = mod.state;
  const auto& config = state.config;
  constexpr auto pi = 3.14159265359;
  constexpr auto degrees = pi / 180.0;
  const auto radius = state.config.corner_radius;
  const auto n_desktops = popcount(state.desktops);
  const auto width = n_desktops * config.height;

  cairo_new_sub_path(cr);
  cairo_arc(cr, width - radius, radius, radius, -90 * degrees, 0 * degrees);
  cairo_arc(cr, width - radius, state.config.height - radius, radius, 0 * degrees,
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

/// Handle WM_PAINT.
void paint(const HWND hwnd, WPARAM wParam, LPARAM lParam) {
  PAINTSTRUCT ps;
  const auto hdc = BeginPaint(hwnd, &ps);

  // Create the cairo surface and context.
  auto* const surface = cairo_win32_surface_create(hdc);
  auto* const cr = cairo_create(surface);

  // Draw on the cairo context.
  draw_rounded_background(cr);

  // Cleanup.
  cairo_destroy(cr);
  cairo_surface_destroy(surface);

  EndPaint(hwnd, &ps);
}

/// Handle our window's messages.
LRESULT CALLBACK wnd_proc(const HWND hwnd, const UINT msg, const WPARAM wParam,
                          const LPARAM lParam) {
  switch (msg) {
    case MOD_INIT: {
      // This must be taken care of in order to get the handle to VirtuaWin.
      // The handle to VirtuaWin comes in the wParam.
      mod.state.vw_handle = (HWND)wParam;
      if (!mod.state.vw_handle) {
        MessageBox(hwnd, _T("Failed to get handle to VirtuaWin."), _T("Module Error"),
                   MB_ICONWARNING);
        exit(1);
      }
      // wParam == 1: Set self as desktop change handler.
      SendMessage(mod.state.vw_handle, VW_ICHANGEDESK, 1, 0);
      // Unmanage self.
      SendMessage(mod.state.vw_handle, VW_WINMANAGE, (WPARAM)hwnd, 0);
      if (!mod.state.initialized) {
        SendMessage(mod.state.vw_handle, VW_USERAPPPATH, (WPARAM)hwnd, 0);
        // TODO: Is a delay necessary here?
        if ((mod.state.initialized & 2) == 0) {
          MessageBox(hwnd,
                     _T("VirtuaWin failed to send the UserApp path to VirtaWin Bar."),
                     _T("Module Error"), MB_ICONWARNING);
          exit(1);
        }
      }
      // Initialize desktop set.
      update_desktop_set();
      mod.state.active_desktop =
        (uint8_t)SendMessage(mod.state.vw_handle, VW_CURDESK, 0, 0);
      mod.state.desktops |= 1 << mod.state.active_desktop;
      resize_client(hwnd);
      InvalidateRect(hwnd, nullptr, true);
    } break;

    case WM_COPYDATA: {
      auto* const cds = (COPYDATASTRUCT*)lParam;
      if ((int)cds->dwData == (0 - VW_USERAPPPATH) && (mod.state.initialized & 2) == 0) {
        if (cds->cbData < 2 || !cds->lpData) {
          return FALSE;
        }
        mod.state.initialized |= 2;
#ifdef _UNICODE
        MultiByteToWideChar(CP_ACP, 0, (char*)cds->lpData, -1, user_app_path, MAX_PATH);
#else
        strncpy(mod.state.user_app_path, (char*)cds->lpData, MAX_PATH);
#endif
        mod.state.user_app_path[MAX_PATH - 1] = '\0';
      }
      return TRUE;
    } break;

    case WM_PAINT:
      paint(hwnd, wParam, lParam);
      break;

    case MOD_CFGCHANGE:
      break;

    case MOD_CHANGEDESK: {
      // VirtuaWin might sent two MOD_CHANGEDESK messages in the case of a wrap-around,
      // but we don't care about that.  So we only handle the first one.
      if (mod.state.active_desktop != lParam) {
        mod.state.active_desktop = decltype(mod.state.active_desktop)(lParam);
        // wParam == 3: Execute the desktop change.
        SendMessage(mod.state.vw_handle, VW_ICHANGEDESK, 3, 0);
        update_desktop_set();
        mod.state.desktops |= 1 << mod.state.active_desktop;
        resize_client(hwnd);
        InvalidateRect(hwnd, nullptr, true);
        // UpdateWindow doesn't seem to be necessary here.
      }
    } break;

    case MOD_SETUP: {
      // Displays when user presses Setup > Modules > Configure.
      TCHAR message[64 + MAX_PATH];
      snprintf(message, 64 + MAX_PATH,
               "The configuration file for virtuawin-bar is located at %sbar.json",
               mod.state.user_app_path);
      MessageBox(hwnd, message, module_name, MB_ICONINFORMATION);
    } break;

    case MOD_QUIT: // This must be handled, otherwise VirtuaWin can't shut down the
                   // module.
    case WM_CLOSE:
    case WM_DESTROY:
      // wParam == 2: Remove self as desktop change handler.
      SendMessage(mod.state.vw_handle, VW_ICHANGEDESK, 2, 0);
      PostQuitMessage(0);
      break;

    default:
      return DefWindowProc(hwnd, msg, wParam, lParam);
  }
  return 0;
}

void get_screen_resolution(LONG* out_width, LONG* out_height) {
  auto* const desktop = GetDesktopWindow();
  RECT rect;
  GetWindowRect(desktop, &rect);
  if (out_width)
    *out_width = rect.right;
  if (out_height)
    *out_height = rect.bottom;
}

} // namespace

int WINAPI WinMain(const HINSTANCE hInstance, const HINSTANCE prev, const LPSTR args,
                   const int) {
  WNDCLASS wc = {0};
  // The classname MUST be the same as the filename since VirtuaWin uses this for locating
  // the window.
  wc.lpszClassName = module_name_exe;
  wc.hInstance = hInstance;
  wc.lpfnWndProc = wnd_proc;
  wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
  RegisterClass(&wc);

  LONG screen_height;
  get_screen_resolution(nullptr, &screen_height);
  const auto& config = mod.state.config;
  const auto hwnd = CreateWindowEx(WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE, wc.lpszClassName,
                                   module_name, WS_POPUP | WS_VISIBLE, config.pad,
                                   screen_height - config.height - config.pad, 0, 0,
                                   nullptr, nullptr, hInstance, nullptr);
  SetWindowLong(hwnd, GWL_STYLE, 0); // Remove title bar and border.
  resize_client(hwnd);
  SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0,
               SWP_NOMOVE | SWP_NOSIZE); // Always on top.

  ShowWindow(hwnd, SW_SHOWNA);

  MSG msg;
  while (GetMessage(&msg, nullptr, 0, 0) > 0) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  return 0;
}
