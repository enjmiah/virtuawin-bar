#include "tiling.h"

#include "handler.h"
#include "logging.h"

#include <Windows.h>
#include <dwmapi.h>

#include <sstream>
#include <vector>

namespace {

/**
 * @return Centroid of window.
 */
auto centroid(const HWND hwnd) {
  RECT rect;
  GetWindowRect(hwnd, &rect);
  return std::make_pair<double, double>(0.5 * ((std::int64_t)rect.left + rect.right),
                                        0.5 * ((std::int64_t)rect.top + rect.bottom));
}

auto get_screen_resolution() {
  RECT rect;
  GetWindowRect(GetDesktopWindow(), &rect);
  return std::make_pair(rect.right, rect.bottom);
}

BOOL is_cloaked(const HWND hwnd) {
  BOOL cloaked = false;
  HRESULT err = DwmGetWindowAttribute(hwnd, DWMWA_CLOAKED, &cloaked, sizeof(cloaked));
  if (err != S_OK) {
    std::stringstream ss;
    ss << "DwnGetWindowAttribute returned " << std::hex << err;
    const auto s = ss.str();
    vwtiling::log_error(s.c_str());
  }
  return cloaked;
}

bool is_zero_area(const HWND hwnd) {
  RECT rect;
  GetWindowRect(hwnd, &rect);
  return rect.left + 1 == rect.right || rect.top + 1 == rect.bottom;
}

bool is_of_skipped_style(const HWND hwnd) {
  const auto exStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
  return exStyle & WS_EX_TOOLWINDOW;
}

/**
 * @return true iff this window should not be handled by doewm in any way
 */
bool skip_window(const HWND hwnd) {
  return !IsWindowVisible(hwnd) || IsIconic(hwnd) || GetWindowTextLength(hwnd) == 0 ||
         is_cloaked(hwnd) || is_of_skipped_style(hwnd) || is_zero_area(hwnd);
}

BOOL CALLBACK get_floating_windows_callback(HWND hwnd, LPARAM lpParam) {
  const auto& tiledWindows = vwtiling::get_state().tiling.tiled_windows;
  if (!skip_window(hwnd) &&
      std::find(tiledWindows.begin(), tiledWindows.end(), hwnd) == tiledWindows.end()) {
    auto* vec = reinterpret_cast<std::vector<HWND>*>(lpParam);
    vec->emplace_back(hwnd);
  }
  return TRUE;
}

BOOL get_floating_windows(std::vector<HWND>& out) {
  return EnumWindows(get_floating_windows_callback, reinterpret_cast<LPARAM>(&out));
}

} // namespace

namespace vwtiling {

void init_tiling(State&) {
  if (!RegisterHotKey(nullptr, Command::SwitchLeft, MOD_ALT, 0x4A)) {
    log_error("Hotkey 'Alt+J' could not be registered");
  }
  if (!RegisterHotKey(nullptr, Command::SwitchRight, MOD_ALT, VK_OEM_1)) {
    log_error("Hotkey 'Alt+;' could not be registered");
  }
  if (!RegisterHotKey(nullptr, Command::SwitchUp, MOD_ALT, 0x4C)) {
    log_error("Hotkey 'Alt+L' could not be registered");
  }
  if (!RegisterHotKey(nullptr, Command::SwitchDown, MOD_ALT, 0x4B)) {
    log_error("Hotkey 'Alt+K' could not be registered");
  }
  log_debug("Hotkeys registered.");
}

void destroy_tiling(State& state) {
  UnregisterHotKey(nullptr, Command::SwitchLeft);
  UnregisterHotKey(nullptr, Command::SwitchRight);
  UnregisterHotKey(nullptr, Command::SwitchUp);
  UnregisterHotKey(nullptr, Command::SwitchDown);
}

void switch_window(const Command::Code direction) {
  const HWND active = GetForegroundWindow();
  auto& state = get_state();
  auto& tiled_windows = state.tiling.tiled_windows;
  const auto it = std::find(tiled_windows.begin(), tiled_windows.end(), active);

  std::vector<HWND> floating_candidates;
  if (it == tiled_windows.end()) {
    // Current window is floating
    if (!get_floating_windows(floating_candidates)) {
      log_error("Failed to enumerate floating windows\n");
      return;
    }
  }
  const std::vector<HWND>& candidates =
    (it == tiled_windows.end() ? floating_candidates : tiled_windows);

  if (candidates.size() <= 1) {
    // Nothing to switch to
    return;
  }

  // The key idea here is that a good window to switch to is both proximally close and
  // in the general direction we are moving in.

  const auto resolution = get_screen_resolution();
  std::pair<double, double> search_direction;
  std::pair<double, double> wraparound;
  switch (direction) {
    case Command::SwitchLeft:
      search_direction = std::make_pair(-1.f, 0.f);
      wraparound = std::make_pair((double)-resolution.first, 0.f);
      break;
    case Command::SwitchRight:
      search_direction = std::make_pair(1.f, 0.f);
      wraparound = std::make_pair((double)resolution.first, 0.f);
      break;
    case Command::SwitchUp:
      search_direction = std::make_pair(0.f, -1.f);
      wraparound = std::make_pair(0.f, (double)-resolution.second);
      break;
    case Command::SwitchDown:
      search_direction = std::make_pair(0.f, 1.f);
      wraparound = std::make_pair(0.f, (double)resolution.second);
      break;
    default:
      log_error("Unknown switch direction");
      return;
  }

  HWND best_window = active;
  auto best_score = -std::numeric_limits<double>::infinity();
  HWND tiebreaker = nullptr;
  bool tiebreaker_valid = false;
  const auto active_centroid = centroid(active);

  for (const auto candidate : candidates) {
    const auto candidate_centroid = centroid(candidate);
    auto disp_x = candidate_centroid.first - active_centroid.first;
    auto disp_y = candidate_centroid.second - active_centroid.second;
    if (disp_x * search_direction.first + disp_y * search_direction.second < 0.f) {
      // Consider the window's displacement as if the screen wrapped around
      disp_x += wraparound.first;
      disp_y += wraparound.second;
    }
    if (disp_x == 0.f && disp_y == 0.f) {
      if (candidate == active) {
        continue;
      }
      // We tiebreak using HWND so that all windows are reachable if there are windows
      // perfectly on top of eachother
      if (search_direction.first + search_direction.second > 0.f) {
        if (candidate > active && (!tiebreaker_valid || candidate < tiebreaker)) {
          best_window = candidate;
          best_score = std::numeric_limits<double>::infinity();
          tiebreaker = candidate;
          tiebreaker_valid = true;
        }
      } else {
        if (candidate < active && (!tiebreaker_valid || candidate > tiebreaker)) {
          best_window = candidate;
          best_score = std::numeric_limits<double>::infinity();
          tiebreaker = candidate;
          tiebreaker_valid = true;
        }
      }
    } else {
      if (tiebreaker_valid)
        continue;

      const auto disp_len_fac = disp_x * disp_x + disp_y * disp_y;
      if (disp_len_fac < 1e-6) {
        log_error("Displacement to a window was very small");
        return;
      }
      // dot(disp, search_direction) is bigger when the window is in a similar direction
      // as the search direction
      // 1 / disp_len_fac is bigger when the windows are close
      const auto score = (disp_x * search_direction.first / disp_len_fac +
                          disp_y * search_direction.second / disp_len_fac);
      if (score > best_score) {
        best_window = candidate;
        best_score = score;
      }
    }
  }
  SetForegroundWindow(best_window);
}

void toggle_tile(HWND active) {
  if (skip_window(active)) {
    log_error("Could not tile current window.");
    return;
  }
  auto& tiled_windows = get_state().tiling.tiled_windows;
  const auto it = std::find(tiled_windows.begin(), tiled_windows.end(), active);
  if (it == tiled_windows.end()) {
    tiled_windows.push_back(active);
  } else {
    const auto hwnd = *it;
    tiled_windows.erase(it);
  }
}

} // namespace vwtiling
