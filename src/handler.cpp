#include "handler.h"

#include "bar.h"

#include "VirtuaWin/messages.h"

#include <cstdio>
#include <tchar.h>

namespace {

constexpr auto module_name = _T("virtuawin-bar");
constexpr auto module_name_exe = _T("virtuawin-bar.exe");

::vwbar::Module mod;

} // namespace

namespace vwbar {

__inline BOOL CALLBACK enum_windows_proc(const HWND hwnd, const LPARAM desk_count) {
  auto& state = mod.state;
  if (state.vw_handle) {
    const auto style = GetWindowLong(hwnd, GWL_STYLE);
    if (!(style & WS_CHILD)) {
      const auto flag = SendMessage(state.vw_handle, VW_WINGETINFO, (WPARAM)hwnd, 0);
      if (flag && (flag & vwWINFLAGS_MANAGED)) {
        const auto desk = vwWindowGetInfoDesk(flag);
        if (desk != state.active_desktop && desk > desk_count) {
          return TRUE; // On a hidden desktop, so skip.
        }
        state.desktops |= 1 << desk;
      }
    }
  }
  return TRUE;
}

void update_desktop_set() {
  mod.state.desktops = 0; // Clear.
  const auto desk_count = SendMessage(mod.state.vw_handle, VW_DESKX, 0, 0) *
                          SendMessage(mod.state.vw_handle, VW_DESKY, 0, 0);
  EnumWindows(enum_windows_proc, desk_count);
}

/// Handle our window's messages.
LRESULT wnd_proc(const HWND hwnd, const UINT msg, const WPARAM wParam,
                 const LPARAM lParam) {
  switch (msg) {
    case MOD_INIT: {
      // This must be taken care of in order to get the handle to VirtuaWin.
      // The handle to VirtuaWin comes in the wParam.
      mod.state.vw_handle = (HWND)wParam;
      if (!mod.state.vw_handle) {
        MessageBox(hwnd, _T("Failed to get handle to VirtuaWin."), _T("Module Error"),
                   MB_ICONWARNING);
        exit(1);
      }
      // wParam == 1: Set self as desktop change handler.
      SendMessage(mod.state.vw_handle, VW_ICHANGEDESK, 1, 0);
      // Unmanage self.
      SendMessage(mod.state.vw_handle, VW_WINMANAGE, (WPARAM)hwnd, 0);
      if (!mod.state.initialized) {
        SendMessage(mod.state.vw_handle, VW_USERAPPPATH, (WPARAM)hwnd, 0);
        // TODO: Is a delay necessary here?
        if ((mod.state.initialized & 2) == 0) {
          MessageBox(hwnd,
                     _T("VirtuaWin failed to send the UserApp path to VirtaWin Bar."),
                     _T("Module Error"), MB_ICONWARNING);
          exit(1);
        }
      }
      // Initialize desktop set.
      update_desktop_set();
      mod.state.active_desktop =
        (uint8_t)SendMessage(mod.state.vw_handle, VW_CURDESK, 0, 0);
      mod.state.desktops |= 1 << mod.state.active_desktop;
      resize_client(mod.state);
      InvalidateRect(mod.state.bar_hwnd, nullptr, true);
    } break;

    case WM_COPYDATA: {
      auto* const cds = (COPYDATASTRUCT*)lParam;
      if ((int)cds->dwData == (0 - VW_USERAPPPATH) && (mod.state.initialized & 2) == 0) {
        if (cds->cbData < 2 || !cds->lpData) {
          return FALSE;
        }
        mod.state.initialized |= 2;
#ifdef _UNICODE
        MultiByteToWideChar(CP_ACP, 0, (char*)cds->lpData, -1, user_app_path, MAX_PATH);
#else
        strncpy(mod.state.user_app_path, (char*)cds->lpData, MAX_PATH);
#endif
        mod.state.user_app_path[MAX_PATH - 1] = '\0';
      }
      return TRUE;
    } break;

    case MOD_CFGCHANGE:
      break;

    case MOD_CHANGEDESK: {
      // VirtuaWin might sent two MOD_CHANGEDESK messages in the case of a wrap-around,
      // but we don't care about that.  So we only do something for the first one.
      if (mod.state.active_desktop != lParam) {
        mod.state.active_desktop = decltype(mod.state.active_desktop)(lParam);
        // wParam == 3: Execute the desktop change.
        SendMessage(mod.state.vw_handle, VW_ICHANGEDESK, 3, 0);
        update_desktop_set();
        mod.state.desktops |= 1 << mod.state.active_desktop;
        resize_client(mod.state);
        InvalidateRect(mod.state.bar_hwnd, nullptr, true);
        // UpdateWindow doesn't seem to be necessary here.
      }
    } break;

    case MOD_SETUP: {
      // Displays when user presses Setup > Modules > Configure.
      TCHAR message[64 + MAX_PATH];
      snprintf(message, 64 + MAX_PATH,
               "The configuration file for virtuawin-bar is located at %sbar.json",
               mod.state.user_app_path);
      MessageBox(hwnd, message, module_name, MB_ICONINFORMATION);
    } break;

    case MOD_QUIT: // This must be handled, otherwise VirtuaWin can't shut down the
                   // module.
    case WM_CLOSE:
    case WM_DESTROY:
      // wParam == 2: Remove self as desktop change handler.
      SendMessage(mod.state.vw_handle, VW_ICHANGEDESK, 2, 0);
      PostQuitMessage(0);
      break;

    default:
      return DefWindowProc(hwnd, msg, wParam, lParam);
  }
  return 0;
}

void init(const HINSTANCE instance) {
  // Create invisible parent window for receiving messages.
  WNDCLASS wc = {0};
  // The classname MUST be the same as the filename since VirtuaWin uses this for locating
  // the window.
  wc.lpszClassName = module_name_exe;
  wc.hInstance = instance;
  wc.lpfnWndProc = wnd_proc;
  wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
  RegisterClass(&wc);
  auto* const hwnd = CreateWindowA(wc.lpszClassName, module_name, NULL, 0, 0, 0, 0,
                                   nullptr, nullptr, instance, nullptr);

  init_bar(mod.state, instance, hwnd);
}

State& get_state() { return mod.state; }

} // namespace vwbar
