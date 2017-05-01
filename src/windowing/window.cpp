#include "window.h"
#include "../foundation/exception.h"

GF_NAMESPACE_BEGIN

Window::Window(size_t w, size_t h, const string_t& title, WNDPROC proc)
    : handle_()
    , clientWidth_(w)
    , clientHeight_(h)
{
    const auto inst = GetModuleHandle(NULL);

    WNDCLASSEX wc;
    wc.cbSize = sizeof(wc);
    wc.style = 0;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = inst;
    wc.lpfnWndProc = proc;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hIconSm = NULL;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
    wc.lpszMenuName = NULL;
    wc.lpszClassName = title.c_str();

    enforce<WindowsException>(RegisterClassEx(&wc),
        "Failed to register the window class.");

    const auto style = WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX;
    RECT rect = { 0, 0, static_cast<LONG>(w), static_cast<LONG>(h) };

    AdjustWindowRect(&rect, style, false);

    const auto windowWidth = rect.right - rect.left;
    const auto windowHeight = rect.bottom - rect.top;

    handle_.reset(enforce<WindowsException>(
        CreateWindow(wc.lpszClassName, wc.lpszClassName, style, CW_USEDEFAULT, CW_USEDEFAULT,
            windowWidth, windowHeight, NULL, NULL, inst, NULL),
        "Failed to create the window."));
}

HWND Window::handle()
{
    return handle_.get();
}

size_t Window::clientWidth() const
{
    return clientWidth_;
}

size_t Window::clientHeight() const
{
    return clientHeight_;
}

size_t Window::toScreenX(size_t clientX) const
{
    POINT p;
    p.x = clientX;
    p.y = 0;
    ClientToScreen(handle_.get(), &p);
    return p.x;
}

size_t Window::toScreenY(size_t clientY) const
{
    POINT p;
    p.x = 0;
    p.y = clientY;
    ClientToScreen(handle_.get(), &p);
    return p.y;
}

size_t Window::toClientX(size_t screenX) const
{
    POINT p;
    p.x = screenX;
    p.y = 0;
    ScreenToClient(handle_.get(), &p);
    return p.x;
}

size_t Window::toClientY(size_t screenY) const
{
    POINT p;
    p.x = 0;
    p.y = screenY;
    ScreenToClient(handle_.get(), &p);
    return p.y;
}

void Window::show(bool b)
{
    ShowWindow(handle_.get(), b ? SW_SHOW : SW_HIDE);
}

GF_NAMESPACE_END