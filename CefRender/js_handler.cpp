#include "js_handler.h"
#include "ipc_string_define.h"
#include "cef_js_bridge.h"

namespace cef_render
{
  bool CefJSHandler::Execute(const CefString& name, CefRefPtr<CefV8Value> object, const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval, CefString& exception)
  {
    CefRefPtr<CefV8Context> context = CefV8Context::GetCurrentContext();
    CefRefPtr<CefFrame> frame = context->GetFrame();
    CefRefPtr<CefBrowser> browser = context->GetBrowser();

    int64_t browser_id = browser->GetIdentifier();
    int64_t frame_id = frame->GetIdentifier();

    size_t sz = arguments.size();
    bool is_valid = sz > 0 && arguments[0]->IsString();
    if (!is_valid) {
      exception = "Invalid arguments.";
      return false;
    }

    if (name == "call") {
      CefString function_name = arguments[0]->GetStringValue();
      CefString params = "";
      if (arguments[1]->IsString()) {
        params = arguments[1]->GetStringValue();
      }

      if (!js_bridge_->CallCppFunction(function_name, params)) {
        WCHAR msg[128]{};
        wsprintf(msg, L"Failed to call function %s.", function_name.c_str());
        exception = msg;
        return false;
      }

      return true;
    } else if (name == "register"){
      if (arguments[0]->IsString() && arguments[1]->IsFunction()){
        std::string function_name = arguments[0]->GetStringValue();
        CefRefPtr<CefV8Value> callback = arguments[1];
        js_bridge_->RegisterJSFunc(function_name, callback, frame_id);
        return true;
      } else {
        exception = "Invalid arguments.";
        return false;
      }
    } else if (name == "unregister") {
      if (arguments[0]->IsString()) {
        std::string function_name = arguments[0]->GetStringValue();
        js_bridge_->UnRegisterJSFunc(function_name, frame_id);
        return true;
      } else {
        exception = "Invalid arguments.";
        return false;
      }
    }

    return false;
  }

}