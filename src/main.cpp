#include "handler.h"

#include <ctime>
#include <sstream>

namespace vt = ::vwtiling;

constexpr auto module_name = "virtuawin-tiling";
constexpr auto module_name_exe = "virtuawin-tiling.exe";

struct Module {
  vt::State state;
#ifdef VWTILING_HOT_RELOAD
  HMODULE dll = nullptr;
  FILETIME load_time;
#endif
  decltype(&vt::init) init;
  decltype(&vt::destroy) destroy;
  decltype(&vt::handle_message) handle_message;
  decltype(&vt::handle_keypress) handle_keypress;
} mod;

bool dynamic_load_module(Module& mod, const HINSTANCE instance,
                         const std::wstring& dll_path) {
#ifdef VWTILING_HOT_RELOAD
  std::wstringstream ss;
  ss << dll_path << L".tmp" << std::time(nullptr);
  const auto new_path = ss.str();
  CopyFileW(dll_path.c_str(), new_path.c_str(), FALSE);
  mod.dll = LoadLibraryW(new_path.c_str());
  if (!mod.dll) {
    MessageBoxA(nullptr, "Could not load vwtiling.dll", nullptr, MB_OK);
    return false;
  }
  mod.init = (decltype(&vt::init))GetProcAddress(mod.dll, "init");
  if (!mod.init) {
    MessageBoxA(nullptr, "Could not load vwtiling.dll:init", nullptr, MB_OK);
    return false;
  }
  mod.destroy = (decltype(&vt::destroy))GetProcAddress(mod.dll, "destroy");
  if (!mod.destroy) {
    MessageBoxA(nullptr, "Could not load vwtiling.dll:destroy", nullptr, MB_OK);
    return false;
  }
  mod.handle_message =
    (decltype(&vt::handle_message))GetProcAddress(mod.dll, "handle_message");
  if (!mod.handle_message) {
    MessageBoxA(nullptr, "Could not load vwtiling.dll:handle_message", nullptr, MB_OK);
    return false;
  }
#else
  mod.init = vt::init;
  mod.destroy = vt::destroy;
  mod.handle_message = vt::handle_message;
  mod.handle_keypress = vt::handle_keypress;
#endif

  mod.init(instance, mod.state);

  return true;
}

#ifdef VWTILING_HOT_RELOAD

bool dynamic_unload_module(Module& mod) {
  mod.destroy();
  const auto result = FreeLibrary(mod.dll);
  mod.dll = nullptr;
  return result;
}

FILETIME get_last_modified(const std::wstring& path) {
  const auto f = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr,
                             OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
  if (f == INVALID_HANDLE_VALUE || GetLastError() == ERROR_FILE_NOT_FOUND) {
    MessageBoxA(nullptr, "Could not find vwtiling.dll", nullptr, MB_OK);
  }
  FILETIME result;
  if (!GetFileTime(f, NULL, NULL, &result)) {
    MessageBoxA(nullptr,
                "Could not access vwtiling.dll's last write time for live reloading",
                nullptr, MB_OK);
  }
  CloseHandle(f);
  return result;
}

LRESULT delegate_message(const HWND hwnd, const UINT msg, const WPARAM wparam,
                         const LPARAM lparam) {
  if (mod.dll) {
    return mod.handle_message(hwnd, msg, wparam, lparam);
  }
  return DefWindowProc(hwnd, msg, wparam, lparam);
}

#endif

int WINAPI WinMain(const HINSTANCE instance, HINSTANCE /*prev*/, LPSTR /*args*/,
                   const int) {
  // Create invisible parent window for receiving messages.
  WNDCLASS wc = {0};
  // The classname MUST be the same as the filename since VirtuaWin uses this for locating
  // the window.
  wc.lpszClassName = module_name_exe;
  wc.hInstance = instance;
#ifdef VWTILING_HOT_RELOAD
  wc.lpfnWndProc = delegate_message;
#else
  wc.lpfnWndProc = vt::handle_message;
#endif
  wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
  RegisterClass(&wc);
  mod.state.messaging_hwnd = CreateWindowA(wc.lpszClassName, module_name, NULL, 0, 0, 0,
                                           0, nullptr, nullptr, instance, nullptr);

  std::wstring dll_path;
#ifdef VWTILING_HOT_RELOAD
  {
    wchar_t dll_path_buffer[MAX_PATH];
    GetModuleFileNameW(nullptr, dll_path_buffer, MAX_PATH);
    wchar_t* basename_start = dll_path_buffer;
    for (auto* scan = dll_path_buffer; *scan; ++scan) {
      if (*scan == '\\') {
        basename_start = scan + 1;
      }
    }
    wchar_t dll_basename[] = L"vwtiling.dll";
    for (int i = 0; i < sizeof(dll_basename) / sizeof(dll_basename[0]); ++i) {
      basename_start[i] = dll_basename[i];
    }
    dll_path = dll_path_buffer;
  }
  mod.load_time = get_last_modified(dll_path);
#endif
  if (!dynamic_load_module(mod, instance, dll_path)) {
    return 1;
  }

  MSG msg;
  BOOL ok;
  while ((ok = GetMessage(&msg, nullptr, 0, 0)) != 0) {
    if (ok) {
      mod.handle_keypress(msg);
      TranslateMessage(&msg);
      DispatchMessage(&msg);

#ifdef VWTILING_HOT_RELOAD
      auto last_write_time = get_last_modified(dll_path);
      if (CompareFileTime(&last_write_time, &mod.load_time) == 1) {
        mod.load_time = last_write_time;
        if (!dynamic_unload_module(mod)) {
          MessageBoxA(nullptr, "Could not unload vwtiling.dll", nullptr, MB_OK);
          return 2;
        }
        if (!dynamic_load_module(mod, instance, dll_path)) {
          return 1;
        }
      }
#endif
    } else {
      OutputDebugStringA("GetMessage failed.");
    }
  }

  return 0;
}
