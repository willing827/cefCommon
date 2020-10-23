#pragma once

#include <functional>
#include "include/cef_app.h"

typedef std::function<void(const std::string& params)> CppFunction;

typedef std::map<std::pair<CefString /*function_name*/, int /*browser_id*/>, CppFunction/* function*/> BrowserRegisteredFunction;

class CefJSBridge
{
public:
  CefJSBridge();
  ~CefJSBridge();
public:

  /**
   * 执行已经注册好的 JS 方法
   * param[in] js_function_name 要调用的 JS 函数名称
   * param[in] params			调用 JS 方法传递的 json 格式参数
   * param[in] browser	  调用哪个浏览器下的 JS 代码
   * return 返回 ture 标识发起执行 JS 函数命令成功
   */
  bool CallJSFunction(const CefString& js_function_name, const CefString& params, CefRefPtr<CefFrame> frame);

  /**
   * 注册一个持久的 C++ 函数提供 JS 端调用
   * param[in] function_name	要提供 JS 调用的函数名字
   * param[in] func		函数体
   */
  void RegisterCppFunc(const CefString& function_name, CppFunction func, int browser_id);

  /**
   * 反注册一个持久的 C++ 函数
   * param[in] function_name	要反注册的函数名称
   */
  void UnRegisterCppFunc(const CefString& function_name, int browser_id);

  /**
   * 执行一个已经注册好的 C++ 方法（接受到 JS 端执行请求时被调用）
   * param[in] function_name	要执行的函数名称
   * param[in] params			携带的参数
   * param[in] browser		browser 实例句柄
   * return 返回 true 表示执行成功，返回 false 表示执行失败，函数名可能不存在
   */
  bool ExecuteCppFunc(const CefString& function_name, const CefString& params, int browser_id);

private:
  BrowserRegisteredFunction	browser_registered_function_;	// 保存 C++ 端已经注册好的持久函数列表
};