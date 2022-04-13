#include "handler.h"

#include <ctime>
#include <sstream>

constexpr auto module_name = TEXT("virtuawin-bar");
constexpr auto module_name_exe = TEXT("virtuawin-bar.exe");

struct Module {
  State state;
  decltype(&init) init;
  decltype(&destroy) destroy;
  decltype(&handle_message) handle_message;
  decltype(&handle_keypress) handle_keypress;
} mod;

bool dynamic_load_module(const HINSTANCE instance) {
  mod.init = init;
  mod.destroy = destroy;
  mod.handle_message = handle_message;
  mod.handle_keypress = handle_keypress;
  mod.init(instance, mod.state);

  return true;
}

int WINAPI WinMain(const HINSTANCE instance, HINSTANCE /*prev*/, LPSTR /*args*/,
                   const int) {
  // Create invisible parent window for receiving messages.
  WNDCLASS wc = {0};
  // The classname MUST be the same as the filename since VirtuaWin uses this for locating
  // the window.
  wc.lpszClassName = module_name_exe;
  wc.hInstance = instance;
  wc.lpfnWndProc = handle_message;
  wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
  RegisterClass(&wc);
  mod.state.messaging_hwnd = CreateWindow(wc.lpszClassName, module_name, NULL, 0, 0, 0, 0,
                                          nullptr, nullptr, instance, nullptr);

  if (!dynamic_load_module(instance)) {
    return 1;
  }

  MSG msg;
  BOOL ok;
  while ((ok = GetMessage(&msg, nullptr, 0, 0)) != 0) {
    if (ok) {
      mod.handle_keypress(msg);
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    } else {
      OutputDebugStringA("GetMessage failed.");
    }
  }

  return 0;
}
