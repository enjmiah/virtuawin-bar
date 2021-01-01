#include "movement.h"

#include "handler.h"

#include <Windows.h>

#include <vector>

namespace {

void log_error(const char* str) { OutputDebugStringA(str); }

/// Return the centroid of a window.
std::pair<double, double> centroid(const HWND hwnd) {
  RECT rect;
  GetWindowRect(hwnd, &rect);
  return {0.5 * ((std::int64_t)rect.left + rect.right),
          0.5 * ((std::int64_t)rect.top + rect.bottom)};
}

auto get_screen_resolution() {
  RECT rect;
  GetWindowRect(GetDesktopWindow(), &rect);
  return std::make_pair(rect.right, rect.bottom);
}

bool skip_window(const HWND hwnd) {
  return !IsWindowVisible(hwnd) || IsIconic(hwnd) || GetWindowTextLength(hwnd) == 0 ||
         is_cloaked(hwnd) || is_of_skipped_style(hwnd) || is_zero_area(hwnd);
}

BOOL CALLBACK get_floating_windows_callback(HWND hwnd, LPARAM lpParam) {
  if (!skip_window(hwnd) &&
      std::find(tiledWindows.begin(), tiledWindows.end(), hwnd) == tiledWindows.end()) {
    log_window(hwnd); // DEBUG:
    auto vec = reinterpret_cast<std::vector<HWND>*>(lpParam);
    vec->emplace_back(hwnd);
  }
  return TRUE;
}

BOOL get_floating_windows(std::vector<HWND>& out) {
  return EnumWindows(get_floating_windows_callback, reinterpret_cast<LPARAM>(&out));
}

} // namespace

namespace vwtiling {


} // namespace vwtiling