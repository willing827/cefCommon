#pragma once
#include "cef_control_base.h"
#include "manager/cef_manager.h"
#include "mouse_hook/cef_mouse_hook.h"

class CefPopupWnd
  :public CefControlBase
  , public helper_lib::DragWindowMoveAdapterT<CefPopupWnd>
{
public:
  CefPopupWnd() = default;

  CefPopupWnd(HWND parent_wnd, RECT const& rect, int round_corner = 0)
  {
    CreateWnd(parent_wnd, rect, round_corner);
  }

  void CreateWnd(HWND parent_wnd, RECT const& rect, int round_corner = 0)
  {
    if (!browser_handler_.get()) {
      parent_wnd_ = parent_wnd;
      rect_ = rect;
      round_corner_ = round_corner;

      browser_handler_ = new BrowserHandler;
      browser_handler_->SetHandlerDelegate(this);

      CefMouseHook::Instance()->AddCefControlBase(this);

      js_bridge_.reset(new CefJSBridge);

      ReCreateBrowser();
    }
  }

  void SetVisible(bool is_visible)
  {
    if (!browser_handler_.get()) return;
    if (browser_handler_->GetBrowser().get()) {
      browser_handler_->SetIsVisible(is_visible);
      HWND hwnd = GetCefHandle();
      if (hwnd) {
        ::ShowWindow(hwnd, is_visible ? SW_SHOW : SW_HIDE);
      }
    } else {
      browser_handler_->AddAfterCreateTask([=]() { SetVisible(is_visible); });
    }
  }

  HWND GetHWND() const { return GetCefHandle(); }

  void EnableDrag() { cb_drag_start_ = [=] {SetEnterDragMode(false); return true; }; }
protected:
  void ReCreateBrowser() override
  {
    if (browser_handler_.get() && !browser_handler_->GetBrowser().get()){
      CefWindowInfo window_info;
      window_info.SetAsPopup(parent_wnd_, L"");
      window_info.x = rect_.left;
      window_info.y = rect_.top;
      window_info.width = rect_.right - rect_.left;
      window_info.height = rect_.bottom - rect_.top;
      window_info.style = WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;

      CefBrowserSettings browser_settings;
      CefBrowserHost::CreateBrowser(window_info, browser_handler_, url_.c_str(), browser_settings, NULL);
    }
  }

  virtual void OnDragEvent(UINT msg_id, HWND wnd, POINT pt_window) override
  {
    on_drag_event_(msg_id, wnd, pt_window);

    ::ScreenToClient(GetCefHandle(), &pt_window);
    HandleDragMessage(msg_id, -1, (LPARAM)MAKELONG(pt_window.x, pt_window.y));
  }

  virtual bool OnAfterCreated(CefRefPtr<CefBrowser> browser) override
  {
    if (round_corner_ > 0) {
      HWND wnd = GetHWND();
      if (wnd) {
        RECT rect{};
        ::GetClientRect(wnd, &rect);
        HRGN rgn = ::CreateRoundRectRgn(rect.left, rect.top, rect.right, rect.bottom, round_corner_, round_corner_);
        ::SetWindowRgn(wnd, rgn, TRUE);
        ::DeleteObject(rgn);
      }
    }
    return __super::OnAfterCreated(browser);
  }

protected:
  HWND parent_wnd_;
  RECT rect_;
  int round_corner_{ 0 };
};