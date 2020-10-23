#pragma once
#include "cef_control_base.h"
#include "manager/cef_manager.h"
#include "mouse_hook/cef_mouse_hook.h"

class CefChildWnd
  : public CefControlBase
{
public:

  CefChildWnd() = default;

  CefChildWnd(HWND parent_wnd, RECT const& rect, bool init_visible = true)
  {
    CreateWnd(parent_wnd, rect, init_visible);
  }

  void CreateWnd(HWND parent_wnd, RECT const& rect, bool init_visible = true)
  {
    if (!browser_handler_.get()) {
      parent_wnd_ = parent_wnd;
      rect_ = rect;
      init_visible_ = init_visible;

      //±ÜÃâsizechangeÊ±ÉÁË¸
      LONG style = GetWindowLong(parent_wnd_, GWL_STYLE);
      SetWindowLong(parent_wnd_, GWL_STYLE, style | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);

      browser_handler_ = new BrowserHandler;
      browser_handler_->SetHandlerDelegate(this);
      browser_handler_->SetHostWindow(parent_wnd_);

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

  void ReCreateBrowser() override
  {
    if (browser_handler_->GetBrowser() == nullptr) {
      CefWindowInfo window_info;
      window_info.SetAsChild(parent_wnd_, rect_);

      if (!init_visible_) window_info.style |= ~WS_VISIBLE;

      CefBrowserSettings browser_settings;
      CefBrowserHost::CreateBrowser(window_info, browser_handler_, url_.c_str(), browser_settings, NULL);
    }
  }
protected:
  HWND parent_wnd_;
  RECT rect_;

  bool init_visible_{ true };
};

