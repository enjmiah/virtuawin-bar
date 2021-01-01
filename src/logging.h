#pragma once

#include <Windows.h>

namespace vwtiling {

inline void log_error(const char* str) {
  MessageBoxA(nullptr, str, "vwtiling", 0);
  // OutputDebugStringA(str);
}

inline void log_debug(const char* str) { OutputDebugStringA(str); }

} // namespace vwtiling
