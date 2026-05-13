#pragma once
#include "types.h"
#include <GLFW/glfw3.h>

namespace yage
{
    class Window
    {
    public:
        explicit Window(const WindowConfig &cfg);
        ~Window();

        bool ShouldClose() const;
        void SwapBuffers();
        void SetTitle(const std::string &title);
        glm::ivec2 GetSize() const;
        float GetAspect() const;

        void Poll();

        float GetDt() const;
        float GetTime() const;

        GLFWwindow *GetHandle() const;

    private:
        GLFWwindow *handle;
        int width, height;
        float time, dt;
    };
}