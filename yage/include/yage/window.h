#pragma once
#include "utils.h"
#include <GLFW/glfw3.h>

namespace yage
{
    class Input;

    class Window
    {
    public:
        explicit Window(const WindowConfig &cfg);
        ~Window();

        bool ShouldClose() const;
        void SwapBuffers();
        void SetTitle(const std::string &title);
        glm::ivec2 GetSize() const;
        int GetWidth() const;
        int GetHeight() const;
        int GetFramebufferWidth() const;
        int GetFramebufferHeight() const;
        float GetAspect() const;

        void Poll();
        void Poll(Input &input);

        float GetDt() const;
        float GetTime() const;

        GLFWwindow *GetHandle() const;

    private:
        GLFWwindow *handle;
        int width, height, fb_width, fb_height;
        float time, dt;
    };
}