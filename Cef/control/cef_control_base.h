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

  //����ҳ�����ű���
  void SetZoomLevel(float zoom_level);

  // ��ȡ[CEF]��������ھ��
  HWND GetCefHandle() const;

  // ��ȡ�ؼ��������ھ��(��CefHostWnd)
  HWND GetHostWindow() const;

  //��ȡҳ�� URL
  CefString GetURL() const;

  //��ȡ UTF8 ��ʽ URL
  std::string GetUTF8URL() const;

  //��ȡ��ַ # ��ǰ�ĵ�ַ
  static CefString GetMainURL(const CefString& url);

  //ע��һ�� C++ �����ṩǰ�˵���
  void RegisterCppFunc(const std::wstring& function_name, CppFunction func, int browser_id = -1);

  //��ע��һ�� C++ ����
  void UnRegisterCppFunc(const std::wstring& function_name, int browser_id = -1);

  //����һ��ǰ���Ѿ�ע��õķ���
  bool CallJSFunction(const std::wstring& js_function_name, const std::wstring& params, int64_t frame_id = -1);

  bool CallJSFunction(const std::wstring& js_function_name, const std::wstring& params, CefString const&frame_name);

  //�ر����������
  virtual void CloseBrowser(bool reset_js_bridge = true);

  // �޸������
  virtual void RepairBrowser();

  //�򿪿����߹���
  virtual bool AttachDevTools();

  //�رտ����߹���
  virtual void DettachDevTools();

protected:
  //�ؽ������
  virtual void ReCreateBrowser() = 0;

  //������ק�¼�
  virtual void OnDragStart();

public:
  //htmlҳ�淢����ק������Ϣ
  OnDragStartEvent cb_drag_start_;

  //before contextmenu(�����û��߷���false�����Ҽ��˵�) ��UI�߳�
  OnBeforeMenuEvent cb_before_menu_;

  //ѡ����ĳ���Ҽ��˵�
  OnMenuCommandEvent cb_menu_command_;

  OnTitleChangeEvent cb_title_change_;

  OnUrlChangeEvent cb_url_change_;

  OnMainURLChengeEvent cb_main_url_change_;

  //��UI�߳�
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

  // ����BrowserHandler��HandlerDelegateί�нӿ�
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

  //��ui�߳�
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

  //��������󴴽�
  virtual bool OnAfterCreated(CefRefPtr<CefBrowser> browser) OVERRIDE;

  //������������󴴽���Ϻ�����ϲ㴰�ڴ�С
  virtual void UpdateWindowPos() OVERRIDE;

  //�������Ҫ�ر�
  virtual void OnBeforeClose(CefRefPtr<CefBrowser> browser) OVERRIDE;

  //��ui�߳�
  virtual void OnBeforeContextMenu(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    CefRefPtr<CefContextMenuParams> params,
    CefRefPtr<CefMenuModel> model) OVERRIDE;

  //��ui�߳�
  virtual bool OnContextMenuCommand(CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    CefRefPtr<CefContextMenuParams> params,
    int command_id,
    CefContextMenuHandler::EventFlags event_flags) OVERRIDE;

  //ui �߳�
  virtual bool OnTooltip(CefRefPtr<CefBrowser> browser, CefString& text) OVERRIDE;

  //ui�߳�
  virtual void OnAddressChange(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, const CefString& url) OVERRIDE;

  //ui�߳�
  virtual void OnTitleChange(CefRefPtr<CefBrowser> browser, const CefString& title) OVERRIDE;

  //ui�߳�
  virtual void OnLoadingStateChange(CefRefPtr<CefBrowser> browser, bool isLoading, bool canGoBack, bool canGoForward) OVERRIDE;

  //ui�߳�
  virtual void OnLoadStart(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame) OVERRIDE;

  //ui�߳�
  virtual void OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int httpStatusCode) OVERRIDE;

  //ui�߳�
  virtual void OnLoadError(CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    CefLoadHandler::ErrorCode errorCode,
    const CefString& errorText,
    const CefString& failedUrl) OVERRIDE;

  // �ļ��������
  //��ui�߳�
  virtual void OnBeforeDownload(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefDownloadItem> download_item,
    const CefString& suggested_name,
    CefRefPtr<CefBeforeDownloadCallback> callback) OVERRIDE;

  //��ui�߳�
  virtual void OnDownloadUpdated(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefDownloadItem> download_item,
    CefRefPtr<CefDownloadItemCallback> callback) OVERRIDE;

  //��ui�߳� ���ļ� Dialog
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