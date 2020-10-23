#pragma once

namespace cef_render
{
  static const char kFocusedNodeChangedMessage[] = "FocusedNodeChanged";		// web页面中获取焦点的元素改变
  static const char kCallCppFunctionMessage[] = "CallCppFunction";		// web调用C++接口接口的通知
  static const char kCallJsFunctionMessage[] = "CallJsFunction";			// C++ 调用 JavaScript 通知
}