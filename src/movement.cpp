#include "movement.h"

#include "handler.h"

#include <Windows.h>
#include <dwmapi.h>

#include <math.h>
#include <stdio.h>

struct Vec2 {
  double x;
  double y;

  Vec2()
    : x(0)
    , y(0) {}

  Vec2(double new_x, double new_y)
    : x(new_x)
    , y(new_y) {}
};

/** Get the centroid of a window. */
static Vec2 centroid(const HWND hwnd) {
  RECT rect;
  GetWindowRect(hwnd, &rect);
  return Vec2(0.5 * (rect.left + rect.right), 0.5 * (rect.top + rect.bottom));
}

static bool is_cloaked(const HWND hwnd) {
  BOOL cloaked = false;
  HRESULT err = DwmGetWindowAttribute(hwnd, DWMWA_CLOAKED, &cloaked, sizeof(cloaked));
  return (err == S_OK ? (bool)cloaked : false);
}

static bool is_zero_area(const HWND hwnd) {
  RECT rect;
  GetWindowRect(hwnd, &rect);
  return rect.left + 1 == rect.right || rect.top + 1 == rect.bottom;
}

static bool is_of_skipped_style(const HWND hwnd) {
  const auto exStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
  return exStyle & WS_EX_TOOLWINDOW;
}

/** Return true iff this window should not be switched to. */
static bool skip_window(const HWND hwnd) {
  return !IsWindowVisible(hwnd) || IsIconic(hwnd) || GetWindowTextLength(hwnd) == 0 ||
         is_cloaked(hwnd) || is_of_skipped_style(hwnd) || is_zero_area(hwnd);
}

static Vec2 search_direction;
static Vec2 wraparound;
static HWND active;
static HWND best_window;
static double best_score;
static HWND tiebreaker;
static bool tiebreaker_valid;
static Vec2 active_centroid;

static BOOL CALLBACK window_callback(HWND hwnd, LPARAM) {
  if (!skip_window(hwnd)) {
    auto& state = get_state();
    const HWND candidate = hwnd;
    const Vec2 candidate_centroid = centroid(candidate);
    auto disp_x = candidate_centroid.x - active_centroid.x;
    auto disp_y = candidate_centroid.y - active_centroid.y;
    if (disp_x * search_direction.x + disp_y * search_direction.y < 0) {
      if (state.config.window_switch_wrap) {
        // Consider the window's displacement as if the screen wrapped around
        disp_x += wraparound.x;
        disp_y += wraparound.y;
      } else {
        disp_x = disp_y = INFINITY;
      }
    }
    if (disp_x == 0 && disp_y == 0) {
      if (candidate == active) {
        return TRUE;
      }
      // We tie-break using HWND so that all windows are reachable if there are windows
      // perfectly on top of each other.
      if (search_direction.x + search_direction.y > 0) {
        if (candidate > active && (!tiebreaker_valid || candidate < tiebreaker)) {
          best_window = candidate;
          best_score = INFINITY;
          tiebreaker = candidate;
          tiebreaker_valid = true;
        }
      } else {
        if (candidate < active && (!tiebreaker_valid || candidate > tiebreaker)) {
          best_window = candidate;
          best_score = INFINITY;
          tiebreaker = candidate;
          tiebreaker_valid = true;
        }
      }
    } else {
      if (tiebreaker_valid)
        return TRUE;

      const auto disp_len_fac = disp_x * disp_x + disp_y * disp_y;
      if (disp_len_fac < 1e-6) {
        OutputDebugStringA("Displacement to a window was very small.\n");
        return TRUE;
      }
      // dot(disp, search_direction) is bigger when the window is in a similar direction
      // as the search direction.
      // 1 / disp_len_fac is bigger when the windows are close.
      const auto score = (disp_x * search_direction.x / disp_len_fac +
                          disp_y * search_direction.y / disp_len_fac);
      if (score > best_score) {
        best_window = candidate;
        best_score = score;
      }
    }
  }
  return TRUE;
}

void switch_window(const Command::Code direction) {
  active = GetForegroundWindow();

  // The key idea here is that a good window to switch to is both proximally close and
  // in the general direction we are moving in.

  LONG screen_width, screen_height;
  get_screen_resolution(&screen_width, &screen_height);
  switch (direction) {
    case Command::SwitchLeft:
      search_direction = Vec2(-1, 0);
      wraparound = Vec2((double)-screen_width, 0);
      break;
    case Command::SwitchRight:
      search_direction = Vec2(1, 0);
      wraparound = Vec2((double)screen_width, 0);
      break;
    case Command::SwitchUp:
      search_direction = Vec2(0, -1);
      wraparound = Vec2(0, (double)-screen_height);
      break;
    case Command::SwitchDown:
      search_direction = Vec2(0, 1);
      wraparound = Vec2(0, (double)screen_height);
      break;
    default:
      OutputDebugStringA("Unknown switch direction.\n");
      return;
  }

  best_window = active;
  best_score = -INFINITY;
  tiebreaker = nullptr;
  tiebreaker_valid = false;
  active_centroid = centroid(active);
  EnumWindows(window_callback, NULL);
  SetForegroundWindow(best_window);
}
