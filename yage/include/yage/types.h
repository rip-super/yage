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

        Color() = default;
        Color(float r, float g, float b, float a = 1.0f) : r(r), g(g), b(b), a(a) {}

        static Color RGB(uint8_t r, uint8_t g, uint8_t b)
        {
            return {r / 255.0f, g / 255.0f, b / 255.0f, 1.0f};
        }

        static Color RGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
        {
            return {r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f};
        }

        static Color Hex(uint32_t hex)
        {
            return {
                ((hex >> 16) & 0xFF) / 255.0f,
                ((hex >> 8) & 0xFF) / 255.0f,
                ((hex) & 0xFF) / 255.0f,
                1.0f};
        }
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