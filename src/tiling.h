#pragma once

#include <Windows.h>

#include <vector>

namespace vwtiling {

struct State;

struct Command {
  enum Code { //
    ToggleTile,
    SwitchLeft,
    SwitchRight,
    SwitchUp,
    SwitchDown,
    Count
  };
};

struct Dimensions {
  int x, y, width, height;
};

struct Window {
  HWND window;
  /// Original dimensions.
  Dimensions orig_dims;

  Window(HWND);
};

struct Tile {
  Window window;

  Tile(HWND);

  bool remove_window(HWND);

  [[nodiscard]] bool empty() const { return window.window != nullptr; }
};

struct Column {
  std::vector<Tile> tiles;

  bool remove_window(HWND);
};

struct Layout {
  std::vector<Column> columns;

  void tile(HWND);

  bool remove_window(HWND);
};

struct TilingState {
  Layout layout;
  // These could also be unordered sets (but need to profile first).
  std::vector<HWND> tiled_windows;
  std::vector<HWND> floating_windows;
};

void init_tiling(State& state);

void destroy_tiling(State& state);

void switch_window(Command::Code direction);

void toggle_tile(HWND);

} // namespace vwtiling
