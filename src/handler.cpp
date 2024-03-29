#include "handler.h"

#include "bar.h"
#include "movement.h"

#include "VirtuaWin/messages.h"

#include <ShlObj.h>
#include <Shlwapi.h>
#include <ini.h>

#include <stdio.h>

static State* state = nullptr;

static void unset_handler() {
  // wParam == 2: Remove self as desktop change handler.
  SendMessage(state->vw_handle, VW_ICHANGEDESK, 2, 0);
}

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

static void update_desktop_set() {
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
        MessageBox(hwnd, TEXT("Failed to get handle to VirtuaWin."), TEXT("Module Error"),
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
                     TEXT("VirtuaWin failed to send the UserApp path to VirtaWin Bar."),
                     TEXT("Module Error"), MB_ICONWARNING);
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
        MultiByteToWideChar(CP_ACP, 0, (char*)cds->lpData, -1, state->user_app_path,
                            MAX_PATH);
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
      _snwprintf(message, 64 + MAX_PATH,
                 L"The configuration file for virtuawin-bar is located at %sbar.ini",
                 state->user_app_path);
      MessageBox(hwnd, message, TEXT("virtuawin-bar"), MB_ICONINFORMATION);
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

  state->config = Config(); // Reset config.
  // Try to load config from user's config file.
  TCHAR config_path[MAX_PATH];
  if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, config_path)) &&
      PathAppend(config_path, TEXT("vwbar.ini"))) {
    // MessageBox(NULL, config_path, TEXT("vwbar"), MB_ICONINFORMATION);
    FILE* f = _wfopen(config_path, L"r");
    if (f) {
      ini_parse_file(f, config_entry_handler, &state->config);
      fclose(f);
    }
  }

  init_bar(*state, instance, state->messaging_hwnd);

  if (state->config.window_switch) {
    if (!RegisterHotKey(nullptr, Command::SwitchLeft, MOD_ALT, 0x4A)) {
      OutputDebugStringA("Hotkey 'Alt+J' could not be registered.\n");
    }
    if (!RegisterHotKey(nullptr, Command::SwitchRight, MOD_ALT, VK_OEM_1)) {
      OutputDebugStringA("Hotkey 'Alt+;' could not be registered.\n");
    }
    if (!RegisterHotKey(nullptr, Command::SwitchUp, MOD_ALT, 0x4C)) {
      OutputDebugStringA("Hotkey 'Alt+L' could not be registered.\n");
    }
    if (!RegisterHotKey(nullptr, Command::SwitchDown, MOD_ALT, 0x4B)) {
      OutputDebugStringA("Hotkey 'Alt+K' could not be registered.\n");
    }
  }
}

// TODO: Function name is misleading here since we also handle mouse clicks here...
void handle_keypress(const MSG& msg) {
  if (msg.message == WM_HOTKEY) {
    switch (msg.wParam) {
      case Command::SwitchLeft:
      case Command::SwitchRight:
      case Command::SwitchUp:
      case Command::SwitchDown:
        switch_window((Command::Code)msg.wParam);
        break;

      default:
        OutputDebugStringA("Unrecognized key command.\n");
    }
  } else if (msg.message == WM_RBUTTONDOWN) {
    if (state->bar_hwnd) {
      // Toggle topmost.
      state->bar_topmost = !state->bar_topmost;
      if (state->bar_topmost) {
        SetWindowPos(state->bar_hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
      } else {
        SetWindowPos(state->bar_hwnd, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
      }
    }
  }
}

void destroy() {
  if (state->config.window_switch) {
    UnregisterHotKey(nullptr, Command::SwitchLeft);
    UnregisterHotKey(nullptr, Command::SwitchRight);
    UnregisterHotKey(nullptr, Command::SwitchUp);
    UnregisterHotKey(nullptr, Command::SwitchDown);
  }
  destroy_bar(*state);
  state = nullptr;
}

State& get_state() { return *state; }

void get_screen_resolution(LONG* out_width, LONG* out_height) {
  auto* const desktop = GetDesktopWindow();
  RECT rect;
  GetWindowRect(desktop, &rect);
  if (out_width)
    *out_width = rect.right;
  if (out_height)
    *out_height = rect.bottom;
}
