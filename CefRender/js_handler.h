#pragma once
#include "include/cef_base.h"
#include "include/cef_app.h"
#include "cef_js_bridge.h"

namespace cef_render
{
  class CefJSHandler : public CefV8Handler
  {
  public:
    CefJSHandler() {}
    virtual bool Execute(
      const CefString& name, 
      CefRefPtr<CefV8Value> object, 
      const CefV8ValueList& arguments, 
      CefRefPtr<CefV8Value>& retval, 
      CefString& exception) OVERRIDE;
    void AttachJSBridge(std::shared_ptr<CefJSBridge> js_bridge) { js_bridge_ = js_bridge; }


    IMPLEMENT_REFCOUNTING(CefJSHandler);
  private:
    std::shared_ptr<CefJSBridge> js_bridge_;
  };

}