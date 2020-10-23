// CefDemo.cpp : 定义应用程序的入口点。
//

#include "framework.h"
#include "CefDemo.h"

#define MAX_LOADSTRING 100

// 全局变量:
HINSTANCE hInst;                                // 当前实例
WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名

// 此代码模块中包含的函数的前向声明:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

CefChildWnd* child_wnd;
CefOsrWindow osr_wnd;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
  _In_opt_ HINSTANCE hPrevInstance,
  _In_ LPWSTR    lpCmdLine,
  _In_ int       nCmdShow)
{
  UNREFERENCED_PARAMETER(hPrevInstance);
  UNREFERENCED_PARAMETER(lpCmdLine);

  //CefManager::GetInstance()->AddCefDllToPath(LR"(C:\Users\swiftlc\Desktop\project\CefLib\cef_bundle\cef)");
  CefManager::GetInstance()->AddCefDllToPath();

  CefSettings setting;

  CefManager::GetInstance()->Initialize(L"D:\\a", setting);

  // 初始化全局字符串
  LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
  LoadStringW(hInstance, IDC_CEFDEMO, szWindowClass, MAX_LOADSTRING);
  MyRegisterClass(hInstance);

  // 执行应用程序初始化:
  if (!InitInstance(hInstance, nCmdShow))
  {
    return FALSE;
  }

  HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_CEFDEMO));

  MSG msg;

  // 主消息循环:
  while (GetMessage(&msg, nullptr, 0, 0))
  {
    if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
    {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
  }

  if (child_wnd) {
    delete child_wnd;
  }

  CefManager::GetInstance()->UnInitialize();

  return (int)msg.wParam;
}



//
//  函数: MyRegisterClass()
//
//  目标: 注册窗口类。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
  WNDCLASSEXW wcex;

  wcex.cbSize = sizeof(WNDCLASSEX);

  wcex.style = CS_HREDRAW | CS_VREDRAW;
  wcex.lpfnWndProc = WndProc;
  wcex.cbClsExtra = 0;
  wcex.cbWndExtra = 0;
  wcex.hInstance = hInstance;
  wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CEFDEMO));
  wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
  wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
  wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_CEFDEMO);
  wcex.lpszClassName = szWindowClass;
  wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

  return RegisterClassExW(&wcex);
}

//
//   函数: InitInstance(HINSTANCE, int)
//
//   目标: 保存实例句柄并创建主窗口
//
//   注释:
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//



BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
  hInst = hInstance; // 将实例句柄存储在全局变量中

  HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
    CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

  //创建cef子窗口
  RECT rect;
  ::GetClientRect(hWnd, &rect);
  child_wnd = new CefChildWnd(hWnd, rect);

  child_wnd->LoadURL(LR"(www.baidu.com)");

  osr_wnd.CreateWnd(L"", { 200,200,1000,800 }, hWnd);

  osr_wnd.LoadURL(LR"(www.baidu.com)");

  osr_wnd.cb_drag_start_ = [=] {
    osr_wnd.SetEnterDragMode(false); 
    return true; 
  };

  osr_wnd.AttachDevTools();

  ::ShowWindow(osr_wnd.GetHWND(), SW_SHOW);

  if (!hWnd)
  {
    return FALSE;
  }

  ShowWindow(hWnd, nCmdShow);
  UpdateWindow(hWnd);

  return TRUE;
}

//
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目标: 处理主窗口的消息。
//
//  WM_COMMAND  - 处理应用程序菜单
//  WM_PAINT    - 绘制主窗口
//  WM_DESTROY  - 发送退出消息并返回
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch (message)
  {
    case WM_COMMAND:
    {
      int wmId = LOWORD(wParam);
      // 分析菜单选择:
      switch (wmId)
      {
        case IDM_EXIT:
          DestroyWindow(hWnd);
          break;
        case ID_INVOKEJS:
          /*child_wnd->CallJSFunction(L"jsFunctionName", LR"({"key":"value"})");*/
          osr_wnd.CloseWindow();
          break;
        default:
          return DefWindowProc(hWnd, message, wParam, lParam);
      }
    }
    break;
    case WM_PAINT:
    {
      PAINTSTRUCT ps;
      HDC hdc = BeginPaint(hWnd, &ps);
      // TODO: 在此处添加使用 hdc 的任何绘图代码...
      EndPaint(hWnd, &ps);
    }
    break;
    case WM_SIZE:
    {
      if (wParam != SIZE_MINIMIZED) {
        RECT rect{};
        ::GetClientRect(hWnd, &rect);
        ::SetWindowPos(child_wnd->GetCefHandle(), NULL, 0, 0, rect.right - rect.left, rect.bottom - rect.top, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
      }
    }
    break;
    case WM_DESTROY:
      PostQuitMessage(0);
      break;
    default:
      return DefWindowProc(hWnd, message, wParam, lParam);
  }
  return 0;
}