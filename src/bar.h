#pragma once

#include <Windows.h>

struct State;

void init_bar(State& state, HINSTANCE instance, HWND parent);

void destroy_bar(State& state);

/// Change the dimensions of the bar.
void resize_client(const State&);
