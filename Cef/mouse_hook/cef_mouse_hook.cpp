#include "control/cef_control_base.h"
#include "cef_mouse_hook.h"
#include <mutex>

namespace {
  volatile HHOOK g_Hook = NULL;

  std::mutex g_Mutex{};
}

void CefMouseHook::Init()
{
  if (!g_Hook) {
    CefPostTask(TID_UI, MakeCefTask([&] {
      if (!g_Hook) {
        g_Hook = ::SetWindowsHookEx(WH_MOUSE, CefMouseHook::MouseProc, 0, ::GetCurrentThreadId());
      }
    }));
  }
}

void CefMouseHook::AddCefControlBase(CefControlBase* control_base)
{
  std::unique_lock<std::mutex> Lock{ g_Mutex };
  cef_control_base_list_.push_back(control_base);
}

void CefMouseHook::RemoveCefControlBase(CefControlBase* control_base)
{
  std::unique_lock<std::mutex> Lock{ g_Mutex };
  auto it_find = std::find(cef_control_base_list_.begin(), cef_control_base_list_.end(), control_base);
  if (it_find != cef_control_base_list_.end()) {
    cef_control_base_list_.erase(it_find);
  }
}

CefMouseHook::CefMouseHook()
{

}

CefMouseHook::~CefMouseHook()
{
  if (g_Hook) {
    ::UnhookWindowsHookEx(g_Hook);
  }
}

LRESULT CALLBACK CefMouseHook::MouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
  MOUSEHOOKSTRUCT* pMsg = (MOUSEHOOKSTRUCT*)lParam;
  HWND hWnd = ::GetParent(pMsg->hwnd);
  if (nCode == HC_ACTION) {
    switch (wParam)
    {
      case WM_LBUTTONDOWN:
      case WM_LBUTTONUP:
      case WM_MOUSEMOVE:
      {
        HWND parent_wnd = ::GetParent(pMsg->hwnd);
        HWND pparent_wnd = ::GetParent(parent_wnd);
        std::unique_lock<std::mutex> Lock{ g_Mutex };
        auto mouse_hook = CefMouseHook::Instance();
        auto it_find = std::find_if(
          mouse_hook->cef_control_base_list_.begin(),
          mouse_hook->cef_control_base_list_.end(),
          [&](auto const& cef_control_base) {
          if (cef_control_base) {
            HWND host_wnd = cef_control_base->GetCefHandle();
            return host_wnd == parent_wnd || host_wnd == pparent_wnd;
          }
          return false;
        });

        if (it_find != mouse_hook->cef_control_base_list_.end()) {
          (*it_find)->OnDragEvent(wParam, pMsg->hwnd, pMsg->pt);
        }

      }
      break;
      default:
        break;
    }

  }

  return ::CallNextHookEx(g_Hook, nCode, wParam, lParam);
}