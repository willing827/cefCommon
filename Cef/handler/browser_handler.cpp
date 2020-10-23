#include "browser_handler.h"
#include "include/cef_frame.h"
#include "manager/cef_manager.h"
#include "app/cef_js_bridge.h"

#include "../CefRender/ipc_string_define.h"

BrowserHandler::BrowserHandler()
{

}

void BrowserHandler::SetViewRect(RECT rc)
{
    if (!CefCurrentlyOn(TID_UI)) {
        // 把操作跳转到Cef线程执行
        CefPostTask(TID_UI, base::Bind(&BrowserHandler::SetViewRect, this, rc));
        return;
    }

    rect_cef_control_ = rc;
    // 调用WasResized接口，调用后，BrowserHandler会调用GetViewRect接口来获取浏览器对象新的位置
    if (browser_.get() && browser_->GetHost().get()) browser_->GetHost()->WasResized();
}

void BrowserHandler::SetIsVisible(bool is_visible)
{
    is_visible_ = is_visible;
    if (browser_.get() && browser_->GetHost().get())
        browser_->GetHost()->WasHidden(!is_visible_);
}

CefRefPtr<CefBrowserHost> BrowserHandler::GetBrowserHost()
{
    if (browser_.get()) {
        return browser_->GetHost();
    }
    return {};
}

void BrowserHandler::CloseBrowser()
{
    is_closing_ = true;
    if (browser_.get() && browser_->GetHost().get()) {
        CefPostTask(TID_UI,
            MakeCefTask([=] {
            if (browser_.get() && browser_->GetHost().get()) {
                browser_->GetHost()->CloseBrowser(true);
            }
        }));
    }
}

bool BrowserHandler::OnProcessMessageReceived(
    CefRefPtr<CefBrowser> browser,
    CefProcessId source_process,
    CefRefPtr<CefProcessMessage> message)
{
    //处理render进程发来的消息
    std::string message_name = message->GetName();
    if (message_name == cef_render::kFocusedNodeChangedMessage) {
        is_focus_oneditable_field_ = message->GetArgumentList()->GetBool(0);
        return true;
    } else if (message_name == cef_render::kCallCppFunctionMessage) {
        CefString fun_name = message->GetArgumentList()->GetString(0);
        CefString param = message->GetArgumentList()->GetString(1);

        CefManager::PostTaskUIThread([=] {
            if (handle_delegate_) {
                handle_delegate_->OnExecuteCppFunc(fun_name, param, browser);
            }
        });

        return true;
    }

    return false;
}

#pragma region CefLifeSpanHandler
// CefLifeSpanHandler methods
bool BrowserHandler::OnBeforePopup(CefRefPtr<CefBrowser> browser,
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
    // 让新的链接在原浏览器对象中打开
    if (browser_.get() && !target_url.empty()) {
        if (handle_delegate_) {
            // 返回true则继续在控件内打开新链接，false则禁止访问
            bool bRet = handle_delegate_->OnBeforePopup(
                browser,
                frame,
                target_url,
                target_frame_name,
                target_disposition,
                user_gesture,
                popupFeatures,
                windowInfo,
                client,
                settings,
                no_javascript_access);
            if (bRet)
                browser_->GetMainFrame()->LoadURL(target_url);
        } else
            browser_->GetMainFrame()->LoadURL(target_url);
    }

    // 禁止弹出popup窗口
    return true;
}

void BrowserHandler::OnAfterCreated(CefRefPtr<CefBrowser> browser)
{
    DCHECK(CefCurrentlyOn(TID_UI));

    DCHECK(!browser_.get());

    if (is_closing_) {
        browser_->GetHost()->CloseBrowser(true);
        return;
    }

    browser_ = browser;

    CefManager::PostTaskUIThread([=]() {
        SetIsVisible(is_visible_);
        CefManager::GetInstance()->AddBrowserCount();

        if (handle_delegate_) {
            handle_delegate_->OnAfterCreated(browser);

            // 有窗模式下，浏览器创建完毕后，让上层更新一下自己的位置；因为在异步状态下，上层更新位置时可能Cef窗口还没有创建出来
            handle_delegate_->UpdateWindowPos();
        }

        std::for_each(task_list_after_created_.begin(), task_list_after_created_.end(), [](auto fn) {fn(); });
        task_list_after_created_.clear();
    });
}

bool BrowserHandler::DoClose(CefRefPtr<CefBrowser> browser)
{
    return false;
}

void BrowserHandler::OnBeforeClose(CefRefPtr<CefBrowser> browser)
{
    DCHECK(CefCurrentlyOn(TID_UI));

    DCHECK(browser_ == browser);

    browser_ = nullptr;

    CefManager::PostTaskUIThread([this, browser]() {
        CefManager::GetInstance()->SubBrowserCount();
        if (handle_delegate_) {
            handle_delegate_->OnBeforeClose(browser);
        }
    });
}

#pragma endregion

#pragma region CefRenderHandler
// CefRenderHandler methods
bool BrowserHandler::GetRootScreenRect(CefRefPtr<CefBrowser> browser, CefRect& rect)
{
    RECT window_rect = { 0 };
    HWND root_window = ::GetAncestor(host_wnd_, GA_ROOT);
    if (::GetWindowRect(root_window, &window_rect)) {
        rect = CefRect(window_rect.left, window_rect.top, window_rect.right - window_rect.left, window_rect.bottom - window_rect.top);
        return true;
    }
    return false;
}

void BrowserHandler::GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect)
{
    if (handle_delegate_) {
        rect.x = 0;
        rect.y = 0;
        rect.width = rect_cef_control_.right - rect_cef_control_.left;
        rect.height = rect_cef_control_.bottom - rect_cef_control_.top;
        rect.width = max(rect.width, 1);
        rect.height = max(rect.height, 1);
        //return true;
    } else {
        RECT clientRect;
        if (!::GetClientRect(host_wnd_, &clientRect))
            return /*false*/;
        rect.x = rect.y = 0;
        rect.width = max(clientRect.right, 1);
        rect.height = max(clientRect.bottom, 1);
        //return true;
    }
}

bool BrowserHandler::GetScreenPoint(CefRefPtr<CefBrowser> browser, int viewX, int viewY, int& screenX, int& screenY)
{
    if (!::IsWindow(host_wnd_))
        return false;

    // Convert the point from view coordinates to actual screen coordinates.
    POINT screen_pt = { viewX, viewY };
    ::ClientToScreen(host_wnd_, &screen_pt);
    screenX = screen_pt.x;
    screenY = screen_pt.y;
    return true;
}

void BrowserHandler::OnPopupShow(CefRefPtr<CefBrowser> browser, bool show)
{
    if (handle_delegate_) {
        CefManager::PostTaskUIThread([=] {
            handle_delegate_->OnPopupShow(browser, show);
        });
    }
}

void BrowserHandler::OnPopupSize(CefRefPtr<CefBrowser> browser, const CefRect& rect)
{
    if (handle_delegate_) {
        CefManager::PostTaskUIThread([=] {
            handle_delegate_->OnPopupSize(browser, rect);
        });
    }
}

void BrowserHandler::OnPaint(CefRefPtr<CefBrowser> browser,
    PaintElementType type,
    const RectList& dirtyRects,
    const void* buffer,
    int width,
    int height)
{
    if (handle_delegate_) {
        int buffer_length = width * height * 4;
        if (buffer_length > (int)paint_buffer_.size()) {
            paint_buffer_.resize(buffer_length);
        }
        memcpy(&paint_buffer_[0], (char*)buffer, width * height * 4);
        CefManager::PostTaskUIThread([=] {handle_delegate_->OnPaint(browser, type, dirtyRects, &paint_buffer_, width, height); });
    }
}

void BrowserHandler::OnCursorChange(CefRefPtr<CefBrowser> browser, CefCursorHandle cursor, CursorType type, const CefCursorInfo& custom_cursor_info)
{
    SetClassLongPtr(host_wnd_, GCLP_HCURSOR, static_cast<LONG>(reinterpret_cast<LONG_PTR>(cursor)));
    SetCursor(cursor);
}

#pragma endregion

#pragma region CefContextMenuHandler
// CefContextMenuHandler methods
void BrowserHandler::OnBeforeContextMenu(CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    CefRefPtr<CefContextMenuParams> params,
    CefRefPtr<CefMenuModel> model)
{
    REQUIRE_UI_THREAD();

    if (handle_delegate_) {
        handle_delegate_->OnBeforeContextMenu(browser, frame, params, model);
    } else {
        // Customize the context menu...
        if ((params->GetTypeFlags() & (CM_TYPEFLAG_PAGE | CM_TYPEFLAG_FRAME)) != 0) {
            if (model->GetCount() > 0) {
                // 禁止右键菜单
                model->Clear();
            }
        }
    }
}

bool BrowserHandler::RunContextMenu(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefContextMenuParams> params, CefRefPtr<CefMenuModel> model, CefRefPtr<CefRunContextMenuCallback> callback)
{
    return false;
}

bool BrowserHandler::OnContextMenuCommand(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefContextMenuParams> params, int command_id, EventFlags event_flags)
{
    if (handle_delegate_)
        return handle_delegate_->OnContextMenuCommand(browser, frame, params, command_id, event_flags);
    else
        return false;
}

void BrowserHandler::OnContextMenuDismissed(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame)
{

}

#pragma endregion

#pragma region CefDisplayHandler

// CefDisplayHandler methods
bool BrowserHandler::OnTooltip(CefRefPtr<CefBrowser> browser, CefString& text)
{
    if (handle_delegate_)
        CefManager::PostTaskUIThread(std::bind(&HandlerDelegate::OnTooltip, handle_delegate_, browser, text));
    return false;
}

void BrowserHandler::OnAddressChange(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, const CefString& url)
{
    if (handle_delegate_)
        CefManager::PostTaskUIThread(std::bind(&HandlerDelegate::OnAddressChange, handle_delegate_, browser, frame, url));
}

void BrowserHandler::OnTitleChange(CefRefPtr<CefBrowser> browser, const CefString& title)
{
    if (handle_delegate_)
        CefManager::PostTaskUIThread(std::bind(&HandlerDelegate::OnTitleChange, handle_delegate_, browser, title));
}

bool BrowserHandler::OnConsoleMessage(
    CefRefPtr<CefBrowser> browser,
    cef_log_severity_t level,
    const CefString& message,
    const CefString& source,
    int line)
{
#ifdef _DEBUG
    return false;
#else
    return true;
#endif //_DEBUG
}

#pragma endregion

// CefLoadHandler methods
void BrowserHandler::OnLoadingStateChange(CefRefPtr<CefBrowser> browser, bool isLoading, bool canGoBack, bool canGoForward)
{
    if (handle_delegate_)
        CefManager::PostTaskUIThread(std::bind(&HandlerDelegate::OnLoadingStateChange, handle_delegate_, browser, isLoading, canGoBack, canGoForward));
}

void BrowserHandler::OnLoadStart(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    TransitionType transition_type)
{
    if (handle_delegate_)
        CefManager::PostTaskUIThread(std::bind(&HandlerDelegate::OnLoadStart, handle_delegate_, browser, frame));
}

void BrowserHandler::OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int httpStatusCode)
{
    if (handle_delegate_)
        CefManager::PostTaskUIThread(std::bind(&HandlerDelegate::OnLoadEnd, handle_delegate_, browser, frame, httpStatusCode));
}

void BrowserHandler::OnLoadError(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, ErrorCode errorCode, const CefString& errorText, const CefString& failedUrl)
{
    if (handle_delegate_)
        CefManager::PostTaskUIThread(std::bind(&HandlerDelegate::OnLoadError, handle_delegate_, browser, frame, errorCode, errorText, failedUrl));
}

bool BrowserHandler::OnJSDialog(CefRefPtr<CefBrowser> browser,
    const CefString& origin_url,
    JSDialogType dialog_type,
    const CefString& message_text,
    const CefString& default_prompt_text,
    CefRefPtr<CefJSDialogCallback> callback,
    bool& suppress_message)
{
    // release时阻止弹出js对话框
#ifndef _DEBUG
    suppress_message = true;
#endif

    return false;
}

void BrowserHandler::OnBeforeDownload(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefDownloadItem> download_item,
    const CefString& suggested_name,
    CefRefPtr<CefBeforeDownloadCallback> callback)
{
    if (handle_delegate_)
        handle_delegate_->OnBeforeDownload(browser, download_item, suggested_name, callback);
}

void BrowserHandler::OnDownloadUpdated(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefDownloadItem> download_item,
    CefRefPtr<CefDownloadItemCallback> callback)
{
    if (handle_delegate_)
        handle_delegate_->OnDownloadUpdated(browser, download_item, callback);
}

bool BrowserHandler::OnFileDialog(
    CefRefPtr<CefBrowser> browser,
    FileDialogMode mode,
    const CefString& title,
    const CefString& default_file_path,
    const std::vector<CefString>& accept_filters,
    int selected_accept_filter,
    CefRefPtr<CefFileDialogCallback> callback)
{
    if (handle_delegate_)
        return handle_delegate_->OnFileDialog(browser, mode, title, default_file_path, accept_filters, selected_accept_filter, callback);
    else
        return false;
}