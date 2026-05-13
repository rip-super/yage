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

    struct Color
    {
        float r = 1.0f, g = 1.0f, b = 1.0f, a = 1.0f;
    };

    struct Vertex
    {
        float x, y;
        float r, g, b, a;
        float u, v;
        float shape;
        float param;
    };
}