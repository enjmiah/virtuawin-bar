#pragma once

#include <Windows.h>

namespace vwbar {

struct State;

void init_bar(State& state, HINSTANCE instance, HWND parent);

/// Change the dimensions of the bar.
void resize_client(const State&);

} // namespace vwbar
