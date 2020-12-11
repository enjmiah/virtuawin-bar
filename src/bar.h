#pragma once

#include "macros.h"

#include <Windows.h>

// TODO: This forward declaration might be fragile.
struct _cairo;
typedef struct _cairo cairo_t;

namespace vwbar {

struct State;

/// Change the dimensions of the bar.
VWBAR_API void resize_client(const State&);

/// Handle WM_PAINT.
VWBAR_API void paint(HWND, const State&);

} // namespace vwbar
