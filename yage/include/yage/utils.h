#pragma once
#include <string>
#include <stdexcept>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include <cmath>

namespace yage
{
    enum class GLVersion
    {
        GL_3_3,
        GL_4_1,
        GL_4_4,
        GL_4_6
    };

    // todo: show fps??
    struct WindowConfig
    {
        std::string title = "YAGE Application";
        int width = 800;
        int height = 600;
        bool vsync = true;
        bool fullscreen = false;
        bool resizable = true;
        bool mouse_grab = false;
        int msaa_samples = 4;
        GLVersion gl_version = GLVersion::GL_3_3;
    };

    struct TextureConfig
    {
        GLenum filter = GL_LINEAR;      // GL_LINEAR or GL_NEAREST
        GLenum wrap = GL_CLAMP_TO_EDGE; // GL_REPEAT, GL_CLAMP_TO_EDGE, GL_MIRRORED_REPEAT
        bool flip_y = false;
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

    inline std::vector<glm::vec2> MakeRegularPolygon(float cx, float cy, float radius, int sides)
    {
        assert(sides >= 3 && "Polygon needs at least 3 sides");

        std::vector<glm::vec2> pts;
        pts.reserve(sides);

        for (int i = 0; i < sides; i++)
        {
            float angle = glm::radians(-90.0f + (360.0f / sides) * i);
            pts.push_back({cx + radius * std::cos(angle), cy + radius * std::sin(angle)});
        }

        return pts;
    }

    inline glm::vec2 Centroid(const std::vector<glm::vec2> &points)
    {
        glm::vec2 c = {0, 0};
        for (auto &p : points)
            c += p;
        return c / (float)points.size();
    }

    inline std::vector<glm::vec2> Rotate(std::vector<glm::vec2> points, float degrees, float cx, float cy)
    {
        float rad = glm::radians(degrees);
        float cos = std::cos(rad);
        float sin = std::sin(rad);
        for (auto &p : points)
        {
            float x = p.x - cx;
            float y = p.y - cy;
            p.x = x * cos - y * sin + cx;
            p.y = x * sin + y * cos + cy;
        }
        return points;
    }

    inline std::vector<glm::vec2> Rotate(std::vector<glm::vec2> points, float degrees)
    {
        auto c = Centroid(points);
        return Rotate(points, degrees, c.x, c.y);
    }
}