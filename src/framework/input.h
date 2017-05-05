#ifndef GAMEFRIENDS_INPUT_H
#define GAMEFRIENDS_INPUT_H

#include "../windowing/windowsinc.h"
#include "foundation/prerequest.h"
#include <array>
#include <vector>

GF_NAMESPACE_BEGIN

class Window;

enum class KeyCode
{
    Esc,
    A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z
};

enum class MouseButton
{
    left, right
};

constexpr size_t NUM_KEY_CODES = static_cast<size_t>(KeyCode::Z) + 1;
constexpr size_t NUM_MOUSE_BUTTONS = static_cast<size_t>(MouseButton::right) + 1;

class InputDevice
{
private:
    std::shared_ptr<Window> window_;

    std::array<bool, NUM_KEY_CODES> keyStatus_;
    std::array<bool, NUM_KEY_CODES> preKeyStatus_;

    std::array<bool, NUM_MOUSE_BUTTONS> mouseButtons_;
    std::array<bool, NUM_MOUSE_BUTTONS> preMouseButtons_;

public:
    bool keyDown(KeyCode kc) const;
    bool keyPressed(KeyCode kc) const;
    bool keyReleased(KeyCode kc) const;

    bool buttonDown(MouseButton mb) const;
    bool buttonPressed(MouseButton mb) const;
    bool buttonReleased(MouseButton mb) const;

    void mousePosition(int* x, int* y) const;
    void setMousePosition(int x, int y);
    void showMouseCursor(bool b);

    GF_NOAPI void startup(const std::shared_ptr<Window>& window);
    GF_NOAPI void shutdown();

    GF_NOAPI void nextFrame();
    GF_NOAPI void onWinMessage(UINT msg, WPARAM wp, LPARAM lp);
};

extern InputDevice inputDevice;

GF_NAMESPACE_END

#endif