#pragma once

#include "config.h"

#include "VirtuaWin/defines.h"

#include <Windows.h>

#include <cstdint>

#ifdef VWBAR_EXPORTS
  #define VWBAR_API extern "C" __declspec(dllexport)
#else
  #define VWBAR_API extern "C" __declspec(dllimport)
#endif

namespace vwbar {

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
};

VWBAR_API void init(HINSTANCE instance, State& init_state);

VWBAR_API LRESULT handle_message(HWND, UINT msg, WPARAM, LPARAM);

VWBAR_API void destroy();

State& get_state();

} // namespace vwbar
