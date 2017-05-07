#include "window.h"
#include "foundation/exception.h"

GF_NAMESPACE_BEGIN

Window::Window(HWND handle, size_t w, size_t h)
    : handle_(handle)
    , clientWidth_(w)
    , clientHeight_(h)
{
}

HWND Window::handle()
{
    return handle_;
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
    ClientToScreen(handle_, &p);
    return p.x;
}

size_t Window::toScreenY(size_t clientY) const
{
    POINT p;
    p.x = 0;
    p.y = clientY;
    ClientToScreen(handle_, &p);
    return p.y;
}

size_t Window::toClientX(size_t screenX) const
{
    POINT p;
    p.x = screenX;
    p.y = 0;
    ScreenToClient(handle_, &p);
    return p.x;
}

size_t Window::toClientY(size_t screenY) const
{
    POINT p;
    p.x = 0;
    p.y = screenY;
    ScreenToClient(handle_, &p);
    return p.y;
}

void Window::show(bool b)
{
    ShowWindow(handle_, b ? SW_SHOW : SW_HIDE);
}

GF_NAMESPACE_END