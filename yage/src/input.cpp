#include <yage/input.h>

static std::unordered_map<GLFWwindow *, yage::Input *> instances;

namespace yage
{
    Input::Input(const Window &window)
    {
        GLFWwindow *handle = window.GetHandle();

        instances[handle] = this;

        glfwSetKeyCallback(handle, KeyCallback);
        glfwSetCursorPosCallback(handle, CursorCallback);
        glfwSetMouseButtonCallback(handle, MouseButtonCallback);
        glfwSetScrollCallback(handle, ScrollCallback);

        double x, y;
        glfwGetCursorPos(handle, &x, &y);
        mouse_pos = prev_mouse_pos = {(float)x, (float)y};
    }

    Input::~Input()
    {
        for (auto it = instances.begin(); it != instances.end(); ++it)
        {
            if (it->second == this)
            {
                instances.erase(it);
                break;
            }
        }
    }

    void Input::Snapshot()
    {
        prev_keys = curr_keys;
        prev_mouse = curr_mouse;
        prev_mouse_pos = mouse_pos;
        scroll_delta = 0.0f;
    }

    bool Input::KeyDown(Key key) const
    {
        auto it = curr_keys.find((int)key);
        return it != curr_keys.end() && it->second;
    }

    bool Input::KeyPressed(Key key) const
    {
        auto it_curr = curr_keys.find((int)key);
        auto it_prev = prev_keys.find((int)key);
        bool curr = it_curr != curr_keys.end() && it_curr->second;
        bool prev = it_prev != prev_keys.end() && it_prev->second;
        return curr && !prev;
    }

    bool Input::KeyReleased(Key key) const
    {
        auto it_curr = curr_keys.find((int)key);
        auto it_prev = prev_keys.find((int)key);
        bool curr = it_curr != curr_keys.end() && it_curr->second;
        bool prev = it_prev != prev_keys.end() && it_prev->second;
        return !curr && prev;
    }

    bool Input::MouseDown(MouseButton button) const
    {
        auto it = curr_mouse.find((int)button);
        return it != curr_mouse.end() && it->second;
    }

    bool Input::MousePressed(MouseButton button) const
    {
        auto it_curr = curr_mouse.find((int)button);
        auto it_prev = prev_mouse.find((int)button);
        bool curr = it_curr != curr_mouse.end() && it_curr->second;
        bool prev = it_prev != prev_mouse.end() && it_prev->second;
        return curr && !prev;
    }

    bool Input::MouseReleased(MouseButton button) const
    {
        auto it_curr = curr_mouse.find((int)button);
        auto it_prev = prev_mouse.find((int)button);
        bool curr = it_curr != curr_mouse.end() && it_curr->second;
        bool prev = it_prev != prev_mouse.end() && it_prev->second;
        return !curr && prev;
    }

    glm::vec2 Input::MousePos() const { return mouse_pos; }
    glm::vec2 Input::MouseDelta() const { return mouse_pos - prev_mouse_pos; }
    float Input::ScrollDelta() const { return scroll_delta; }

    void Input::KeyCallback(GLFWwindow *handle, int key, int scancode, int action, int mods)
    {
        auto *input = instances[handle];
        if (action == GLFW_PRESS || action == GLFW_RELEASE)
            input->curr_keys[key] = (action == GLFW_PRESS);
    }

    void Input::CursorCallback(GLFWwindow *handle, double x, double y)
    {
        instances[handle]->mouse_pos = {(float)x, (float)y};
    }

    void Input::MouseButtonCallback(GLFWwindow *handle, int button, int action, int mods)
    {
        instances[handle]->curr_mouse[button] = (action == GLFW_PRESS);
    }

    void Input::ScrollCallback(GLFWwindow *handle, double dx, double dy)
    {
        instances[handle]->scroll_delta += (float)dy;
    }
}