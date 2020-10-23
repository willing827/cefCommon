#include "cef_js_bridge.h"
#include "../CefRender/ipc_string_define.h"
#include "manager/cef_manager.h"


CefJSBridge::CefJSBridge()
{

}

CefJSBridge::~CefJSBridge()
{

}

bool CefJSBridge::CallJSFunction(const CefString& js_function_name, const CefString& params, CefRefPtr<CefFrame> frame)
{
  if (!frame.get()) {
    return false;
  }

  // 发送消息给 render 要求执行一个 js function
  CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create(cef_render::kCallJsFunctionMessage);
  CefRefPtr<CefListValue> args = message->GetArgumentList();
  args->SetString(0, js_function_name);
  args->SetString(1, params);
  args->SetInt(2, (int)frame->GetIdentifier());

  return frame->GetBrowser()->SendProcessMessage(PID_RENDERER, message);
}

void CefJSBridge::RegisterCppFunc(const CefString& function_name, CppFunction func, int browser_id)
{
  browser_registered_function_[{function_name, browser_id}] = func;
}

void CefJSBridge::UnRegisterCppFunc(const CefString& function_name, int browser_id)
{
  browser_registered_function_.erase({ function_name,browser_id });
}

bool CefJSBridge::ExecuteCppFunc(const CefString& function_name, const CefString& params, int browser_id)
{
  auto it = browser_registered_function_.find({ function_name,browser_id });
  if (it == browser_registered_function_.cend()) {
    it = browser_registered_function_.find({ function_name,-1 });
  }
  if (it != browser_registered_function_.cend()) {
    CppFunction func = it->second;
    func(params);
    return true;
  } else {
    return false;
  }
}
