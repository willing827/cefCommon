#include "client_render_app.h"
#include "js_handler.h"
#include "ipc_string_define.h"

namespace cef_render
{
  void ClientRenderApp::OnBeforeCommandLineProcessing(
    const CefString& process_type,
    CefRefPtr<CefCommandLine> command_line)
  {

  }

  void ClientRenderApp::OnRegisterCustomSchemes(CefRawPtr<CefSchemeRegistrar> registrar)
  {

  }

  void ClientRenderApp::OnRenderThreadCreated(CefRefPtr<CefListValue> extra_info)
  {

  }

  void ClientRenderApp::OnWebKitInitialized()
  {
    /**
   * JavaScript 扩展代码，这里定义一个 cef_web_func 对象提供 call 方法来让 Web 端触发 native 的 CefV8Handler 处理代码
   * param[in] functionName	要调用的 C++ 方法名称
   * param[in] params			调用该方法传递的参数，在前端指定的是一个 Object，但转到 Native 的时候转为了字符串
   * param[in] callback		执行该方法后的回调函数
   * 前端调用示例
   * cef.call('showMessage','msg');
   * cef.register("js_function_name",('msg') => {...})
   * cef.unregister("js_function_name")
   */

    std::string extensionCode = R"(
		var cef = {};
		(() => {
			cef.call = (functionName, arg1) => {
					native function call(functionName, arg1);
					return call(functionName, arg1);
			};
			cef.register = (functionName, callback) => {
				native function register(functionName, callback);
				return register(functionName, callback);
			};
      cef.unregister = (functionName) => {
        native function unregister(functionName);
				return unregister(functionName);
      };
		})();
	)";

    CefRefPtr<CefJSHandler> handler = new CefJSHandler();

    if (!render_js_bridge_)
      render_js_bridge_.reset(new CefJSBridge);
    handler->AttachJSBridge(render_js_bridge_);
    CefRegisterExtension("v8/extern", extensionCode, handler);
  }

  void ClientRenderApp::OnBrowserCreated(CefRefPtr<CefBrowser> browser)
  {
    if (!render_js_bridge_.get())
      render_js_bridge_.reset(new CefJSBridge);
  }

  void ClientRenderApp::OnBrowserDestroyed(CefRefPtr<CefBrowser> browser)
  {

  }

  void ClientRenderApp::OnContextCreated(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    CefRefPtr<CefV8Context> context)
  {

  }

  void ClientRenderApp::OnContextReleased(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    CefRefPtr<CefV8Context> context)
  {
    render_js_bridge_->UnRegisterJSFuncWithFrame(frame);
  }

  void ClientRenderApp::OnUncaughtException(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    CefRefPtr<CefV8Context> context,
    CefRefPtr<CefV8Exception> exception,
    CefRefPtr<CefV8StackTrace> stackTrace)
  {

  }

  void ClientRenderApp::OnFocusedNodeChanged(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    CefRefPtr<CefDOMNode> node)
  {
    bool is_editable = (node.get() && node->IsEditable());
    if (is_editable != last_node_is_editable_)
    {
      // Notify the browser of the change in focused element type.
      last_node_is_editable_ = is_editable;
      CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create(kFocusedNodeChangedMessage);

      message->GetArgumentList()->SetBool(0, is_editable);
      frame->GetBrowser()->SendProcessMessage(PID_BROWSER, message);
    }
  }

  bool ClientRenderApp::OnProcessMessageReceived(
    CefRefPtr<CefBrowser> browser,
    CefProcessId source_process,
    CefRefPtr<CefProcessMessage> message)
  {
    DCHECK(source_process == PID_BROWSER);

    const CefString& message_name = message->GetName();
    if (message_name == kCallJsFunctionMessage) {
      CefString function_name = message->GetArgumentList()->GetString(0);
      CefString json_string = message->GetArgumentList()->GetString(1);
      int64 frame_id = message->GetArgumentList()->GetInt(2);

      CefRefPtr<CefFrame> js_frame = frame_id < 0 ? browser->GetMainFrame() : browser->GetFrame(frame_id);
      js_frame = js_frame.get() ? js_frame : browser->GetMainFrame();
      return render_js_bridge_->ExecuteJSFunc(function_name, json_string, js_frame);
    }

    return false;
  }

}