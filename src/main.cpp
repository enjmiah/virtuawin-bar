#include "handler.h"

namespace vb = ::vwbar;

struct Module {
  HMODULE dll;
  decltype(&vb::init) init;
} mod;

int WINAPI WinMain(const HINSTANCE instance, HINSTANCE /*prev*/, LPSTR /*args*/,
                   const int) {
  mod.dll = LoadLibraryA("vwbar.dll");
  if (!mod.dll) {
    MessageBoxA(nullptr, "Could not load vwbar.dll", nullptr, MB_OK);
    return 1;
  }
  mod.init = (decltype(&vb::init))GetProcAddress(mod.dll, "init");
  if (!mod.init) {
    MessageBoxA(nullptr, "Could not load vwbar.dll:init", nullptr, MB_OK);
    return 1;
  }

  mod.init(instance);

  MSG msg;
  while (GetMessage(&msg, nullptr, 0, 0) > 0) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  return 0;
}
