#pragma once

#include <Windows.h>

namespace vwtiling {

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

struct State;

void init_tiling(State& state);

void destroy_tiling(State& state);

void switch_window(Command::Code direction);

void toggle_tile(HWND window);

} // namespace vwtiling
