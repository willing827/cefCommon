#include "cef_js_bridge.h"
#include "ipc_string_define.h"

namespace cef_render
{
  CefJSBridge::CefJSBridge()
  {

  }

  CefJSBridge::~CefJSBridge()
  {

  }

  bool CefJSBridge::CallCppFunction(const CefString& function_name, const CefString& params)
  {
    CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create(kCallCppFunctionMessage);

    message->GetArgumentList()->SetString(0, function_name);
    message->GetArgumentList()->SetString(1, params);

    // ������Ϣ�� browser ����
    return CefV8Context::GetCurrentContext()->GetBrowser()->SendProcessMessage(PID_BROWSER, message);
  }

  void CefJSBridge::RegisterJSFunc(const CefString& function_name, CefRefPtr<CefV8Value> func, int64_t frame_id)
  {
    render_registered_function_[{function_name, frame_id}] = func;
  }

  void CefJSBridge::UnRegisterJSFunc(const CefString& function_name, int64_t frame_id)
  {
    render_registered_function_.erase(std::make_pair(function_name, frame_id));
  }

  void CefJSBridge::UnRegisterJSFuncWithFrame(CefRefPtr<CefFrame> frame)
  {
    // ���ڱ�����ÿһ�� render �� browser ���̶�����һ��ʵ���������ǵ���ģʽ
    // ���������ȡ�� browser ����ȫ��Ψһ�ģ����Ը������ browser ��ȡ���� frame �� context
    auto browser = frame->GetBrowser();

    if (!render_registered_function_.empty()){
      for (auto it = render_registered_function_.begin(); it != render_registered_function_.end();){
        auto child_frame = browser->GetFrame(it->first.second);
        if (child_frame.get() && child_frame->GetV8Context()->IsSame(frame->GetV8Context())){
          it = render_registered_function_.erase(it);
        } else{
          it++;
        }
      }
    }
  }

  bool CefJSBridge::ExecuteJSFunc(const CefString& function_name, const CefString& json_params, CefRefPtr<CefFrame> frame)
  {
    if (!frame.get()) return false;
    auto it = render_registered_function_.find(std::make_pair(function_name, frame->GetIdentifier()));
    if (it != render_registered_function_.cend()) {
      auto context = frame->GetV8Context();
      auto func = it->second;

      if (context.get() && func.get()) {
        context->Enter();

        CefV8ValueList arguments;
        arguments.push_back(CefV8Value::CreateString(json_params));

        CefRefPtr<CefV8Value> retval = func->ExecuteFunction(NULL, arguments);

        context->Exit();

        return true;
      }
    }

    return false;
  }
}