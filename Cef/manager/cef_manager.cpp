#include "cef_manager.h"
#include "include/wrapper/cef_closure_task.h"
#include "include/base/cef_bind.h"

#include "app/client_app.h"
#include "handler/browser_handler.h"
#include "util/util.h"

#include "mouse_hook/cef_mouse_hook.h"

#include <Shlwapi.h>
#pragma comment(lib,"shlwapi.lib")


namespace
{
  std::wstring BackslashStandard(std::wstring& str)
  {
    for (;;)
    {
      auto find_pos = str.find('/');
      if (find_pos == std::wstring::npos) break;
      str.replace(find_pos, 1, L"\\");
    }
    return str;
  }

  //递归创建目录
  bool CreateDirRecursion(LPCWSTR dir_path)
  {
    if (dir_path == NULL) return false;
    if (::PathFileExists(dir_path)) {
      return !!::PathIsDirectory(dir_path);
    }

    std::wstring dir_path_str{ dir_path };
    BackslashStandard(dir_path_str);

    auto find_last_sep = dir_path_str.rfind('\\');
    if (find_last_sep != std::wstring::npos && !CreateDirRecursion(dir_path_str.substr(0, find_last_sep).c_str())) {
      return false;
    } else {
      return !!::CreateDirectory(dir_path_str.c_str(), NULL);
    }
  }
}

CefManager::CefManager()
{
  browser_count_ = 0;
  message_wnd_.reset(new helper_lib::MessageWnd(L"CefMessageWnd"));
}

bool CefManager::PostTaskUIThread(std::function<void()> const& task)
{
  auto ins = CefManager::GetInstance();
  if (ins->message_wnd_ && ins->message_wnd_->GetHWnd() != INVALID_HANDLE_VALUE) {
    return ins->message_wnd_->Asyn(task);
  }
  return false;
}

bool CefManager::PostTaskUIThread(helper_lib::AsyncTask *task)
{
  if (task == nullptr) return false;
  auto ins = CefManager::GetInstance();
  if (ins->message_wnd_ && ins->message_wnd_->GetHWnd() != INVALID_HANDLE_VALUE) {
    return ins->message_wnd_->Asyn(task);
  } else {
    delete task;
  }
  return false;
}

void CefManager::AddCefDllToPath(LPCWSTR cef_path /*= nullptr*/)
{
  TCHAR path_envirom[4096] = { 0 };
  GetEnvironmentVariable(L"path", path_envirom, 4096);

  std::wstring cef_bundle_path;
  if (cef_path) {
    cef_bundle_path = cef_path;
  } else {
    std::wstring module_file_path;
    module_file_path.resize(MAX_PATH);
    ::GetModuleFileName(NULL, &module_file_path[0], MAX_PATH);
    ::PathRemoveFileSpec(&module_file_path[0]);
#ifdef _DEBUG
    //::PathCombine(&module_file_path[0], &module_file_path[0], L"cef_debug");
    ::PathCombine(&module_file_path[0], &module_file_path[0], L"cef");
#else
    ::PathCombine(&module_file_path[0], &module_file_path[0], L"cef");
#endif
    cef_bundle_path = module_file_path.c_str();
}


  if (!PathFileExists(cef_bundle_path.c_str()))
  {
    MessageBox(NULL, L"未找到cef资源包", L"提示", MB_OK);
    exit(0);
  }

  std::wstring new_envirom(cef_bundle_path);
  new_envirom.append(L";").append(path_envirom);
  SetEnvironmentVariable(L"path", new_envirom.c_str());
}


bool CefManager::Initialize(const std::wstring& app_data_dir, CefSettings& settings)
{
  CefMainArgs main_args(::GetModuleHandle(NULL));
  CefRefPtr<ClientApp> app(new ClientApp);

  GetCefSetting(app_data_dir, settings);

  bool is_success = message_wnd_ && message_wnd_->CreateWnd(::GetModuleHandle(NULL))
    && CefInitialize(main_args, settings, app.get(), NULL);
  if (is_success) {
    CefMouseHook::Instance()->Init();
  }

  return is_success;
}

void CefManager::UnInitialize()
{
  if (message_wnd_) {
    message_wnd_->SendMessage(WM_DESTROY, 0, 0);
  }

  message_wnd_.reset();

  CefShutdown();
}

void CefManager::AddBrowserCount()
{
  browser_count_++;
}

void CefManager::SubBrowserCount()
{
  browser_count_--;
  DCHECK(browser_count_ >= 0);
}

int CefManager::GetBrowserCount()
{
  return browser_count_;
}

void CefManager::GetCefSetting(const std::wstring& app_data_dir, CefSettings& settings)
{
  settings.no_sandbox = true;
  CefString(&settings.browser_subprocess_path).FromASCII("CefRender.exe");
  CefString(&settings.locale).FromASCII("zh-CN");
  settings.multi_threaded_message_loop = true;
  settings.windowless_rendering_enabled = true;
#ifdef _DEBUG
  settings.remote_debugging_port = 8080;
#else

#endif

  std::wstring app_data_dir_tmp = app_data_dir;
  app_data_dir_tmp.resize(MAX_PATH);
  BackslashStandard(app_data_dir_tmp);

  bool create_dir_success = CreateDirRecursion(app_data_dir.c_str());

  ::PathAddBackslash(&app_data_dir_tmp[0]);

  std::wstring tmp = app_data_dir_tmp.c_str();

  // 设置localstorage，不要在路径末尾加"\\"，否则运行时会报错
  CefString(&settings.cache_path) = tmp + L"CefLocalStorage";

  // 设置debug log文件位置
  CefString(&settings.log_file) = tmp + L"cef.log";

}