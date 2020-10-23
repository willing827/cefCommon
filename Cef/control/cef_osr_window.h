#pragma once
#include "cef_control_base.h"
#include "manager/cef_manager.h"
#include "mouse_hook/cef_mouse_hook.h"
#include "util/util.h"

#include <CommCtrl.h>

/**
 * ������Ⱦ ͸������
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

  //***��Ҫֱ�ӵ��� ::CloseWindow����ȥ�رմ���,�����browser�޷���ȷ�رյ�����
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
   * @brief ת����갴����Ϣ�� BrowserHost
   * @param[in] uMsg ��Ϣ
   * @param[in] wParam ��Ϣ���Ӳ���
   * @param[in] lParam ��Ϣ���Ӳ���
   * @param[out] bHandled �Ƿ����������Ϣ
   * @return ������Ϣ������
   */
  LRESULT SendButtonDownEvent(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);

  /**
   * @brief ת�����˫����Ϣ�� BrowserHost
   * @param[in] uMsg ��Ϣ
   * @param[in] wParam ��Ϣ���Ӳ���
   * @param[in] lParam ��Ϣ���Ӳ���
   * @param[out] bHandled �Ƿ����������Ϣ
   * @return ������Ϣ������
   */
  LRESULT SendButtonDoubleDownEvent(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);

  /**
   * @brief ת����굯����Ϣ�� BrowserHost
   * @param[in] uMsg ��Ϣ
   * @param[in] wParam ��Ϣ���Ӳ���
   * @param[in] lParam ��Ϣ���Ӳ���
   * @param[out] bHandled �Ƿ����������Ϣ
   * @return ������Ϣ������
   */
  LRESULT SendButtonUpEvent(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);

  /**
   * @brief ת������ƶ���Ϣ�� BrowserHost
   * @param[in] uMsg ��Ϣ
   * @param[in] wParam ��Ϣ���Ӳ���
   * @param[in] lParam ��Ϣ���Ӳ���
   * @param[out] bHandled �Ƿ����������Ϣ
   * @return ������Ϣ������
   */
  LRESULT SendMouseMoveEvent(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);

  /**
   * @brief ת����������Ϣ�� BrowserHost
   * @param[in] uMsg ��Ϣ
   * @param[in] wParam ��Ϣ���Ӳ���
   * @param[in] lParam ��Ϣ���Ӳ���
   * @param[out] bHandled �Ƿ����������Ϣ
   * @return ������Ϣ������
   */
  LRESULT SendMouseWheelEvent(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);

  /**
   * @brief ת������뿪��Ϣ�� BrowserHost
   * @param[in] uMsg ��Ϣ
   * @param[in] wParam ��Ϣ���Ӳ���
   * @param[in] lParam ��Ϣ���Ӳ���
   * @param[out] bHandled �Ƿ����������Ϣ
   * @return ������Ϣ������
   */
  LRESULT SendMouseLeaveEvent(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);

  /**
   * @brief ת�����������Ϣ�� BrowserHost
   * @param[in] uMsg ��Ϣ
   * @param[in] wParam ��Ϣ���Ӳ���
   * @param[in] lParam ��Ϣ���Ӳ���
   * @param[out] bHandled �Ƿ����������Ϣ
   * @return ������Ϣ������
   */
  LRESULT SendKeyEvent(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);

  /**
   * @brief ת�����񽹵���Ϣ�� BrowserHost
   * @param[in] uMsg ��Ϣ
   * @param[in] wParam ��Ϣ���Ӳ���
   * @param[in] lParam ��Ϣ���Ӳ���
   * @param[out] bHandled �Ƿ����������Ϣ
   * @return ������Ϣ������
   */
  LRESULT SendCaptureLostEvent(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);

  /**
   * @brief �ж��Ƿ��а�������
   * @param[in] wparam ��Ϣ���Ӳ���
   * @return ���� true ��ʾ�а������£�false ��ʾû�а�������
   */
  static bool IsKeyDown(WPARAM wparam);

  /**
   * @brief ת����ͨ������Ϣ�� CEF ��ʶ��ļ�����Ϣ
   * @param[in] wparam ��Ϣ���Ӳ���
   * @param[in] lparam ��Ϣ���Ӳ���
   * @return ����ת����Ľ��
   */
  static int GetCefKeyboardModifiers(WPARAM wparam, LPARAM lparam);

  /**
   * @brief ת����ͨ�����Ϣ�� CEF ��ʶ��������Ϣ
   * @param[in] wparam ��Ϣ���Ӳ���
   * @return ����ת����Ľ��
   */
  static int GetCefMouseModifiers(WPARAM wparam);

private:
  MemoryDC mem_dc_;

  bool is_track_{ false };

  HWND tooltip_wnd_{};
  TOOLINFO tooltip_info_;
};