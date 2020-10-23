#include "cef_osr_window.h"
#include <windowsx.h>
#pragma comment(lib, "comctl32.lib")

namespace
{
    LPCWSTR osr_wnd_class_name = L"CefOsrTransparentWndClassName";

    void SetUserDataPtr(HWND hWnd, void* ptr) {
        ::SetLastError(ERROR_SUCCESS);
        LONG_PTR result =
            ::SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(ptr));
        CHECK(result != 0 || GetLastError() == ERROR_SUCCESS);
    }

    // Return the window's user data pointer.
    template <typename T>
    T GetUserDataPtr(HWND hWnd) {
        return reinterpret_cast<T>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
    }
}

CefOsrWindow::CefOsrWindow()
{

}

CefOsrWindow::~CefOsrWindow()
{

}

HWND CefOsrWindow::CreateWnd(LPCWSTR wnd_title, RECT const&pos, HWND parent_wnd/* = nullptr*/)
{
    if (!is_register_wnd_) {
        is_register_wnd_ = true;
        RegisterWnd();
    }

    if (browser_handler_.get())return browser_handler_->GetHostWindow();

    browser_handler_ = new BrowserHandler;

    browser_handler_->SetIsVisible(false);

    HWND hWnd = CreateWindowEx(WS_EX_LAYERED, osr_wnd_class_name, wnd_title, WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
        pos.left, pos.top, pos.right - pos.left, pos.bottom - pos.top, parent_wnd, nullptr, ::GetModuleHandle(NULL), this);

    CHECK(hWnd);

    if (!hWnd) return NULL;

    //set in wnd_proc -
    browser_handler_->SetHostWindow(hWnd);
    browser_handler_->SetHandlerDelegate(this);

    //CefMouseHook::Instance()->AddCefControlBase(this);

    js_bridge_.reset(new CefJSBridge);

    ReCreateBrowser();

    return hWnd;
}

HWND CefOsrWindow::GetHWND() const
{
    return GetHostWindow();
}

void CefOsrWindow::ShowWindow(bool is_show /*= true*/)
{
    ::ShowWindow(GetHWND(), is_show ? SW_SHOW : SW_HIDE);
}

void CefOsrWindow::CloseWindow()
{
    ::SendMessage(GetHWND(), WM_CLOSE, 0, 0);
}

void CefOsrWindow::SetWindowPos(HWND after, RECT rect, UINT flag)
{
    ::SetWindowPos(GetHWND(), after, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, flag);
}

void CefOsrWindow::OnClose(bool &handled)
{
    HWND hWnd = GetHWND();
    CloseBrowser();
    ::DestroyWindow(hWnd);
}

void CefOsrWindow::OnFinalMessage(HWND hWnd)
{

}

void CefOsrWindow::ReCreateBrowser()
{
    if (!browser_handler_->GetBrowser().get()) {
        // 使用无窗模式，离屏渲染
        CefWindowInfo window_info;
        window_info.SetAsWindowless(GetHostWindow());
        CefBrowserSettings browser_settings;
        browser_settings.windowless_frame_rate = 60;

        //browser_settings.background_color = CefColorSetARGB(255, 255, 255, 255);
        //browser_settings.file_access_from_file_urls = STATE_ENABLED;
        //browser_settings.universal_access_from_file_urls = STATE_ENABLED;
        CefBrowserHost::CreateBrowser(window_info, browser_handler_, L"", browser_settings, NULL);
    }
}

LRESULT CefOsrWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    bool bHandled = false;

    HandleDragMessage(uMsg, wParam, lParam);

    if (browser_handler_) {
        switch (uMsg)
        {
            case WM_MOUSEMOVE:
            {
                if (!is_track_) {
                    TRACKMOUSEEVENT tme;
                    tme.cbSize = sizeof(tme);
                    tme.hwndTrack = GetHostWindow();
                    tme.dwFlags = TME_LEAVE;
                    tme.dwHoverTime = 1;
                    is_track_ = ::TrackMouseEvent(&tme);
                }
                SendMouseMoveEvent(uMsg, wParam, lParam, bHandled);
            }
            break;
            case WM_SETCURSOR:
            {
                /*return FALSE;*/
            }
            break;
            case WM_LBUTTONDOWN:
            case WM_MBUTTONDOWN:
            case WM_RBUTTONDOWN:
            {
                SendButtonDownEvent(uMsg, wParam, lParam, bHandled);
            }
            break;

            case WM_LBUTTONUP:
            case WM_MBUTTONUP:
            case WM_RBUTTONUP:
            {
                SendButtonUpEvent(uMsg, wParam, lParam, bHandled);
            }
            break;

            case WM_SYSCHAR:
            case WM_SYSKEYDOWN:
            case WM_SYSKEYUP:
            case WM_KEYDOWN:
            case WM_KEYUP:
            case WM_CHAR:
            {
                SendKeyEvent(uMsg, wParam, lParam, bHandled);
            }
            break;

            case WM_LBUTTONDBLCLK:
            case WM_MBUTTONDBLCLK:
            case WM_RBUTTONDBLCLK:
            {
                SendButtonDoubleDownEvent(uMsg, wParam, lParam, bHandled);
            }
            break;

            case WM_CAPTURECHANGED:
            case WM_CANCELMODE:
            {
                SendCaptureLostEvent(uMsg, wParam, lParam, bHandled);
            }
            break;

            case WM_MOUSEWHEEL:
            {
                SendMouseWheelEvent(uMsg, wParam, lParam, bHandled);
            }
            break;

            case WM_MOUSELEAVE:
            {
                is_track_ = false;
                SendMouseLeaveEvent(uMsg, wParam, lParam, bHandled);
            }
            break;

            case WM_SIZE:
            {
                RECT rc{};
                ::GetClientRect(GetHostWindow(), &rc);
                browser_handler_->SetViewRect(rc);
            }
            break;

            case WM_SHOWWINDOW:
            {
                if (browser_handler_.get()) {
                    browser_handler_->SetIsVisible(!!(BOOL)wParam);
                }
            }
            break;
            case WM_CLOSE:
            {
                bool handled = false;
                OnClose(handled);
                if (handled) return FALSE;
            }
            break;
            default:
                break;
        }
    }

    return ::DefWindowProc(GetHostWindow(), uMsg, wParam, lParam);
}

void CefOsrWindow::OnPaint(
    CefRefPtr<CefBrowser> browser,
    CefRenderHandler::PaintElementType type,
    const CefRenderHandler::RectList& dirtyRects,
    const std::string* buffer,
    int width,
    int height)
{
    if (type == PET_VIEW) {
        if (mem_dc_.GetWidth() != width || mem_dc_.GetHeight() != height)
            mem_dc_.Init(::GetDC(GetHWND()), width, height);

        LPBYTE dest = (LPBYTE)mem_dc_.GetBits();
        if (dest) {
            memcpy(dest, (char*)buffer->c_str(), height * width * 4);
        }

        BLENDFUNCTION blendFunc32bpp;
        blendFunc32bpp.AlphaFormat = AC_SRC_ALPHA;
        blendFunc32bpp.BlendFlags = 0;
        blendFunc32bpp.BlendOp = AC_SRC_OVER;
        blendFunc32bpp.SourceConstantAlpha = 0xFF;

        SIZE sz{ width,height };
        POINT pt{};

        ::UpdateLayeredWindow(GetHWND(), ::GetDC(GetHWND()), NULL, &sz, mem_dc_.GetDC(), &pt, 0, &blendFunc32bpp, ULW_ALPHA);
    }
}

void CefOsrWindow::OnPopupShow(CefRefPtr<CefBrowser> browser, bool show)
{

}

void CefOsrWindow::OnPopupSize(CefRefPtr<CefBrowser> browser, const CefRect& rect)
{

}

bool CefOsrWindow::OnTooltip(CefRefPtr<CefBrowser> browser, CefString& text)
{
    std::wstring info_now = text.ToWString();
    HWND hwnd = GetHWND();
    if (!tooltip_wnd_) {
        ::ZeroMemory(&tooltip_info_, sizeof(TOOLINFO));
        tooltip_info_.cbSize = TTTOOLINFOA_V2_SIZE;
        tooltip_info_.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
        tooltip_info_.hwnd = hwnd;
        tooltip_info_.uId = (UINT_PTR)hwnd;
        tooltip_info_.hinst = nullptr;
        tooltip_info_.lpszText = const_cast<LPTSTR>((LPCTSTR)info_now.c_str());
        tooltip_wnd_ =
            ::CreateWindowEx(
                WS_EX_TRANSPARENT,
                TOOLTIPS_CLASS,
                NULL,
                WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP,
                CW_USEDEFAULT, CW_USEDEFAULT,
                CW_USEDEFAULT, CW_USEDEFAULT,
                hwnd, NULL, nullptr, NULL);
        //::SendMessage(m_hwndTooltip, TTM_SETMAXTIPWIDTH, 0, 500);
        ::SendMessage(tooltip_wnd_, TTM_ADDTOOL, 0, (LPARAM)&tooltip_info_);
    }

    if (info_now.empty()) {
        ::SendMessage(tooltip_wnd_, TTM_DELTOOL, 0, (LPARAM)&tooltip_info_);
        ::DestroyWindow(tooltip_wnd_);
        tooltip_wnd_ = nullptr;
    }

    return false;
}

ATOM CefOsrWindow::RegisterWnd()
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wcex.lpfnWndProc = &CefOsrWindow::WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = ::GetModuleHandle(NULL);
    wcex.hIcon = NULL;
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = osr_wnd_class_name;
    wcex.hIconSm = NULL;

    return RegisterClassExW(&wcex);
}

LRESULT CALLBACK CefOsrWindow::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    CefOsrWindow* pThis = NULL;
    if (message == WM_NCCREATE) {
        LPCREATESTRUCT lpcs = reinterpret_cast<LPCREATESTRUCT>(lParam);
        pThis = static_cast<CefOsrWindow*>(lpcs->lpCreateParams);
        ::SetProp(hWnd, L"WndX", (HANDLE)pThis);
        pThis->browser_handler_->SetHostWindow(hWnd);
    } else {
        pThis = reinterpret_cast<CefOsrWindow*>(::GetProp(hWnd, L"WndX"));
        if (message == WM_NCDESTROY && pThis != NULL) {
            LRESULT lRes = ::DefWindowProc(hWnd, message, wParam, lParam);
            ::SetProp(hWnd, L"WndX", NULL);
            pThis->OnFinalMessage(hWnd);
            return lRes;
        }
    }
    if (pThis != NULL) {
        return pThis->HandleMessage(message, wParam, lParam);
    }

    return ::DefWindowProc(hWnd, message, wParam, lParam);
}

bool CefOsrWindow::is_register_wnd_ = false;




//////////////////////////////////////////////////////////////////////////////////
LRESULT CefOsrWindow::SendButtonDownEvent(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
    CefRefPtr<CefBrowserHost> host = browser_handler_->GetBrowserHost();

    POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };

    CefMouseEvent mouse_event;
    mouse_event.x = pt.x;
    mouse_event.y = pt.y;
    mouse_event.modifiers = GetCefMouseModifiers(wParam);

    CefBrowserHost::MouseButtonType btnType =
        (uMsg == WM_LBUTTONDOWN ? MBT_LEFT : (
            uMsg == WM_RBUTTONDOWN ? MBT_RIGHT : MBT_MIDDLE));

    host->SendMouseClickEvent(mouse_event, btnType, false, 1);

    return 0;
}

LRESULT CefOsrWindow::SendButtonDoubleDownEvent(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
    CefRefPtr<CefBrowserHost> host = browser_handler_->GetBrowserHost();

    POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };

    CefMouseEvent mouse_event;
    mouse_event.x = pt.x;
    mouse_event.y = pt.y;
    mouse_event.modifiers = GetCefMouseModifiers(wParam);

    CefBrowserHost::MouseButtonType btnType = MBT_LEFT;

    host->SendMouseClickEvent(mouse_event, btnType, false, 2);

    bHandled = true;
    return 0;
}

LRESULT CefOsrWindow::SendButtonUpEvent(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
    CefRefPtr<CefBrowserHost> host = browser_handler_->GetBrowserHost();

    POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };

    CefMouseEvent mouse_event;

    mouse_event.x = pt.x;
    mouse_event.y = pt.y;


    mouse_event.modifiers = GetCefMouseModifiers(wParam);

    CefBrowserHost::MouseButtonType btnType =
        (uMsg == WM_LBUTTONUP ? MBT_LEFT : (
            uMsg == WM_RBUTTONUP ? MBT_RIGHT : MBT_MIDDLE));

    host->SendMouseClickEvent(mouse_event, btnType, true, 1);

    return 0;
}

LRESULT CefOsrWindow::SendMouseMoveEvent(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
    CefRefPtr<CefBrowserHost> host = browser_handler_->GetBrowserHost();

    if (!host) return FALSE;

    POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };

    CefMouseEvent mouse_event;
    mouse_event.x = pt.x;
    mouse_event.y = pt.y;
    mouse_event.modifiers = GetCefMouseModifiers(wParam);
    host->SendMouseMoveEvent(mouse_event, false);

    return 0;
}

LRESULT CefOsrWindow::SendMouseWheelEvent(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
    CefRefPtr<CefBrowserHost> host = browser_handler_->GetBrowserHost();

    POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
    HWND scrolled_wnd = ::WindowFromPoint(pt);
    if (scrolled_wnd != GetHostWindow())
        return 0;

    ScreenToClient(GetHostWindow(), &pt);

    int delta = GET_WHEEL_DELTA_WPARAM(wParam);

    CefMouseEvent mouse_event;
    mouse_event.x = pt.x;
    mouse_event.y = pt.y;
    mouse_event.modifiers = GetCefMouseModifiers(wParam);
    host->SendMouseWheelEvent(mouse_event, IsKeyDown(VK_SHIFT) ? delta : 0, !IsKeyDown(VK_SHIFT) ? delta : 0);

    bHandled = true;
    return 0;
}

LRESULT CefOsrWindow::SendMouseLeaveEvent(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
    CefRefPtr<CefBrowserHost> host = browser_handler_->GetBrowserHost();

    POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };

    CefMouseEvent mouse_event;
    mouse_event.x = pt.x;
    mouse_event.y = pt.y;
    mouse_event.modifiers = GetCefMouseModifiers(wParam);

    host->SendMouseMoveEvent(mouse_event, true);

    bHandled = true;
    return 0;
}

LRESULT CefOsrWindow::SendKeyEvent(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
    CefRefPtr<CefBrowserHost> host = browser_handler_->GetBrowserHost();

    CefKeyEvent event;
    event.windows_key_code = wParam;
    event.native_key_code = lParam;
    event.is_system_key = uMsg == WM_SYSCHAR || uMsg == WM_SYSKEYDOWN || uMsg == WM_SYSKEYUP;

    if (uMsg == WM_KEYDOWN || uMsg == WM_SYSKEYDOWN)
        event.type = KEYEVENT_RAWKEYDOWN;
    else if (uMsg == WM_KEYUP || uMsg == WM_SYSKEYUP)
        event.type = KEYEVENT_KEYUP;
    else
        event.type = KEYEVENT_CHAR;
    event.modifiers = GetCefKeyboardModifiers(wParam, lParam);

    host->SendKeyEvent(event);

    bHandled = true;
    return 0;
}

LRESULT CefOsrWindow::SendCaptureLostEvent(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
    CefRefPtr<CefBrowserHost> host = browser_handler_->GetBrowserHost();

    host->SendCaptureLostEvent();
    bHandled = true;
    return 0;
}

bool CefOsrWindow::IsKeyDown(WPARAM wparam)
{
    return (GetKeyState(wparam) & 0x8000) != 0;
}

int CefOsrWindow::GetCefMouseModifiers(WPARAM wparam)
{
    int modifiers = 0;
    if (wparam & MK_CONTROL)
        modifiers |= EVENTFLAG_CONTROL_DOWN;
    if (wparam & MK_SHIFT)
        modifiers |= EVENTFLAG_SHIFT_DOWN;
    if (IsKeyDown(VK_MENU))
        modifiers |= EVENTFLAG_ALT_DOWN;
    if (wparam & MK_LBUTTON)
        modifiers |= EVENTFLAG_LEFT_MOUSE_BUTTON;
    if (wparam & MK_MBUTTON)
        modifiers |= EVENTFLAG_MIDDLE_MOUSE_BUTTON;
    if (wparam & MK_RBUTTON)
        modifiers |= EVENTFLAG_RIGHT_MOUSE_BUTTON;

    // Low bit set from GetKeyState indicates "toggled".
    if (::GetKeyState(VK_NUMLOCK) & 1)
        modifiers |= EVENTFLAG_NUM_LOCK_ON;
    if (::GetKeyState(VK_CAPITAL) & 1)
        modifiers |= EVENTFLAG_CAPS_LOCK_ON;
    return modifiers;
}

int CefOsrWindow::GetCefKeyboardModifiers(WPARAM wparam, LPARAM lparam)
{
    int modifiers = 0;
    if (IsKeyDown(VK_SHIFT))
        modifiers |= EVENTFLAG_SHIFT_DOWN;
    if (IsKeyDown(VK_CONTROL))
        modifiers |= EVENTFLAG_CONTROL_DOWN;
    if (IsKeyDown(VK_MENU))
        modifiers |= EVENTFLAG_ALT_DOWN;

    // Low bit set from GetKeyState indicates "toggled".
    if (::GetKeyState(VK_NUMLOCK) & 1)
        modifiers |= EVENTFLAG_NUM_LOCK_ON;
    if (::GetKeyState(VK_CAPITAL) & 1)
        modifiers |= EVENTFLAG_CAPS_LOCK_ON;

    switch (wparam) {
        case VK_RETURN:
            if ((lparam >> 16) & KF_EXTENDED)
                modifiers |= EVENTFLAG_IS_KEY_PAD;
            break;
        case VK_INSERT:
        case VK_DELETE:
        case VK_HOME:
        case VK_END:
        case VK_PRIOR:
        case VK_NEXT:
        case VK_UP:
        case VK_DOWN:
        case VK_LEFT:
        case VK_RIGHT:
            if (!((lparam >> 16) & KF_EXTENDED))
                modifiers |= EVENTFLAG_IS_KEY_PAD;
            break;
        case VK_NUMLOCK:
        case VK_NUMPAD0:
        case VK_NUMPAD1:
        case VK_NUMPAD2:
        case VK_NUMPAD3:
        case VK_NUMPAD4:
        case VK_NUMPAD5:
        case VK_NUMPAD6:
        case VK_NUMPAD7:
        case VK_NUMPAD8:
        case VK_NUMPAD9:
        case VK_DIVIDE:
        case VK_MULTIPLY:
        case VK_SUBTRACT:
        case VK_ADD:
        case VK_DECIMAL:
        case VK_CLEAR:
            modifiers |= EVENTFLAG_IS_KEY_PAD;
            break;
        case VK_SHIFT:
            if (IsKeyDown(VK_LSHIFT))
                modifiers |= EVENTFLAG_IS_LEFT;
            else if (IsKeyDown(VK_RSHIFT))
                modifiers |= EVENTFLAG_IS_RIGHT;
            break;
        case VK_CONTROL:
            if (IsKeyDown(VK_LCONTROL))
                modifiers |= EVENTFLAG_IS_LEFT;
            else if (IsKeyDown(VK_RCONTROL))
                modifiers |= EVENTFLAG_IS_RIGHT;
            break;
        case VK_MENU:
            if (IsKeyDown(VK_LMENU))
                modifiers |= EVENTFLAG_IS_LEFT;
            else if (IsKeyDown(VK_RMENU))
                modifiers |= EVENTFLAG_IS_RIGHT;
            break;
        case VK_LWIN:
            modifiers |= EVENTFLAG_IS_LEFT;
            break;
        case VK_RWIN:
            modifiers |= EVENTFLAG_IS_RIGHT;
            break;
    }
    return modifiers;
}
