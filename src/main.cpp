#include "handler.h"

int WINAPI WinMain(const HINSTANCE instance, HINSTANCE /*prev*/, LPSTR /*args*/,
                   const int) {
  ::vwbar::init(instance);

  MSG msg;
  while (GetMessage(&msg, nullptr, 0, 0) > 0) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  return 0;
}
