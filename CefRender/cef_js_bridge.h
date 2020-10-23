#pragma once

#include <functional>
#include "include/cef_app.h"

namespace cef_render
{
  typedef std::map<std::pair<CefString/* function_name*/, int64_t /*frame_id*/>, CefRefPtr<CefV8Value>/* function*/> RenderRegisteredFunction;

  class CefJSBridge
  {
  public:
    CefJSBridge();
    ~CefJSBridge();

  public:
    /**
     * 执行已经注册好的 C++ 方法
     * param[in] function_name	要调用的函数名称
     * param[in] params			调用该函数传递的 json 格式参数
     * return 返回 true 表示发起执行请求成功
     */
    bool CallCppFunction(const CefString& function_name, const CefString& params);


    /**
     * 注册一个持久的 JS 函数提供 C++ 调用
     * param[in] function_name	函数名称，字符串形式提供 C++ 直接调用，名称不能重复
     * param[in] func		函数体
     */
    void RegisterJSFunc(const CefString& function_name, CefRefPtr<CefV8Value> func, int64_t frame_id);

    /**
     * 反注册一个持久的 JS 函数
     * param[in] function_name	函数名称
     * param[in] frame			要取消注册哪个框架下的相关函数
     */
    void UnRegisterJSFunc(const CefString& function_name, int64_t frame_id);

    /**
    * 根据执行上下文反注册一个或多个持久的 JS 函数
    * param[in] frame			当前运行所属框架
    */
    void UnRegisterJSFuncWithFrame(CefRefPtr<CefFrame> frame);

    /**
     * 根据名称执行某个具体的 JS 函数
     * param[in] function_name	函数名称
     * param[in] json_params	要传递的 json 格式的参数
     * param[in] frame			执行哪个框架下的 JS 函数
     * return 返回 true 表示成功执行某个 JS 函数
     */
    bool ExecuteJSFunc(const CefString& function_name, const CefString& json_params, CefRefPtr<CefFrame> frame);

  private:
    RenderRegisteredFunction	render_registered_function_;	// 保存 JS 端已经注册好的持久函数列表
  };

}