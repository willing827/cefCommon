#pragma once
#include <functional>

#include "include/cef_load_handler.h"
#include "include/cef_request_handler.h"
#include "include/cef_context_menu_handler.h"
#include "include/cef_download_handler.h"
#include "include/cef_dialog_handler.h"

#include "Helper/event_observer.h"

using OnDragStartEvent = helper_lib::Events<>;
typedef std::function<bool(CefRefPtr<CefContextMenuParams> params, CefRefPtr<CefMenuModel> model)> OnBeforeMenuEvent;
typedef std::function<bool(CefRefPtr<CefContextMenuParams> params, int command_id, CefContextMenuHandler::EventFlags event_flags)> OnMenuCommandEvent;
typedef std::function<void(const std::wstring &title)> OnTitleChangeEvent;
typedef std::function<void(const std::wstring &url)> OnUrlChangeEvent;
typedef std::function<bool(const std::wstring &url)> OnLinkClickEvent;
typedef std::function<void(const CefString &old_url, const CefString &new_url)> OnMainURLChengeEvent;
typedef std::function<void(bool isLoading, bool canGoBack, bool canGoForward)> OnLoadingStateChangeEvent;
using OnLoadStartEvent = helper_lib::Events<>;
using OnLoadEndEvent = helper_lib::Events<int /*httpStatusCode*/>;
using OnLoadErrorEvent = helper_lib::Events<CefLoadHandler::ErrorCode /*errorCode*/, const CefString & /*errorText*/, const CefString & /*failedUrl*/>;
using OnAfterCreatedEvent = helper_lib::Events<CefRefPtr<CefBrowser> /*browser*/>;
using OnBeforeCloseEvent = helper_lib::Events<CefRefPtr<CefBrowser> /*browser*/>;
typedef std::function<void(CefRefPtr<CefBrowser> browser, CefRefPtr<CefDownloadItem> download_item, const CefString & suggested_name, CefRefPtr<CefBeforeDownloadCallback> callback)> OnBeforeDownloadEvent;
typedef std::function<void(CefRefPtr<CefBrowser> browser, CefRefPtr<CefDownloadItem> download_item, CefRefPtr<CefDownloadItemCallback> callback)> OnDownloadUpdatedEvent;
typedef std::function<bool(CefDialogHandler::FileDialogMode mode, const CefString & title, const CefString & default_file_path, const std::vector<CefString> & accept_filters, int selected_accept_filter, CefRefPtr<CefFileDialogCallback> callback)> OnFileDialogEvent;
typedef std::function<void(bool visible)> OnDevToolAttachedStateChangeEvent;