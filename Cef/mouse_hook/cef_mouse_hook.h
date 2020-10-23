#pragma once
#include "util/util.h"

class CefControlBase;
class CefMouseHook
  :public helper_lib::SingletonT<CefMouseHook>
{
  HELPERLIBSINGLETON(CefMouseHook);
public:
  void Init();

  void AddCefControlBase(CefControlBase *control_base);
  void RemoveCefControlBase(CefControlBase *control_base);
protected:
  CefMouseHook();

  ~CefMouseHook();

  static LRESULT CALLBACK MouseProc(int nCode, WPARAM wParam, LPARAM lParam);

private:
  std::list<CefControlBase*> cef_control_base_list_{};
};

