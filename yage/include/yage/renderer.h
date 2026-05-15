#pragma once
#include "utils.h"
#include "window.h"
#include <glm/glm.hpp>
#include <glad/glad.h>
#include <vector>

namespace yage
{
    class Shader;
    class Texture;

    class Renderer
    {
    public:
        explicit Renderer(const Window &window);
        ~Renderer();

        void BeginFrame(const glm::mat4 &view_proj = glm::mat4(1.0f));
        void EndFrame();

        // custom shaders must handle the circle SDF manually.
        void SetShader(const Shader &shader);
        void ResetShader();

        void SetTexture(const Texture &tex);
        void ResetTexture();

        void Clear(Color color = {0.0f, 0.0f, 0.0f, 1.0f});

        void DrawLine(float x0, float y0, float x1, float y1, float thickness, Color color);
        void DrawLine(float x0, float y0, float x1, float y1, float thickness, Color from, Color to);

        void DrawRotatedLine(float x0, float y0, float x1, float y1, float thickness, float degrees, Color color);
        void DrawRotatedLine(float x0, float y0, float x1, float y1, float thickness, float degrees, Color from, Color to);

        void DrawTriangle(float x0, float y0, float x1, float y1, float x2, float y2, Color color);
        void DrawTriangle(float x0, float y0, float x1, float y1, float x2, float y2, Color c0, Color c1, Color c2);
        void DrawTriangleOutline(float x0, float y0, float x1, float y1, float x2, float y2, float thickness, Color color);
        void DrawTriangleOutline(float x0, float y0, float x1, float y1, float x2, float y2, float thickness, Color c0, Color c1, Color c2);

        void DrawRotatedTriangle(float x0, float y0, float x1, float y1, float x2, float y2, float degrees, Color color);
        void DrawRotatedTriangle(float x0, float y0, float x1, float y1, float x2, float y2, float degrees, Color c0, Color c1, Color c2);
        void DrawRotatedTriangleOutline(float x0, float y0, float x1, float y1, float x2, float y2, float thickness, float degrees, Color color);
        void DrawRotatedTriangleOutline(float x0, float y0, float x1, float y1, float x2, float y2, float thickness, float degrees, Color c0, Color c1, Color c2);

        void DrawRect(float x, float y, float w, float h, Color color);
        void DrawRect(float x, float y, float w, float h, Color tl, Color tr, Color bl, Color br);
        void DrawRectOutline(float x, float y, float w, float h, float thickness, Color color);
        void DrawRectOutline(float x, float y, float w, float h, float thickness, Color tl, Color tr, Color bl, Color br);

        void DrawRotatedRect(float x, float y, float w, float h, float degrees, Color color);
        void DrawRotatedRect(float x, float y, float w, float h, float degrees, Color tl, Color tr, Color bl, Color br);
        void DrawRotatedRectOutline(float x, float y, float w, float h, float thickness, float degrees, Color color);
        void DrawRotatedRectOutline(float x, float y, float w, float h, float thickness, float degrees, Color tl, Color tr, Color bl, Color br);

        void DrawCircle(float x, float y, float radius, Color color);
        void DrawCircleOutline(float x, float y, float radius, float thickness, Color color);

        void DrawPolygon(std::vector<glm::vec2> points, Color color);
        void DrawPolygon(std::vector<glm::vec2> points, std::vector<Color> colors);
        void DrawPolygonOutline(std::vector<glm::vec2> points, float thickness, Color color);
        void DrawPolygonOutline(std::vector<glm::vec2> points, float thickness, std::vector<Color> colors);

        void DrawRotatedPolygon(std::vector<glm::vec2> points, float degrees, Color color);
        void DrawRotatedPolygon(std::vector<glm::vec2> points, float degrees, std::vector<Color> colors);
        void DrawRotatedPolygonOutline(std::vector<glm::vec2> points, float thickness, float degrees, Color color);
        void DrawRotatedPolygonOutline(std::vector<glm::vec2> points, float thickness, float degrees, std::vector<Color> colors);

    private:
        GLuint vao = 0;
        GLuint vbo = 0;
        GLuint batch_shader = 0;
        GLuint current_shader = 0;
        GLuint white_texture = 0;

        std::vector<Vertex> verts;
        static constexpr uint32_t MAX_VERTS = 65'536;

        glm::mat4 view_proj = glm::mat4(1.0f);
        bool in_frame = false;

        const Window &window;

        void Flush();
    };
}