#pragma once
#include <string>
#include <stdexcept>
#include <glad/glad.h>
#include <glm/glm.hpp>

namespace yage
{
    enum class GLVersion
    {
        GL_3_3,
        GL_4_1,
        GL_4_4,
        GL_4_6
    };

    struct WindowConfig
    {
        std::string title = "YAGE Application";
        int width = 800;
        int height = 600;
        bool vsync = true;
        bool fullscreen = false;
        bool resizable = true;
        bool mouse_grab = false;
        GLVersion gl_version = GLVersion::GL_3_3;
    };
}