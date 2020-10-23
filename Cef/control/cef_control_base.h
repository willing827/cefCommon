#pragma once
#include "include/cef_base.h"
#include "cef_control_event.h"
#include "app/cef_js_bridge.h"
#include "handler/browser_handler.h"
#include "Helper/event_observer.h"

class CefControlBase :public BrowserHandler::HandlerDelegate
{
public:
  CefControlBase(void);

  virtual ~CefControlBase(void);

  helper_lib::Events<UINT /*msg_id*/, HWND /*wnd*/, POINT /*pt_window*/> on_drag_event_;
public:

  void LoadURL(const CefString& url);

  void GoBack();

  void GoForward();

  bool CanGoBack();

  bool CanGoForward();

  void Refresh();

  void StopLoad();

  bool IsLoading();

  void StartDownload(const CefString& url);

  //设置页面缩放比例
  void SetZoomLevel(float zoom_level);

  // 获取[CEF]浏览器窗口句柄
  HWND GetCefHandle() const;

  // 获取控件附属窗口句柄(非CefHostWnd)
  HWND GetHostWindow() const;

  //获取页面 URL
  CefString GetURL() const;

  //获取 UTF8 格式 URL
  std::string GetUTF8URL() const;

  //获取网址 # 号前的地址
  static CefString GetMainURL(const CefString& url);

  //注册一个 C++ 方法提供前端调用
  void RegisterCppFunc(const std::wstring& function_name, CppFunction func, int browser_id = -1);

  //反注册一个 C++ 方法
  void UnRegisterCppFunc(const std::wstring& function_name, int browser_id = -1);

  //调用一个前端已经注册好的方法
  bool CallJSFunction(const std::wstring& js_function_name, const std::wstring& params, int64_t frame_id = -1);

  bool CallJSFunction(const std::wstring& js_function_name, const std::wstring& params, CefString const&frame_name);

  //关闭清理浏览器
  virtual void CloseBrowser(bool reset_js_bridge = true);

  // 修复浏览器
  virtual void RepairBrowser();

  //打开开发者工具
  virtual bool AttachDevTools();

  //关闭开发者工具
  virtual void DettachDevTools();

protected:
  //重建浏览器
  virtual void ReCreateBrowser() = 0;

  //接收拖拽事件
  virtual void OnDragStart();

public:
  //html页面发起拖拽激活信息
  OnDragStartEvent cb_drag_start_;

  //before contextmenu(不设置或者返回false禁用右键菜单) 非UI线程
  OnBeforeMenuEvent cb_before_menu_;

  //选择了某个右键菜单
  OnMenuCommandEvent cb_menu_command_;

  OnTitleChangeEvent cb_title_change_;

  OnUrlChangeEvent cb_url_change_;

  OnMainURLChengeEvent cb_main_url_change_;

  //非UI线程
  OnLinkClickEvent cb_link_click_;

  OnLoadingStateChangeEvent cb_loadstate_change_;

  OnLoadStartEvent cb_load_start_;

  OnLoadEndEvent cb_load_end_;

  OnLoadErrorEvent cb_load_error_;

  OnAfterCreatedEvent cb_after_created_;

  OnBeforeCloseEvent cb_before_close_;

  OnBeforeDownloadEvent cb_before_download_;

  OnDownloadUpdatedEvent cb_download_updated_;

  OnFileDialogEvent cb_file_dialog_;

  OnDevToolAttachedStateChangeEvent cb_devtool_visible_change_;

protected:
  friend class CefMouseHook;

  virtual bool ShouldCaptureMouseWhenLButtonDown() const { return false; }

  virtual void OnDragEvent(UINT msg_id, HWND wnd, POINT pt_window);

  // 处理BrowserHandler的HandlerDelegate委托接口
  virtual void OnPaint(CefRefPtr<CefBrowser> browser,
    CefRenderHandler::PaintElementType type,
    const CefRenderHandler::RectList& dirtyRects,
    const std::string* buffer,
    int width,
    int height) OVERRIDE {}

  virtual void OnPopupShow(CefRefPtr<CefBrowser> browser, bool show) OVERRIDE {}

  virtual void OnPopupSize(CefRefPtr<CefBrowser> browser, const CefRect& rect) OVERRIDE {}

  virtual bool OnExecuteCppFunc(
    const CefString& function_name,
    const CefString& params,
    CefRefPtr<CefBrowser> browser);

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
    bool* no_javascript_access) OVERRIDE;

  //浏览器对象创建
  virtual bool OnAfterCreated(CefRefPtr<CefBrowser> browser) OVERRIDE;

  //用与浏览器对象创建完毕后调整上层窗口大小
  virtual void UpdateWindowPos() OVERRIDE;

  //浏览器将要关闭
  virtual void OnBeforeClose(CefRefPtr<CefBrowser> browser) OVERRIDE;

  //非ui线程
  virtual void OnBeforeContextMenu(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    CefRefPtr<CefContextMenuParams> params,
    CefRefPtr<CefMenuModel> model) OVERRIDE;

  //非ui线程
  virtual bool OnContextMenuCommand(CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    CefRefPtr<CefContextMenuParams> params,
    int command_id,
    CefContextMenuHandler::EventFlags event_flags) OVERRIDE;

  //ui 线程
  virtual bool OnTooltip(CefRefPtr<CefBrowser> browser, CefString& text) OVERRIDE;

  //ui线程
  virtual void OnAddressChange(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, const CefString& url) OVERRIDE;

  //ui线程
  virtual void OnTitleChange(CefRefPtr<CefBrowser> browser, const CefString& title) OVERRIDE;

  //ui线程
  virtual void OnLoadingStateChange(CefRefPtr<CefBrowser> browser, bool isLoading, bool canGoBack, bool canGoForward) OVERRIDE;

  //ui线程
  virtual void OnLoadStart(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame) OVERRIDE;

  //ui线程
  virtual void OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int httpStatusCode) OVERRIDE;

  //ui线程
  virtual void OnLoadError(CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    CefLoadHandler::ErrorCode errorCode,
    const CefString& errorText,
    const CefString& failedUrl) OVERRIDE;

  // 文件下载相关
  //非ui线程
  virtual void OnBeforeDownload(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefDownloadItem> download_item,
    const CefString& suggested_name,
    CefRefPtr<CefBeforeDownloadCallback> callback) OVERRIDE;

  //非ui线程
  virtual void OnDownloadUpdated(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefDownloadItem> download_item,
    CefRefPtr<CefDownloadItemCallback> callback) OVERRIDE;

  //非ui线程 打开文件 Dialog
  virtual bool OnFileDialog(
    CefRefPtr<CefBrowser> browser,
    CefDialogHandler::FileDialogMode mode,
    const CefString& title,
    const CefString& default_file_path,
    const std::vector<CefString>& accept_filters,
    int selected_accept_filter,
    CefRefPtr<CefFileDialogCallback> callback) OVERRIDE;

protected:

  CefRefPtr<BrowserHandler> browser_handler_{};
  std::unique_ptr<CefJSBridge> js_bridge_;
  CefString url_;
  bool devtool_attached_{ false };
};