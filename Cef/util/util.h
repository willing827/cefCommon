#pragma once

#include<cassert> 

#include "include/cef_task.h"
#include "include/base/cef_bind.h"
#include "include/wrapper/cef_closure_task.h"

#define REQUIRE_UI_THREAD()   assert(CefCurrentlyOn(TID_UI));
#define REQUIRE_IO_THREAD()   assert(CefCurrentlyOn(TID_IO));
#define REQUIRE_FILE_THREAD() assert(CefCurrentlyOn(TID_FILE));

#include "Helper/singleton.h"
#include "Helper/message_wnd.h"
#include "Helper/string.h"
#include "Helper/defer.h"
#include "Helper/drag_adapter.h"
#include "Helper/event_observer.h"

#include "memory_dc.h"

class CefTaskWrapper
  :public CefTask
{
public:
  CefTaskWrapper(std::function<void()> const&task)
    :task_(task)
  {

  }

  virtual void Execute()
  {
    if (task_) {
      task_();
    }
  }

private:
  std::function<void()> task_{ nullptr };

  IMPLEMENT_REFCOUNTING(CefTaskWrapper)
};

using CefTaskWrapperPtr = CefRefPtr<CefTaskWrapper>;

inline CefTaskWrapperPtr MakeCefTask(std::function<void()> task)
{
  return new CefTaskWrapper{ task };
}