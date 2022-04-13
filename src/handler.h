#pragma once

#include "config.h"

#include "VirtuaWin/defines.h"

#include <Windows.h>

#include <cstdint>

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
  // Whether the bar should be kept on top always.
  bool bar_topmost = true;

  // Desktop i has at least one window if and only if desktops & (1 << i) != 0.
  uint32_t desktops = 0;
  static_assert(vwDESKTOP_MAX < 32,
                "cannot store desktop set in an uint32, use uint64 or bigger instead");
  // Index of currently active desktop.
  uint8_t active_desktop = 0;
};

void init(HINSTANCE instance, State& init_state);

LRESULT handle_message(HWND, UINT msg, WPARAM, LPARAM);

void handle_keypress(const MSG& msg);

void destroy();

State& get_state();

// TODO: Probably belongs somewhere else...
void get_screen_resolution(LONG* out_width, LONG* out_height);
