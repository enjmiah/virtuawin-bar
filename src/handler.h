#pragma once

#include "config.h"
#include "macros.h"

#include "VirtuaWin/defines.h"

#include <Windows.h>

#include <cstdint>

namespace vwbar {

struct State {
  Config config;
  // Flags.
  uint8_t initialized = 0;
  // Handle to VirtuaWin.
  HWND vw_handle = nullptr;
  // Handle to the bar window (null if non-existent).
  HWND bar_hwnd = nullptr;
  // User's config path.
  TCHAR user_app_path[MAX_PATH];

  // Desktop i has at least one window if and only if desktops & (1 << i) != 0.
  std::uint32_t desktops = 0;
  static_assert(vwDESKTOP_MAX < 32,
                "cannot store desktop set in an int32, use int64 or bigger instead");
  // Index of currently active desktop.
  std::uint8_t active_desktop = 0;
};

struct Module {
  State state;
};

VWBAR_API void init(HINSTANCE instance);

} // namespace vwbar
