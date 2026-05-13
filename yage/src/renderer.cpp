#include <yage/types.h>
#include <yage/renderer.h>
#include <yage/shader_source.h>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

namespace yage
{
    Renderer::Renderer(const Window &window)
        : window(window),
          shader(Shader::FromSource(internal::BATCH_VERT_SRC, internal::BATCH_FRAG_SRC))
    {
        verts.reserve(MAX_VERTS);

        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);

        glBufferData(GL_ARRAY_BUFFER, MAX_VERTS * sizeof(Vertex), nullptr, GL_DYNAMIC_DRAW);

        constexpr int stride = sizeof(Vertex);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, (void *)offsetof(Vertex, x));
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, stride, (void *)offsetof(Vertex, r));
        glEnableVertexAttribArray(1);

        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void *)offsetof(Vertex, u));
        glEnableVertexAttribArray(2);

        glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, stride, (void *)offsetof(Vertex, shape));
        glEnableVertexAttribArray(3);

        glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, stride, (void *)offsetof(Vertex, param));
        glEnableVertexAttribArray(4);

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    Renderer::~Renderer()
    {
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vbo);
    }

    void Renderer::BeginFrame(const glm::mat4 &view_proj)
    {
        assert(!in_frame && "BeginFrame called twice without EndFrame");

        in_frame = true;
        verts.clear();

        if (view_proj == glm::mat4(1.0f))
        {
            float w = (float)window.GetWidth();
            float h = (float)window.GetHeight();
            this->view_proj = glm::ortho(0.0f, w, h, 0.0f, -1.0f, 1.0f);
        }
        else
        {
            this->view_proj = view_proj;
        }
    }

    void Renderer::EndFrame()
    {
        assert(in_frame && "EndFrame called without BeginFrame");

        in_frame = false;

        if (verts.empty())
            return;

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0,
                        verts.size() * sizeof(Vertex),
                        verts.data());

        shader.bind();
        shader.setMat4("u_view_proj", view_proj);

        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, (GLsizei)verts.size());
        glBindVertexArray(0);
    }

    void Renderer::Clear(Color color)
    {
        glClearColor(color.r, color.g, color.b, color.a);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    void Renderer::DrawLine(float x0, float y0, float x1, float y1, float thickness, Color color)
    {
        assert(in_frame && "Draw called outside BeginFrame/EndFrame");

        float dx = x1 - x0;
        float dy = y1 - y0;

        float len = std::sqrt(dx * dx + dy * dy);
        if (len == 0.0f)
            return;

        float px = -(dy / len) * (thickness * 0.5f);
        float py = (dx / len) * (thickness * 0.5f);

        float ax = x0 + px, ay = y0 + py;
        float bx = x0 - px, by = y0 - py;
        float cx = x1 + px, cy = y1 + py;
        float ex = x1 - px, ey = y1 - py;

        verts.push_back({ax, ay, color.r, color.g, color.b, color.a, 0, 0, 0, 0});
        verts.push_back({bx, by, color.r, color.g, color.b, color.a, 0, 0, 0, 0});
        verts.push_back({cx, cy, color.r, color.g, color.b, color.a, 0, 0, 0, 0});

        verts.push_back({bx, by, color.r, color.g, color.b, color.a, 0, 0, 0, 0});
        verts.push_back({ex, ey, color.r, color.g, color.b, color.a, 0, 0, 0, 0});
        verts.push_back({cx, cy, color.r, color.g, color.b, color.a, 0, 0, 0, 0});
    }

    void Renderer::DrawLine(float x0, float y0, float x1, float y1,
                            float thickness, Color from, Color to)
    {
        assert(in_frame && "Draw called outside BeginFrame/EndFrame");

        float dx = x1 - x0;
        float dy = y1 - y0;

        float len = std::sqrt(dx * dx + dy * dy);
        if (len == 0.0f)
            return;

        float px = -(dy / len) * (thickness * 0.5f);
        float py = (dx / len) * (thickness * 0.5f);

        float ax = x0 + px, ay = y0 + py;
        float bx = x0 - px, by = y0 - py;
        float cx = x1 + px, cy = y1 + py;
        float ex = x1 - px, ey = y1 - py;

        verts.push_back({ax, ay, from.r, from.g, from.b, from.a, 0, 0, 0, 0});
        verts.push_back({bx, by, from.r, from.g, from.b, from.a, 0, 0, 0, 0});
        verts.push_back({cx, cy, to.r, to.g, to.b, to.a, 0, 0, 0, 0});

        verts.push_back({bx, by, from.r, from.g, from.b, from.a, 0, 0, 0, 0});
        verts.push_back({ex, ey, to.r, to.g, to.b, to.a, 0, 0, 0, 0});
        verts.push_back({cx, cy, to.r, to.g, to.b, to.a, 0, 0, 0, 0});
    }
}