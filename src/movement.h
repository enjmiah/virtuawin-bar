#pragma once

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

void switch_window(const Command::Code direction);
