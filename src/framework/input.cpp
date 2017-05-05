#include "input.h"
#include "../windowing/window.h"
#include "foundation/exception.h"

GF_NAMESPACE_BEGIN

namespace
{
    KeyCode keyCode(WPARAM vkc)
    {
        if (vkc == VK_ESCAPE) return KeyCode::Esc;
        return static_cast<KeyCode>(static_cast<size_t>(KeyCode::A) + vkc - 0x41);
        check(false);
        return KeyCode::A;
    }

    size_t index(KeyCode kc)
    {
        return static_cast<size_t>(kc);
    }

    size_t index(WPARAM vkc)
    {
        return index(keyCode(vkc));
    }

    size_t index(MouseButton mb)
    {
        return static_cast<size_t>(mb);
    }
}

InputDevice inputDevice;

bool InputDevice::keyDown(KeyCode kc) const
{
    return keyStatus_[index(kc)];
}

bool InputDevice::keyPressed(KeyCode kc) const
{
    return keyStatus_[index(kc)] && !preKeyStatus_[index(kc)];
}

bool InputDevice::keyReleased(KeyCode kc) const
{
    return !keyStatus_[index(kc)] && preKeyStatus_[index(kc)];
}

bool InputDevice::buttonDown(MouseButton mb) const
{
    return mouseButtons_[index(mb)];
}

bool InputDevice::buttonPressed(MouseButton mb) const
{
    return mouseButtons_[index(mb)] && !preMouseButtons_[index(mb)];
}

bool InputDevice::buttonReleased(MouseButton mb) const
{
    return !mouseButtons_[index(mb)] && preMouseButtons_[index(mb)];
}

void InputDevice::mousePosition(int* x, int* y) const
{
    POINT pos;
    GetCursorPos(&pos);
    if (x) *x = window_->toClientX(pos.x);
    if (y) *y = window_->toClientY(pos.y);
}

void InputDevice::setMousePosition(int x, int y)
{
    SetCursorPos(window_->toScreenX(x), window_->toScreenY(y));
}

void InputDevice::showMouseCursor(bool b)
{
    if (b)
    {
        while (ShowCursor(TRUE) < 0) {}
    }
    else
    {
        while (ShowCursor(FALSE) >= 0) {}
    }
}

void InputDevice::startup(const std::shared_ptr<Window>& window)
{
    window_ = window;

    keyStatus_.fill(false);
    preKeyStatus_.fill(false);
    mouseButtons_.fill(false);
    preMouseButtons_.fill(false);
}

void InputDevice::shutdown()
{
    window_.reset();
}

void InputDevice::nextFrame()
{
    preKeyStatus_ = keyStatus_;
    preMouseButtons_ = mouseButtons_;
}

void InputDevice::onWinMessage(UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg)
    {
    case WM_SYSKEYDOWN:
    case WM_KEYDOWN:
        keyStatus_[index(wp)] = true;
        break;

    case WM_SYSKEYUP:
    case WM_KEYUP:
        keyStatus_[index(wp)] = false;
        break;

    case WM_LBUTTONDOWN:
        keyStatus_[index(MouseButton::left)] = true;
        break;

    case WM_LBUTTONUP:
        keyStatus_[index(MouseButton::left)] = false;
        break;

    case WM_RBUTTONDOWN:
        keyStatus_[index(MouseButton::right)] = true;
        break;

    case WM_RBUTTONUP:
        keyStatus_[index(MouseButton::right)] = false;
        break;

    default:
        break;
    }
}

GF_NAMESPACE_END