// CefRender.cpp : 定义应用程序的入口点。
//

#include "framework.h"
#include "client_render_app.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
  _In_opt_ HINSTANCE hPrevInstance,
  _In_ LPWSTR    lpCmdLine,
  _In_ int       nCmdShow)
{
  UNREFERENCED_PARAMETER(hPrevInstance);
  UNREFERENCED_PARAMETER(lpCmdLine);

  // Enable High-DPI support on Windows 7 or newer.
  CefEnableHighDPISupport();

  // Provide CEF with command-line arguments.
  CefMainArgs main_args(hInstance);

  CefRefPtr<cef_render::ClientRenderApp> app{ new cef_render::ClientRenderApp };

  return CefExecuteProcess(main_args, app, NULL);
}
