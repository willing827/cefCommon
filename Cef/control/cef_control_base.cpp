#include "cef_control_base.h"
#include "util/util.h"

void CefControlBase::CloseBrowser(bool reset_js_bridge /*= true*/)
{
  if (reset_js_bridge && js_bridge_) {
    js_bridge_.reset();
  }
  if (browser_handler_.get()) {
    browser_handler_->SetHostWindow(NULL);
    browser_handler_->SetHandlerDelegate(NULL);
    browser_handler_->CloseBrowser();
    browser_handler_ = nullptr;
  }
}

void CefControlBase::OnDragStart()
{
  cb_drag_start_();
}

void CefControlBase::OnDragEvent(UINT msg_id, HWND wnd, POINT pt_window)
{
    on_drag_event_(msg_id, wnd, pt_window);

  if (ShouldCaptureMouseWhenLButtonDown()) {
    if (msg_id == WM_LBUTTONDOWN) {
      ::SetCapture(wnd);
    } else if (msg_id == WM_LBUTTONUP) {
      if (::GetCapture() == wnd) {
        ::ReleaseCapture();
      }
    }
  }
  HWND host_wnd = GetHostWindow();
  if (host_wnd) {
    ::ScreenToClient(host_wnd, &pt_window);
    ::SendMessage(host_wnd, msg_id, -1, (LPARAM)MAKELONG(pt_window.x, pt_window.y));
  }
}

bool CefControlBase::OnExecuteCppFunc(const CefString& function_name, const CefString& params, CefRefPtr<CefBrowser> browser)
{
  if (js_bridge_) {
    return js_bridge_->ExecuteCppFunc(function_name, params, browser ? browser->GetIdentifier() : -1);
  }

  return false;
}

bool CefControlBase::OnBeforePopup(CefRefPtr<CefBrowser> browser,
  CefRefPtr<CefFrame> frame,
  const CefString& target_url,
  const CefString& target_frame_name,
  CefLifeSpanHandler::WindowOpenDisposition target_disposition,
  bool user_gesture,
  const CefPopupFeatures& popupFeatures,
  CefWindowInfo& windowInfo,
  CefRefPtr<CefClient>& client,
  CefBrowserSettings& settings,
  bool* no_javascript_access)
{
  if (cb_link_click_ && !target_url.empty())
    return cb_link_click_(target_url);

  return false;
}

bool CefControlBase::OnAfterCreated(CefRefPtr<CefBrowser> browser)
{
  if (js_bridge_) {
    js_bridge_->RegisterCppFunc(L"DragStart", [=](const std::string& params) {
      OnDragStart();
    }, browser ? browser->GetIdentifier() : -1);
  }

  cb_after_created_(browser);

  return false;
}

void CefControlBase::UpdateWindowPos()
{

}

void CefControlBase::OnBeforeClose(CefRefPtr<CefBrowser> browser)
{
  cb_before_close_(browser);
}

void CefControlBase::OnBeforeContextMenu(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefContextMenuParams> params, CefRefPtr<CefMenuModel> model)
{
  if (!cb_before_menu_ || !cb_before_menu_(params, model)) {
    //½ûÓÃÓÒ¼ü²Ëµ¥
    if ((params->GetTypeFlags() & (CM_TYPEFLAG_PAGE | CM_TYPEFLAG_FRAME)) != 0) {
      if (model->GetCount() > 0) {
        model->Clear();
      }
    }
  }
}

bool CefControlBase::OnContextMenuCommand(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefContextMenuParams> params, int command_id, CefContextMenuHandler::EventFlags event_flags)
{
  if (cb_menu_command_) return cb_menu_command_(params, command_id, event_flags);

  return false;
}

bool CefControlBase::OnTooltip(CefRefPtr<CefBrowser> browser, CefString& text)
{
    return false;
}

void CefControlBase::OnAddressChange(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, const CefString& url)
{
  if (cb_url_change_) cb_url_change_(url);

  if (frame->IsMain()) {
    auto old_url = url_;
    url_ = frame->GetURL();
    if (cb_main_url_change_ != nullptr && GetMainURL(old_url).compare(GetMainURL(url_)) != 0) {
      cb_main_url_change_(old_url, url_);
    }
  }
}

void CefControlBase::OnTitleChange(CefRefPtr<CefBrowser> browser, const CefString& title)
{
  if (cb_title_change_)cb_title_change_(title);
}

void CefControlBase::OnLoadingStateChange(CefRefPtr<CefBrowser> browser, bool isLoading, bool canGoBack, bool canGoForward)
{
  if (cb_loadstate_change_) cb_loadstate_change_(isLoading, canGoBack, canGoForward);
}

void CefControlBase::OnLoadStart(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame)
{
  cb_load_start_();
}

void CefControlBase::OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int httpStatusCode)
{
  cb_load_end_(httpStatusCode);
}

void CefControlBase::OnLoadError(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefLoadHandler::ErrorCode errorCode, const CefString& errorText, const CefString& failedUrl)
{
  cb_load_error_(errorCode, errorText, failedUrl);
}

void CefControlBase::OnBeforeDownload(CefRefPtr<CefBrowser> browser, CefRefPtr<CefDownloadItem> download_item, const CefString& suggested_name, CefRefPtr<CefBeforeDownloadCallback> callback)
{
  if (cb_before_download_)
    cb_before_download_(browser, download_item, suggested_name, callback);
}

void CefControlBase::OnDownloadUpdated(CefRefPtr<CefBrowser> browser, CefRefPtr<CefDownloadItem> download_item, CefRefPtr<CefDownloadItemCallback> callback)
{
  if (cb_download_updated_)
    cb_download_updated_(browser, download_item, callback);
}

bool CefControlBase::OnFileDialog(CefRefPtr<CefBrowser> browser, CefDialogHandler::FileDialogMode mode, const CefString& title, const CefString& default_file_path, const std::vector<CefString>& accept_filters, int selected_accept_filter, CefRefPtr<CefFileDialogCallback> callback)
{
  if (cb_file_dialog_)
    return cb_file_dialog_(mode, title, default_file_path, accept_filters, selected_accept_filter, callback);
  else
    return false;
}