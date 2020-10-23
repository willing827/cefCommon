#pragma once
#include "include/cef_client.h"
#include "include/cef_browser.h"
#include "app/cef_js_bridge.h"

class BrowserHandler :
  public CefClient,
  public CefLifeSpanHandler,
  public CefRenderHandler,
  public CefContextMenuHandler,
  public CefDisplayHandler,
  public CefJSDialogHandler,
  public CefLoadHandler,
  public CefDownloadHandler,
  public CefDialogHandler
{
public:
  BrowserHandler();


  class HandlerDelegate
  {
  public:
    // UI线程
    virtual void OnPaint(CefRefPtr<CefBrowser> browser,
      CefRenderHandler::PaintElementType type,
      const CefRenderHandler::RectList& dirtyRects,
      const std::string* buffer,
      int width,
      int height) = 0;

    // UI线程
    virtual void OnPopupShow(CefRefPtr<CefBrowser> browser, bool show) = 0;

    // UI线程
    virtual void OnPopupSize(CefRefPtr<CefBrowser> browser, const CefRect& rect) = 0;


    virtual bool OnExecuteCppFunc(
      const CefString& function_name,
      const CefString& params,
      CefRefPtr<CefBrowser> browser) = 0;

    //非ui线程
    virtual bool OnBeforePopup(CefRefPtr<CefBrowser> browser,
      CefRefPtr<CefFrame> frame,
      const CefString& target_url,
      const CefString& target_frame_name,
      CefLifeSpanHandler::WindowOpenDisposition target_disposition,
      bool user_gesture,
      const CefPopupFeatures& popupFeatures,
      CefWindowInfo& windowInfo,
      CefRefPtr<CefClient>& client,
      CefBrowserSettings& settings,
      bool* no_javascript_access) = 0;

    //ui线程 浏览器对象创建
    virtual bool OnAfterCreated(CefRefPtr<CefBrowser> browser) = 0;

    //ui线程 用与浏览器对象创建完毕后调整上层窗口大小
    virtual void UpdateWindowPos() = 0;

    //ui线程 浏览器将要关闭
    virtual void OnBeforeClose(CefRefPtr<CefBrowser> browser) = 0;

    //非ui线程
    virtual void OnBeforeContextMenu(
      CefRefPtr<CefBrowser> browser,
      CefRefPtr<CefFrame> frame,
      CefRefPtr<CefContextMenuParams> params,
      CefRefPtr<CefMenuModel> model) = 0;

    //非ui线程
    virtual bool OnContextMenuCommand(CefRefPtr<CefBrowser> browser,
      CefRefPtr<CefFrame> frame,
      CefRefPtr<CefContextMenuParams> params,
      int command_id,
      CefContextMenuHandler::EventFlags event_flags) = 0;

    virtual bool OnTooltip(CefRefPtr<CefBrowser> browser, CefString& text) = 0;

    virtual void OnAddressChange(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, const CefString& url) = 0;

    virtual void OnTitleChange(CefRefPtr<CefBrowser> browser, const CefString& title) = 0;

    virtual void OnLoadingStateChange(CefRefPtr<CefBrowser> browser, bool isLoading, bool canGoBack, bool canGoForward) = 0;

    virtual void OnLoadStart(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame) = 0;

    virtual void OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int httpStatusCode) = 0;

    virtual void OnLoadError(CefRefPtr<CefBrowser> browser,
      CefRefPtr<CefFrame> frame,
      CefLoadHandler::ErrorCode errorCode,
      const CefString& errorText,
      const CefString& failedUrl) = 0;

    // 文件下载相关
    //非ui线程
    virtual void OnBeforeDownload(
      CefRefPtr<CefBrowser> browser,
      CefRefPtr<CefDownloadItem> download_item,
      const CefString& suggested_name,
      CefRefPtr<CefBeforeDownloadCallback> callback) = 0;

    //非ui线程
    virtual void OnDownloadUpdated(
      CefRefPtr<CefBrowser> browser,
      CefRefPtr<CefDownloadItem> download_item,
      CefRefPtr<CefDownloadItemCallback> callback) = 0;

    //非ui线程 打开文件 Dialog
    virtual bool OnFileDialog(
      CefRefPtr<CefBrowser> browser,
      CefDialogHandler::FileDialogMode mode,
      const CefString& title,
      const CefString& default_file_path,
      const std::vector<CefString>& accept_filters,
      int selected_accept_filter,
      CefRefPtr<CefFileDialogCallback> callback) = 0;
  };

public:
  // 设置Cef浏览器寄宿窗
  void SetHostWindow(HWND hwnd) { host_wnd_ = hwnd; }

  // 获取CefCef浏览器寄宿窗
  HWND GetHostWindow() const { return host_wnd_; }

  // 设置Cef渲染内容的大小
  void SetViewRect(RECT rc);

  // 设置cef渲染 '区' 可见状态
  void SetIsVisible(bool is_visible);

  void SetHandlerDelegate(HandlerDelegate* handler) { handle_delegate_ = handler; }

  // 当前Web页面中获取焦点的元素是否可编辑
  bool IsCurFieldEditable() const { return is_focus_oneditable_field_; }

  CefRefPtr<CefBrowser> GetBrowser() const{ return browser_; }

  CefRefPtr<CefBrowserHost> GetBrowserHost();

  // 添加一个任务到队列中，当Browser对象创建成功后，会依次触发任务
  void AddAfterCreateTask(const std::function<void()>& cb) { task_list_after_created_.push_back(cb); }

  void CloseBrowser();
protected:

  // CefClient methods. Important to return |this| for the handler callbacks.
  virtual CefRefPtr<CefContextMenuHandler> GetContextMenuHandler() OVERRIDE { return this; }
  virtual CefRefPtr<CefDisplayHandler> GetDisplayHandler() OVERRIDE { return this; }
  virtual CefRefPtr<CefJSDialogHandler> GetJSDialogHandler() { return this; }
  virtual CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() OVERRIDE { return this; }
  virtual CefRefPtr<CefRenderHandler> GetRenderHandler() OVERRIDE { return this; }
  virtual CefRefPtr<CefLoadHandler> GetLoadHandler() OVERRIDE { return this; }
  virtual CefRefPtr<CefDownloadHandler> GetDownloadHandler() OVERRIDE { return this; }
  virtual CefRefPtr<CefDialogHandler> GetDialogHandler() OVERRIDE { return this; }

  virtual bool OnProcessMessageReceived(
    CefRefPtr<CefBrowser> browser,
    CefProcessId source_process,
    CefRefPtr<CefProcessMessage> message) OVERRIDE;


  // CefLifeSpanHandler methods
  virtual bool OnBeforePopup(CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    const CefString& target_url,
    const CefString& target_frame_name,
    WindowOpenDisposition target_disposition,
    bool user_gesture,
    const CefPopupFeatures& popupFeatures,
    CefWindowInfo& windowInfo,
    CefRefPtr<CefClient>& client,
    CefBrowserSettings& settings,
    bool* no_javascript_access)OVERRIDE;

  virtual void OnAfterCreated(CefRefPtr<CefBrowser> browser) OVERRIDE;

  virtual bool DoClose(CefRefPtr<CefBrowser> browser) OVERRIDE;

  virtual void OnBeforeClose(CefRefPtr<CefBrowser> browser) OVERRIDE;

  // CefRenderHandler methods
  virtual bool GetRootScreenRect(CefRefPtr<CefBrowser> browser, CefRect& rect) OVERRIDE;

  virtual void GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect) OVERRIDE;

  virtual bool GetScreenPoint(CefRefPtr<CefBrowser> browser, int viewX, int viewY, int& screenX, int& screenY) OVERRIDE;

  virtual void OnPopupShow(CefRefPtr<CefBrowser> browser, bool show) OVERRIDE;

  virtual void OnPopupSize(CefRefPtr<CefBrowser> browser, const CefRect& rect) OVERRIDE;

  virtual void OnPaint(CefRefPtr<CefBrowser> browser, PaintElementType type, const RectList& dirtyRects, const void* buffer, int width, int height) OVERRIDE;

  virtual void OnCursorChange(CefRefPtr<CefBrowser> browser, CefCursorHandle cursor, CursorType type, const CefCursorInfo& custom_cursor_info) OVERRIDE;

  // CefContextMenuHandler methods
  virtual void OnBeforeContextMenu(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    CefRefPtr<CefContextMenuParams> params,
    CefRefPtr<CefMenuModel> model) OVERRIDE;

  virtual bool RunContextMenu(CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    CefRefPtr<CefContextMenuParams> params,
    CefRefPtr<CefMenuModel> model,
    CefRefPtr<CefRunContextMenuCallback> callback)  OVERRIDE;

  virtual bool OnContextMenuCommand(CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    CefRefPtr<CefContextMenuParams> params,
    int command_id,
    EventFlags event_flags) OVERRIDE;

  virtual void OnContextMenuDismissed(CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame) OVERRIDE;

  // CefDisplayHandler methods
  virtual bool OnTooltip(CefRefPtr<CefBrowser> browser, CefString& text)OVERRIDE;

  virtual void OnAddressChange(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, const CefString& url) OVERRIDE;

  virtual void OnTitleChange(CefRefPtr<CefBrowser> browser, const CefString& title) OVERRIDE;

  virtual bool OnConsoleMessage(
    CefRefPtr<CefBrowser> browser,
    cef_log_severity_t level,
    const CefString& message,
    const CefString& source,
    int line) OVERRIDE;

  // CefLoadHandler methods
  virtual void OnLoadingStateChange(
    CefRefPtr<CefBrowser> browser,
    bool isLoading,
    bool canGoBack,
    bool canGoForward) OVERRIDE;

  virtual void OnLoadStart(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    TransitionType transition_type) OVERRIDE;

  virtual void OnLoadEnd(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    int httpStatusCode) OVERRIDE;

  virtual void OnLoadError(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    ErrorCode errorCode,
    const CefString& errorText,
    const CefString& failedUrl) OVERRIDE;

  // CefJSDialogHandler methods

  virtual bool OnJSDialog(CefRefPtr<CefBrowser> browser,
    const CefString& origin_url,
    JSDialogType dialog_type,
    const CefString& message_text,
    const CefString& default_prompt_text,
    CefRefPtr<CefJSDialogCallback> callback,
    bool& suppress_message) OVERRIDE;

  // CefDownloadHandler methods

  virtual void OnBeforeDownload(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefDownloadItem> download_item,
    const CefString& suggested_name,
    CefRefPtr<CefBeforeDownloadCallback> callback) OVERRIDE;

  virtual void OnDownloadUpdated(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefDownloadItem> download_item,
    CefRefPtr<CefDownloadItemCallback> callback) OVERRIDE;

  // CefDialogHandler methods
  virtual bool OnFileDialog(CefRefPtr<CefBrowser> browser,
    FileDialogMode mode,
    const CefString& title,
    const CefString& default_file_path,
    const std::vector<CefString>& accept_filters,
    int selected_accept_filter,
    CefRefPtr<CefFileDialogCallback> callback) OVERRIDE;

protected:
  volatile bool is_closing_{ false };
  CefRefPtr<CefBrowser>	browser_{};
  HWND					host_wnd_{ nullptr };  //寄宿窗口,非CEF句柄窗，也是OSR模式的承载窗
  RECT					rect_cef_control_;
  bool is_visible_{ true };
  std::string				paint_buffer_{};
  HandlerDelegate* handle_delegate_{ nullptr };
  bool is_focus_oneditable_field_{false};
  std::vector<std::function<void()>>	task_list_after_created_;

  IMPLEMENT_REFCOUNTING(BrowserHandler);
};