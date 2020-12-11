#include "handler.h"

#include "bar.h"

#include "VirtuaWin/messages.h"

#include <cstdio>
#include <tchar.h>

namespace {

::vwtiling::State* state = nullptr;

void unset_handler() {
  // wParam == 2: Remove self as desktop change handler.
  SendMessage(state->vw_handle, VW_ICHANGEDESK, 2, 0);
}

} // namespace

namespace vwtiling {

__inline BOOL CALLBACK enum_windows_proc(const HWND hwnd, const LPARAM desk_count) {
  if (state->vw_handle) {
    const auto style = GetWindowLong(hwnd, GWL_STYLE);
    if (!(style & WS_CHILD)) {
      const auto flag = SendMessage(state->vw_handle, VW_WINGETINFO, (WPARAM)hwnd, 0);
      if (flag && (flag & vwWINFLAGS_MANAGED)) {
        const auto desk = vwWindowGetInfoDesk(flag);
        if (desk != state->active_desktop && desk > desk_count) {
          return TRUE; // On a hidden desktop, so skip.
        }
        state->desktops |= 1 << desk;
      }
    }
  }
  return TRUE;
}

void update_desktop_set() {
  state->desktops = 0; // Clear.
  const auto desk_count = SendMessage(state->vw_handle, VW_DESKX, 0, 0) *
                          SendMessage(state->vw_handle, VW_DESKY, 0, 0);
  EnumWindows(enum_windows_proc, desk_count);
}

/// Handle our window's messages.
LRESULT handle_message(const HWND hwnd, const UINT msg, const WPARAM wParam,
                       const LPARAM lParam) {
  switch (msg) {
    case MOD_INIT: {
      // This must be taken care of in order to get the handle to VirtuaWin.
      // The handle to VirtuaWin comes in the wParam.
      state->vw_handle = (HWND)wParam;
      if (!state->vw_handle) {
        MessageBox(hwnd, _T("Failed to get handle to VirtuaWin."), _T("Module Error"),
                   MB_ICONWARNING);
        exit(1);
      }
      // wParam == 1: Set self as desktop change handler.
      SendMessage(state->vw_handle, VW_ICHANGEDESK, 1, 0);
      // Unmanage self.
      SendMessage(state->vw_handle, VW_WINMANAGE, (WPARAM)hwnd, 0);
      if (!state->initialized) {
        SendMessage(state->vw_handle, VW_USERAPPPATH, (WPARAM)hwnd, 0);
        // TODO: Is a delay necessary here?
        if ((state->initialized & 2) == 0) {
          MessageBox(hwnd,
                     _T("VirtuaWin failed to send the UserApp path to VirtaWin Bar."),
                     _T("Module Error"), MB_ICONWARNING);
          exit(1);
        }
      }
      // Initialize desktop set.
      update_desktop_set();
      state->active_desktop = (uint8_t)SendMessage(state->vw_handle, VW_CURDESK, 0, 0);
      state->desktops |= 1 << state->active_desktop;
      resize_client(*state);
      InvalidateRect(state->bar_hwnd, nullptr, TRUE);
    } break;

    case WM_COPYDATA: {
      auto* const cds = (COPYDATASTRUCT*)lParam;
      if ((int)cds->dwData == (0 - VW_USERAPPPATH) && (state->initialized & 2) == 0) {
        if (cds->cbData < 2 || !cds->lpData) {
          return FALSE;
        }
        state->initialized |= 2;
#ifdef _UNICODE
        MultiByteToWideChar(CP_ACP, 0, (char*)cds->lpData, -1, user_app_path, MAX_PATH);
#else
        strncpy(state->user_app_path, (char*)cds->lpData, MAX_PATH);
#endif
        state->user_app_path[MAX_PATH - 1] = '\0';
      }
      return TRUE;
    } break;

    case MOD_CFGCHANGE:
      break;

    case MOD_CHANGEDESK: {
      // VirtuaWin might sent two MOD_CHANGEDESK messages in the case of a wrap-around,
      // but we don't care about that.  So we only do something for the first one.
      if (state->active_desktop != lParam) {
        state->active_desktop = decltype(state->active_desktop)(lParam);
        // wParam == 3: Execute the desktop change.
        SendMessage(state->vw_handle, VW_ICHANGEDESK, 3, 0);
        update_desktop_set();
        state->desktops |= 1 << state->active_desktop;
        resize_client(*state);
        InvalidateRect(state->bar_hwnd, nullptr, TRUE);
        // UpdateWindow doesn't seem to be necessary here.
      }
    } break;

    case MOD_SETUP: {
      // Displays when user presses Setup > Modules > Configure.
      TCHAR message[64 + MAX_PATH];
      snprintf(message, 64 + MAX_PATH,
               "The configuration file for virtuawin-bar is located at %sbar.json",
               state->user_app_path);
      MessageBox(hwnd, message, "virtuawin-bar", MB_ICONINFORMATION);
    } break;

    case MOD_QUIT: // This must be handled, otherwise VirtuaWin can't shut down the
                   // module.
    case WM_CLOSE:
    case WM_DESTROY:
      unset_handler();
      PostQuitMessage(0);
      break;

    default:
      return DefWindowProc(hwnd, msg, wParam, lParam);
  }
  return 0;
}

void init(const HINSTANCE instance, State& init_state) {
  state = &init_state;
  state->config = Config(); // Reload config.
  init_bar(*state, instance, state->messaging_hwnd);
}

void destroy() {
  destroy_bar(*state);
  state = nullptr;
}

State& get_state() { return *state; }

} // namespace vwtiling
