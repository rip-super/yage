#pragma once

#include <yage/window.h>
#include <yage/utils.h>
#include <GLFW/glfw3.h>
#include <unordered_map>

namespace yage
{
    // clang-format off
    enum class Key : int 
    {
        A = GLFW_KEY_A, B = GLFW_KEY_B, C = GLFW_KEY_C, D = GLFW_KEY_D,
        E = GLFW_KEY_E, F = GLFW_KEY_F, G = GLFW_KEY_G, H = GLFW_KEY_H,
        I = GLFW_KEY_I, J = GLFW_KEY_J, K = GLFW_KEY_K, L = GLFW_KEY_L,
        M = GLFW_KEY_M, N = GLFW_KEY_N, O = GLFW_KEY_O, P = GLFW_KEY_P,
        Q = GLFW_KEY_Q, R = GLFW_KEY_R, S = GLFW_KEY_S, T = GLFW_KEY_T,
        U = GLFW_KEY_U, V = GLFW_KEY_V, W = GLFW_KEY_W, X = GLFW_KEY_X,
        Y = GLFW_KEY_Y, Z = GLFW_KEY_Z,

        Num0 = GLFW_KEY_0, Num1 = GLFW_KEY_1, Num2 = GLFW_KEY_2,
        Num3 = GLFW_KEY_3, Num4 = GLFW_KEY_4, Num5 = GLFW_KEY_5,
        Num6 = GLFW_KEY_6, Num7 = GLFW_KEY_7, Num8 = GLFW_KEY_8,
        Num9 = GLFW_KEY_9,

        F1  = GLFW_KEY_F1,  F2  = GLFW_KEY_F2,  F3  = GLFW_KEY_F3,
        F4  = GLFW_KEY_F4,  F5  = GLFW_KEY_F5,  F6  = GLFW_KEY_F6,
        F7  = GLFW_KEY_F7,  F8  = GLFW_KEY_F8,  F9  = GLFW_KEY_F9,
        F10 = GLFW_KEY_F10, F11 = GLFW_KEY_F11, F12 = GLFW_KEY_F12,

        Up    = GLFW_KEY_UP,    Down  = GLFW_KEY_DOWN,
        Left  = GLFW_KEY_LEFT,  Right = GLFW_KEY_RIGHT,
        Home  = GLFW_KEY_HOME,  End   = GLFW_KEY_END,
        PageUp = GLFW_KEY_PAGE_UP, PageDown = GLFW_KEY_PAGE_DOWN,

        Space     = GLFW_KEY_SPACE,
        Enter     = GLFW_KEY_ENTER,
        Escape    = GLFW_KEY_ESCAPE,
        Tab       = GLFW_KEY_TAB,
        Backspace = GLFW_KEY_BACKSPACE,
        Delete    = GLFW_KEY_DELETE,
        Insert    = GLFW_KEY_INSERT,
        LShift    = GLFW_KEY_LEFT_SHIFT,
        RShift    = GLFW_KEY_RIGHT_SHIFT,
        LCtrl     = GLFW_KEY_LEFT_CONTROL,
        RCtrl     = GLFW_KEY_RIGHT_CONTROL,
        LAlt      = GLFW_KEY_LEFT_ALT,
        RAlt      = GLFW_KEY_RIGHT_ALT,
        LSuper    = GLFW_KEY_LEFT_SUPER,
        RSuper    = GLFW_KEY_RIGHT_SUPER,
        CapsLock  = GLFW_KEY_CAPS_LOCK,

        KP0 = GLFW_KEY_KP_0, KP1 = GLFW_KEY_KP_1, KP2 = GLFW_KEY_KP_2,
        KP3 = GLFW_KEY_KP_3, KP4 = GLFW_KEY_KP_4, KP5 = GLFW_KEY_KP_5,
        KP6 = GLFW_KEY_KP_6, KP7 = GLFW_KEY_KP_7, KP8 = GLFW_KEY_KP_8,
        KP9 = GLFW_KEY_KP_9,
        KPAdd      = GLFW_KEY_KP_ADD,
        KPSubtract = GLFW_KEY_KP_SUBTRACT,
        KPMultiply = GLFW_KEY_KP_MULTIPLY,
        KPDivide   = GLFW_KEY_KP_DIVIDE,
        KPEnter    = GLFW_KEY_KP_ENTER,
        KPDecimal  = GLFW_KEY_KP_DECIMAL,

        Apostrophe  = GLFW_KEY_APOSTROPHE,
        Comma       = GLFW_KEY_COMMA,
        Minus       = GLFW_KEY_MINUS,
        Period      = GLFW_KEY_PERIOD,
        Slash       = GLFW_KEY_SLASH,
        Semicolon   = GLFW_KEY_SEMICOLON,
        Equal       = GLFW_KEY_EQUAL,
        LBracket    = GLFW_KEY_LEFT_BRACKET,
        RBracket    = GLFW_KEY_RIGHT_BRACKET,
        Backslash   = GLFW_KEY_BACKSLASH,
        GraveAccent = GLFW_KEY_GRAVE_ACCENT,
    };
    // clang-format on

    enum class MouseButton : int
    {
        Left = GLFW_MOUSE_BUTTON_LEFT,
        Right = GLFW_MOUSE_BUTTON_RIGHT,
        Middle = GLFW_MOUSE_BUTTON_MIDDLE,
    };

    class Input
    {
    public:
        explicit Input(const Window &window);
        ~Input();

        bool KeyDown(Key key) const;
        bool KeyPressed(Key key) const;
        bool KeyReleased(Key key) const;

        bool MouseDown(MouseButton button) const;
        bool MousePressed(MouseButton button) const;
        bool MouseReleased(MouseButton button) const;

        glm::vec2 MousePos() const;
        glm::vec2 MouseDelta() const;
        float ScrollDelta() const;

    private:
        friend class Window;
        void Snapshot();

        static void KeyCallback(GLFWwindow *handle, int key, int scancode, int action, int mods);
        static void CursorCallback(GLFWwindow *handle, double x, double y);
        static void MouseButtonCallback(GLFWwindow *handle, int button, int action, int mods);
        static void ScrollCallback(GLFWwindow *handle, double dx, double dy);

        std::unordered_map<int, bool> curr_keys, prev_keys;
        std::unordered_map<int, bool> curr_mouse, prev_mouse;
        glm::vec2 mouse_pos{}, prev_mouse_pos{};
        float scroll_delta = 0.0f;
    };
}