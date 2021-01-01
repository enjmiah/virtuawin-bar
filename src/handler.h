#pragma once

#include "config.h"
#include "tiling.h"

#include "VirtuaWin/defines.h"

#include <Windows.h>

#include <cstdint>
#include <vector>

#ifdef VWTILING_HOT_RELOAD
  #ifdef VWTILING_EXPORTS
    #define VWTILING_API extern "C" __declspec(dllexport)
  #else
    #define VWTILING_API extern "C" __declspec(dllimport)
  #endif
#else
  #define VWTILING_API
#endif

namespace vwtiling {

struct State {
  Config config;
  // Flags.
  uint8_t initialized = 0;
  // Handle to VirtuaWin.
  HWND vw_handle = nullptr;
  // User's config path.
  TCHAR user_app_path[MAX_PATH];
  // Handle to messaging window.
  HWND messaging_hwnd = nullptr;
  // Handle to the bar window (null if non-existent).
  HWND bar_hwnd = nullptr;

  // Desktop i has at least one window if and only if desktops & (1 << i) != 0.
  std::uint32_t desktops = 0;
  static_assert(vwDESKTOP_MAX < 32,
                "cannot store desktop set in an uint32, use uint64 or bigger instead");
  // Index of currently active desktop.
  std::uint8_t active_desktop = 0;

  TilingState tiling;
};

VWTILING_API void init(HINSTANCE instance, State& init_state);

VWTILING_API LRESULT handle_message(HWND, UINT msg, WPARAM, LPARAM);

VWTILING_API void handle_keypress(const MSG& msg);

VWTILING_API void destroy();

State& get_state();

} // namespace vwtiling
