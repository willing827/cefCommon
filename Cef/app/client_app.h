#pragma once

#include "include/cef_app.h"

class ClientApp : public CefApp,
  public CefBrowserProcessHandler
{
public:
  ClientApp() = default;

private:
  virtual void OnBeforeCommandLineProcessing(const CefString& process_type, CefRefPtr<CefCommandLine> command_line) OVERRIDE
  {
    if (process_type.empty()) {
      command_line->AppendSwitchWithValue("ppapi-flash-version", "32.0.0.270");
      command_line->AppendSwitchWithValue("ppapi-flash-path", "PepperFlash\\pepflashplayer32_32_0_0_270.dll");

      //同一个域下的使用同一个渲染进程
      //command_line->AppendSwitch("process-per-site");
      //command_line->AppendSwitch("disable-gpu");
      //command_line->AppendSwitch("disable-gpu-compositing");
      //command_line->AppendSwitchWithValue("proxy-server", "SOCKS5://127.0.0.1:1080");	

    }
  }
  virtual void OnRegisterCustomSchemes(CefRawPtr<CefSchemeRegistrar> registrar) OVERRIDE
  {

  }

  virtual CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() OVERRIDE { return this; }

  // CefBrowserProcessHandler methods.
  virtual void OnContextInitialized() OVERRIDE
  {

  }

  virtual void OnBeforeChildProcessLaunch(
    CefRefPtr<CefCommandLine> command_line) OVERRIDE
  {

  }

  virtual void OnRenderProcessThreadCreated(
    CefRefPtr<CefListValue> extra_info) OVERRIDE
  {

  }
private:

  IMPLEMENT_REFCOUNTING(ClientApp);

};