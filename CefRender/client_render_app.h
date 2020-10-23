#pragma once
#include <include/cef_app.h>

#include "cef_js_bridge.h"

namespace cef_render
{
  class ClientRenderApp
    :public CefApp
    ,public CefRenderProcessHandler
  {
  protected:

    virtual void OnBeforeCommandLineProcessing(
      const CefString& process_type,
      CefRefPtr<CefCommandLine> command_line) OVERRIDE;

    virtual void OnRegisterCustomSchemes(
      CefRawPtr<CefSchemeRegistrar> registrar) OVERRIDE;

    virtual CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler() OVERRIDE { return this; }


    virtual void OnRenderThreadCreated(CefRefPtr<CefListValue> extra_info) OVERRIDE;

    virtual void OnWebKitInitialized() OVERRIDE;

    virtual void OnBrowserCreated(CefRefPtr<CefBrowser> browser) OVERRIDE;

    virtual void OnBrowserDestroyed(CefRefPtr<CefBrowser> browser) OVERRIDE;

    virtual void OnContextCreated(CefRefPtr<CefBrowser> browser,
      CefRefPtr<CefFrame> frame,
      CefRefPtr<CefV8Context> context) OVERRIDE;

    virtual void OnContextReleased(CefRefPtr<CefBrowser> browser,
      CefRefPtr<CefFrame> frame,
      CefRefPtr<CefV8Context> context) OVERRIDE;

    virtual void OnUncaughtException(CefRefPtr<CefBrowser> browser,
      CefRefPtr<CefFrame> frame,
      CefRefPtr<CefV8Context> context,
      CefRefPtr<CefV8Exception> exception,
      CefRefPtr<CefV8StackTrace> stackTrace) OVERRIDE;

    virtual void OnFocusedNodeChanged(CefRefPtr<CefBrowser> browser,
      CefRefPtr<CefFrame> frame,
      CefRefPtr<CefDOMNode> node) OVERRIDE;

    virtual bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
      CefProcessId source_process,
      CefRefPtr<CefProcessMessage> message) OVERRIDE;

  private:
    std::shared_ptr<CefJSBridge> render_js_bridge_;

    bool last_node_is_editable_{false};

    IMPLEMENT_REFCOUNTING(ClientRenderApp);
  };

}