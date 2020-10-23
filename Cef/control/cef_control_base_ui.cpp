#include "cef_control_base.h"
#include "include/cef_browser.h"
#include "include/cef_frame.h"
#include "handler/browser_handler.h"
#include "manager/cef_manager.h"
#include "app/cef_js_bridge.h"
#include "mouse_hook/cef_mouse_hook.h"


CefControlBase::CefControlBase(void)
{

}

CefControlBase::~CefControlBase(void)
{
  CloseBrowser();

  DettachDevTools();

  CefMouseHook::Instance()->RemoveCefControlBase(this);
}

void CefControlBase::LoadURL(const CefString& url)
{
  if (browser_handler_.get() && browser_handler_->GetBrowser().get()) {
    CefRefPtr<CefFrame> frame = browser_handler_->GetBrowser()->GetMainFrame();
    if (!frame)
      return;

    frame->LoadURL(url);
  } else if (browser_handler_.get()) {
    browser_handler_->AddAfterCreateTask([=]() {LoadURL(url); });
  }
}

void CefControlBase::GoBack()
{
  if (browser_handler_.get() && browser_handler_->GetBrowser().get()) {
    return browser_handler_->GetBrowser()->GoBack();
  }
}

void CefControlBase::GoForward()
{
  if (browser_handler_.get() && browser_handler_->GetBrowser().get()) {
    return browser_handler_->GetBrowser()->GoForward();
  }
}

bool CefControlBase::CanGoBack()
{
  if (browser_handler_.get() && browser_handler_->GetBrowser().get()) {
    return browser_handler_->GetBrowser()->CanGoBack();
  }
  return false;
}

bool CefControlBase::CanGoForward()
{
  if (browser_handler_.get() && browser_handler_->GetBrowser().get()) {
    return browser_handler_->GetBrowser()->CanGoForward();
  }
  return false;
}

void CefControlBase::Refresh()
{
  if (browser_handler_.get() && browser_handler_->GetBrowser().get()) {
    return browser_handler_->GetBrowser()->Reload();
  }
}

void CefControlBase::StopLoad()
{
  if (browser_handler_.get() && browser_handler_->GetBrowser().get()) {
    return browser_handler_->GetBrowser()->StopLoad();
  }
}

bool CefControlBase::IsLoading()
{
  if (browser_handler_.get() && browser_handler_->GetBrowser().get()) {
    return browser_handler_->GetBrowser()->IsLoading();
  }
  return false;
}

void CefControlBase::StartDownload(const CefString& url)
{
  if (browser_handler_.get() && browser_handler_->GetBrowser().get()) {
    browser_handler_->GetBrowser()->GetHost()->StartDownload(url);
  }
}

void CefControlBase::SetZoomLevel(float zoom_level)
{
  if (browser_handler_.get() && browser_handler_->GetBrowser().get()) {
    browser_handler_->GetBrowser()->GetHost()->SetZoomLevel(zoom_level);
  }
}

HWND CefControlBase::GetCefHandle() const
{
  if (browser_handler_.get() && browser_handler_->GetBrowserHost().get())
    return browser_handler_->GetBrowserHost()->GetWindowHandle();

  return NULL;
}

HWND CefControlBase::GetHostWindow() const
{
  return browser_handler_.get() ? browser_handler_->GetHostWindow() : NULL;
}

CefString CefControlBase::GetURL() const
{
  if (browser_handler_.get() && browser_handler_->GetBrowser().get()) {
    return browser_handler_->GetBrowser()->GetMainFrame()->GetURL();
  }

  return {};
}

std::string CefControlBase::GetUTF8URL() const
{
  return helper_lib::ToString(GetURL().c_str(), CP_UTF8);
}

CefString CefControlBase::GetMainURL(const CefString& url)
{
  std::string temp = url.ToString();
  int end_pos = temp.find("#") == std::string::npos ? temp.length() : temp.find("#");
  temp = temp.substr(0, end_pos);
  return CefString(temp.c_str());
}

void CefControlBase::RegisterCppFunc(const std::wstring& function_name, CppFunction func, int browser_id /*= -1*/)
{
  if (js_bridge_) {
    js_bridge_->RegisterCppFunc(helper_lib::ToString(function_name, CP_UTF8).c_str(), func, browser_id);
  }
}

void CefControlBase::UnRegisterCppFunc(const std::wstring& function_name, int browser_id /*= -1*/)
{
  if (js_bridge_) {
    js_bridge_->UnRegisterCppFunc(helper_lib::ToString(function_name, CP_UTF8).c_str(), browser_id);
  }
}

bool CefControlBase::CallJSFunction(const std::wstring& js_function_name, const std::wstring& params, int64_t frame_id /*= -1*/)
{
  if (browser_handler_.get() && browser_handler_->GetBrowser().get() && js_bridge_) {
    auto browser = browser_handler_->GetBrowser();
    auto frame = browser->GetFrame(frame_id);
    frame = frame.get() ? frame : browser->GetMainFrame();
    return js_bridge_->CallJSFunction(
      helper_lib::ToString(js_function_name.c_str(), CP_UTF8).c_str(),
      helper_lib::ToString(params.c_str(), CP_UTF8).c_str(),
      frame);
  }

  return false;
}

bool CefControlBase::CallJSFunction(const std::wstring& js_function_name, const std::wstring& params, CefString const&frame_name)
{
  if (browser_handler_.get() && browser_handler_->GetBrowser().get() && js_bridge_) {
    auto browser = browser_handler_->GetBrowser();
    auto frame = browser->GetFrame(frame_name);
    frame = frame.get() ? frame : browser->GetMainFrame();
    return js_bridge_->CallJSFunction(
      helper_lib::ToString(js_function_name.c_str(), CP_UTF8).c_str(),
      helper_lib::ToString(params.c_str(), CP_UTF8).c_str(),
      frame);
  }

  return false;
}

void CefControlBase::RepairBrowser()
{
  ReCreateBrowser();
}

namespace
{
  class DevToolsHandler
    :public CefClient
    , public CefLifeSpanHandler
  {

  public:
    OnBeforeCloseEvent before_close_event_;
  protected:
    virtual CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() { return this; }

    virtual void OnBeforeClose(CefRefPtr<CefBrowser> browser)
    {
      CefManager::PostTaskUIThread([=] {
        before_close_event_(CefRefPtr<CefBrowser>{browser});
      });
    }

    IMPLEMENT_REFCOUNTING(DevToolsHandler);
  };
}

bool CefControlBase::AttachDevTools()
{
  if (!browser_handler_.get()) return false;

  if (devtool_attached_) return true;

  auto browser = browser_handler_->GetBrowser();
  if (!browser.get()) {
    browser_handler_->AddAfterCreateTask([this]() {CefManager::PostTaskUIThread([this]() {AttachDevTools(); }); });
  } else {
    CefWindowInfo windowInfo;
    windowInfo.SetAsPopup(NULL, L"cef_devtools");
    CefBrowserSettings settings;
    windowInfo.width = 900;
    windowInfo.height = 700;
    auto dev_tools_handler = new DevToolsHandler;
    dev_tools_handler->before_close_event_ = [=](auto const&) {
      if (devtool_attached_) {
        devtool_attached_ = false;
      }
      if (cb_devtool_visible_change_ != nullptr) {
        cb_devtool_visible_change_(devtool_attached_);
      }
      return true;
    };


    browser->GetHost()->ShowDevTools(windowInfo, dev_tools_handler, settings, CefPoint());
    devtool_attached_ = true;
    if (cb_devtool_visible_change_ != nullptr)
      cb_devtool_visible_change_(devtool_attached_);
  }
  return true;
}

void CefControlBase::DettachDevTools()
{
  if (!browser_handler_.get())return;
  if (!devtool_attached_) return;

  auto browser = browser_handler_->GetBrowser();
  if (browser.get()) {
    browser->GetHost()->CloseDevTools();
    devtool_attached_ = false;
    if (cb_devtool_visible_change_ != nullptr)
      cb_devtool_visible_change_(devtool_attached_);
  }
}