#pragma once
#include "cef_control_base.h"
#include "manager/cef_manager.h"
#include "mouse_hook/cef_mouse_hook.h"
#include "util/util.h"

#include <CommCtrl.h>

/**
 * 离屏渲染 透明窗口
 */

class CefOsrWindow
  :public CefControlBase
  , public helper_lib::DragWindowMoveAdapterT<CefOsrWindow>
{
public:
  CefOsrWindow();

  ~CefOsrWindow();

  HWND CreateWnd(LPCWSTR wnd_title, RECT const&pos, HWND parent_wnd = nullptr);

  HWND GetHWND() const;

  void ShowWindow(bool is_show = true);

  //***不要直接调用 ::CloseWindow方法去关闭窗口,会造成browser无法正确关闭的问题
  void CloseWindow();

  void SetWindowPos(HWND after, RECT rect, UINT flag);
protected:
  virtual void OnClose(bool &handled);

  virtual void OnFinalMessage(HWND hWnd);

  virtual void ReCreateBrowser() override;

  virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
  virtual void OnPaint(CefRefPtr<CefBrowser> browser,
    CefRenderHandler::PaintElementType type,
    const CefRenderHandler::RectList& dirtyRects,
    const std::string* buffer,
    int width,
    int height) OVERRIDE;

  virtual void OnPopupShow(CefRefPtr<CefBrowser> browser, bool show) OVERRIDE;

  virtual void OnPopupSize(CefRefPtr<CefBrowser> browser, const CefRect& rect) OVERRIDE;

  virtual bool OnTooltip(CefRefPtr<CefBrowser> browser, CefString& text) OVERRIDE;
protected:
  static ATOM RegisterWnd();
  static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
  static bool is_register_wnd_;

private:
  /**
   * @brief 转发鼠标按下消息到 BrowserHost
   * @param[in] uMsg 消息
   * @param[in] wParam 消息附加参数
   * @param[in] lParam 消息附加参数
   * @param[out] bHandled 是否继续传递消息
   * @return 返回消息处理结果
   */
  LRESULT SendButtonDownEvent(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);

  /**
   * @brief 转发鼠标双击消息到 BrowserHost
   * @param[in] uMsg 消息
   * @param[in] wParam 消息附加参数
   * @param[in] lParam 消息附加参数
   * @param[out] bHandled 是否继续传递消息
   * @return 返回消息处理结果
   */
  LRESULT SendButtonDoubleDownEvent(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);

  /**
   * @brief 转发鼠标弹起消息到 BrowserHost
   * @param[in] uMsg 消息
   * @param[in] wParam 消息附加参数
   * @param[in] lParam 消息附加参数
   * @param[out] bHandled 是否继续传递消息
   * @return 返回消息处理结果
   */
  LRESULT SendButtonUpEvent(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);

  /**
   * @brief 转发鼠标移动消息到 BrowserHost
   * @param[in] uMsg 消息
   * @param[in] wParam 消息附加参数
   * @param[in] lParam 消息附加参数
   * @param[out] bHandled 是否继续传递消息
   * @return 返回消息处理结果
   */
  LRESULT SendMouseMoveEvent(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);

  /**
   * @brief 转发鼠标滚动消息到 BrowserHost
   * @param[in] uMsg 消息
   * @param[in] wParam 消息附加参数
   * @param[in] lParam 消息附加参数
   * @param[out] bHandled 是否继续传递消息
   * @return 返回消息处理结果
   */
  LRESULT SendMouseWheelEvent(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);

  /**
   * @brief 转发鼠标离开消息到 BrowserHost
   * @param[in] uMsg 消息
   * @param[in] wParam 消息附加参数
   * @param[in] lParam 消息附加参数
   * @param[out] bHandled 是否继续传递消息
   * @return 返回消息处理结果
   */
  LRESULT SendMouseLeaveEvent(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);

  /**
   * @brief 转发键盘相关消息到 BrowserHost
   * @param[in] uMsg 消息
   * @param[in] wParam 消息附加参数
   * @param[in] lParam 消息附加参数
   * @param[out] bHandled 是否继续传递消息
   * @return 返回消息处理结果
   */
  LRESULT SendKeyEvent(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);

  /**
   * @brief 转发捕获焦点消息到 BrowserHost
   * @param[in] uMsg 消息
   * @param[in] wParam 消息附加参数
   * @param[in] lParam 消息附加参数
   * @param[out] bHandled 是否继续传递消息
   * @return 返回消息处理结果
   */
  LRESULT SendCaptureLostEvent(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);

  /**
   * @brief 判断是否有按键按下
   * @param[in] wparam 消息附加参数
   * @return 返回 true 表示有按键按下，false 表示没有按键按下
   */
  static bool IsKeyDown(WPARAM wparam);

  /**
   * @brief 转换普通键盘消息到 CEF 可识别的键盘消息
   * @param[in] wparam 消息附加参数
   * @param[in] lparam 消息附加参数
   * @return 返回转换后的结果
   */
  static int GetCefKeyboardModifiers(WPARAM wparam, LPARAM lparam);

  /**
   * @brief 转换普通鼠标消息到 CEF 可识别的鼠标消息
   * @param[in] wparam 消息附加参数
   * @return 返回转换后的结果
   */
  static int GetCefMouseModifiers(WPARAM wparam);

private:
  MemoryDC mem_dc_;

  bool is_track_{ false };

  HWND tooltip_wnd_{};
  TOOLINFO tooltip_info_;
};