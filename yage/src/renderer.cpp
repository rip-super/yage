#include <yage/utils.h>
#include <yage/renderer.h>
#include <yage/shader_source.h>
#include <glm/gtc/matrix_transform.hpp>
#include <earcut.hpp>
#include <cmath>

// todo: make param naming consistent

static void ComputePolygonMiter(
    const glm::vec2 *verts, int count,
    float half_thickness,
    glm::vec2 *outer,
    glm::vec2 *inner)
{
    for (int i = 0; i < count; i++)
    {
        glm::vec2 prev = verts[(i + count - 1) % count];
        glm::vec2 curr = verts[i];
        glm::vec2 next = verts[(i + 1) % count];

        glm::vec2 d1 = glm::normalize(curr - prev);
        glm::vec2 d2 = glm::normalize(next - curr);

        glm::vec2 n1 = {-d1.y, d1.x};
        glm::vec2 n2 = {-d2.y, d2.x};

        glm::vec2 miter = glm::normalize(n1 + n2);
        float len = half_thickness / glm::dot(miter, n1);
        len = std::min(len, half_thickness * 4.0f);

        outer[i] = curr + miter * len;
        inner[i] = curr - miter * len;
    }
}

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

    void Renderer::DrawLine(float x0, float y0, float x1, float y1, float thickness, Color from, Color to)
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

    void Renderer::DrawRotatedLine(float x0, float y0, float x1, float y1, float thickness, float degrees, Color color)
    {
        auto pts = Rotate({{x0, y0}, {x1, y1}}, degrees, (x0 + x1) * 0.5f, (y0 + y1) * 0.5f);
        DrawLine(pts[0].x, pts[0].y, pts[1].x, pts[1].y, thickness, color);
    }

    void Renderer::DrawRotatedLine(float x0, float y0, float x1, float y1, float thickness, float degrees, Color from, Color to)
    {
        auto pts = Rotate({{x0, y0}, {x1, y1}}, degrees, (x0 + x1) * 0.5f, (y0 + y1) * 0.5f);
        DrawLine(pts[0].x, pts[0].y, pts[1].x, pts[1].y, thickness, from, to);
    }

    void Renderer::DrawTriangle(float x0, float y0, float x1, float y1, float x2, float y2, Color color)
    {
        assert(in_frame && "Draw called outside BeginFrame/EndFrame");

        verts.push_back({x0, y0, color.r, color.g, color.b, color.a, 0, 0, 0, 0});
        verts.push_back({x1, y1, color.r, color.g, color.b, color.a, 0, 0, 0, 0});
        verts.push_back({x2, y2, color.r, color.g, color.b, color.a, 0, 0, 0, 0});
    }

    void Renderer::DrawTriangle(float x0, float y0, float x1, float y1, float x2, float y2, Color c0, Color c1, Color c2)
    {
        assert(in_frame && "Draw called outside BeginFrame/EndFrame");

        verts.push_back({x0, y0, c0.r, c0.g, c0.b, c0.a, 0, 0, 0, 0});
        verts.push_back({x1, y1, c1.r, c1.g, c1.b, c1.a, 0, 0, 0, 0});
        verts.push_back({x2, y2, c2.r, c2.g, c2.b, c2.a, 0, 0, 0, 0});
    }

    void Renderer::DrawTriangleOutline(float x0, float y0, float x1, float y1, float x2, float y2, float thickness, Color c)
    {
        assert(in_frame && "Draw called outside BeginFrame/EndFrame");

        glm::vec2 v[3] = {{x0, y0}, {x1, y1}, {x2, y2}};
        glm::vec2 outer[3], inner[3];
        ComputePolygonMiter(v, 3, thickness * 0.5f, outer, inner);

        for (int i = 0; i < 3; i++)
        {
            int j = (i + 1) % 3;

            verts.push_back({outer[i].x, outer[i].y, c.r, c.g, c.b, c.a, 0, 0, 0, 0});
            verts.push_back({inner[i].x, inner[i].y, c.r, c.g, c.b, c.a, 0, 0, 0, 0});
            verts.push_back({outer[j].x, outer[j].y, c.r, c.g, c.b, c.a, 0, 0, 0, 0});
            verts.push_back({inner[i].x, inner[i].y, c.r, c.g, c.b, c.a, 0, 0, 0, 0});
            verts.push_back({inner[j].x, inner[j].y, c.r, c.g, c.b, c.a, 0, 0, 0, 0});
            verts.push_back({outer[j].x, outer[j].y, c.r, c.g, c.b, c.a, 0, 0, 0, 0});
        }
    }

    void Renderer::DrawTriangleOutline(float x0, float y0, float x1, float y1, float x2, float y2, float thickness, Color c0, Color c1, Color c2)
    {
        assert(in_frame && "Draw called outside BeginFrame/EndFrame");

        glm::vec2 v[3] = {{x0, y0}, {x1, y1}, {x2, y2}};
        glm::vec2 outer[3], inner[3];
        ComputePolygonMiter(v, 3, thickness * 0.5f, outer, inner);

        Color colors[3] = {c0, c1, c2};

        for (int i = 0; i < 3; i++)
        {
            int j = (i + 1) % 3;
            Color ci = colors[i], cj = colors[j];

            verts.push_back({outer[i].x, outer[i].y, ci.r, ci.g, ci.b, ci.a, 0, 0, 0, 0});
            verts.push_back({inner[i].x, inner[i].y, ci.r, ci.g, ci.b, ci.a, 0, 0, 0, 0});
            verts.push_back({outer[j].x, outer[j].y, cj.r, cj.g, cj.b, cj.a, 0, 0, 0, 0});
            verts.push_back({inner[i].x, inner[i].y, ci.r, ci.g, ci.b, ci.a, 0, 0, 0, 0});
            verts.push_back({inner[j].x, inner[j].y, cj.r, cj.g, cj.b, cj.a, 0, 0, 0, 0});
            verts.push_back({outer[j].x, outer[j].y, cj.r, cj.g, cj.b, cj.a, 0, 0, 0, 0});
        }
    }

    void Renderer::DrawRotatedTriangle(float x0, float y0, float x1, float y1, float x2, float y2, float degrees, Color color)
    {
        auto pts = Rotate({{x0, y0}, {x1, y1}, {x2, y2}}, degrees, (x0 + x1 + x2) / 3.0f, (y0 + y1 + y2) / 3.0f);
        DrawTriangle(pts[0].x, pts[0].y, pts[1].x, pts[1].y, pts[2].x, pts[2].y, color);
    }

    void Renderer::DrawRotatedTriangle(float x0, float y0, float x1, float y1, float x2, float y2, float degrees, Color c0, Color c1, Color c2)
    {
        auto pts = Rotate({{x0, y0}, {x1, y1}, {x2, y2}}, degrees, (x0 + x1 + x2) / 3.0f, (y0 + y1 + y2) / 3.0f);
        DrawTriangle(pts[0].x, pts[0].y, pts[1].x, pts[1].y, pts[2].x, pts[2].y, c0, c1, c2);
    }

    void Renderer::DrawRotatedTriangleOutline(float x0, float y0, float x1, float y1, float x2, float y2, float thickness, float degrees, Color color)
    {
        auto pts = Rotate({{x0, y0}, {x1, y1}, {x2, y2}}, degrees, (x0 + x1 + x2) / 3.0f, (y0 + y1 + y2) / 3.0f);
        DrawTriangleOutline(pts[0].x, pts[0].y, pts[1].x, pts[1].y, pts[2].x, pts[2].y, thickness, color);
    }

    void Renderer::DrawRotatedTriangleOutline(float x0, float y0, float x1, float y1, float x2, float y2, float thickness, float degrees, Color c0, Color c1, Color c2)
    {
        auto pts = Rotate({{x0, y0}, {x1, y1}, {x2, y2}}, degrees, (x0 + x1 + x2) / 3.0f, (y0 + y1 + y2) / 3.0f);
        DrawTriangleOutline(pts[0].x, pts[0].y, pts[1].x, pts[1].y, pts[2].x, pts[2].y, thickness, c0, c1, c2);
    }

    void Renderer::DrawRect(float x, float y, float w, float h, Color color)
    {
        assert(in_frame && "Draw called outside BeginFrame/EndFrame");

        verts.push_back({x, y, color.r, color.g, color.b, color.a, 0, 0, 0, 0});
        verts.push_back({x + w, y, color.r, color.g, color.b, color.a, 0, 0, 0, 0});
        verts.push_back({x, y + h, color.r, color.g, color.b, color.a, 0, 0, 0, 0});

        verts.push_back({x + w, y, color.r, color.g, color.b, color.a, 0, 0, 0, 0});
        verts.push_back({x, y + h, color.r, color.g, color.b, color.a, 0, 0, 0, 0});
        verts.push_back({x + w, y + h, color.r, color.g, color.b, color.a, 0, 0, 0, 0});
    }

    void Renderer::DrawRect(float x, float y, float w, float h, Color tl, Color tr, Color bl, Color br)
    {
        assert(in_frame && "Draw called outside BeginFrame/EndFrame");

        verts.push_back({x, y, tl.r, tl.g, tl.b, tl.a, 0, 0, 0, 0});
        verts.push_back({x + w, y, tr.r, tr.g, tr.b, tr.a, 0, 0, 0, 0});
        verts.push_back({x, y + h, bl.r, bl.g, bl.b, bl.a, 0, 0, 0, 0});

        verts.push_back({x + w, y, tr.r, tr.g, tr.b, tr.a, 0, 0, 0, 0});
        verts.push_back({x, y + h, bl.r, bl.g, bl.b, bl.a, 0, 0, 0, 0});
        verts.push_back({x + w, y + h, br.r, br.g, br.b, br.a, 0, 0, 0, 0});
    }

    void Renderer::DrawRectOutline(float x, float y, float w, float h, float thickness, Color c)
    {
        assert(in_frame && "Draw called outside BeginFrame/EndFrame");

        glm::vec2 v[4] = {{x, y}, {x + w, y}, {x + w, y + h}, {x, y + h}};
        glm::vec2 outer[4], inner[4];
        ComputePolygonMiter(v, 4, thickness * 0.5f, outer, inner);

        for (int i = 0; i < 4; i++)
        {
            int j = (i + 1) % 4;
            verts.push_back({outer[i].x, outer[i].y, c.r, c.g, c.b, c.a, 0, 0, 0, 0});
            verts.push_back({inner[i].x, inner[i].y, c.r, c.g, c.b, c.a, 0, 0, 0, 0});
            verts.push_back({outer[j].x, outer[j].y, c.r, c.g, c.b, c.a, 0, 0, 0, 0});
            verts.push_back({inner[i].x, inner[i].y, c.r, c.g, c.b, c.a, 0, 0, 0, 0});
            verts.push_back({inner[j].x, inner[j].y, c.r, c.g, c.b, c.a, 0, 0, 0, 0});
            verts.push_back({outer[j].x, outer[j].y, c.r, c.g, c.b, c.a, 0, 0, 0, 0});
        }
    }

    void Renderer::DrawRectOutline(float x, float y, float w, float h, float thickness, Color tl, Color tr, Color bl, Color br)
    {
        assert(in_frame && "Draw called outside BeginFrame/EndFrame");

        glm::vec2 v[4] = {{x, y}, {x + w, y}, {x + w, y + h}, {x, y + h}};
        glm::vec2 outer[4], inner[4];
        ComputePolygonMiter(v, 4, thickness * 0.5f, outer, inner);

        Color colors[4] = {tl, tr, br, bl};

        for (int i = 0; i < 4; i++)
        {
            int j = (i + 1) % 4;
            Color ci = colors[i], cj = colors[j];
            verts.push_back({outer[i].x, outer[i].y, ci.r, ci.g, ci.b, ci.a, 0, 0, 0, 0});
            verts.push_back({inner[i].x, inner[i].y, ci.r, ci.g, ci.b, ci.a, 0, 0, 0, 0});
            verts.push_back({outer[j].x, outer[j].y, cj.r, cj.g, cj.b, cj.a, 0, 0, 0, 0});
            verts.push_back({inner[i].x, inner[i].y, ci.r, ci.g, ci.b, ci.a, 0, 0, 0, 0});
            verts.push_back({inner[j].x, inner[j].y, cj.r, cj.g, cj.b, cj.a, 0, 0, 0, 0});
            verts.push_back({outer[j].x, outer[j].y, cj.r, cj.g, cj.b, cj.a, 0, 0, 0, 0});
        }
    }

    void Renderer::DrawRotatedRect(float x, float y, float w, float h, float degrees, Color color)
    {
        auto pts = Rotate({{x, y}, {x + w, y}, {x + w, y + h}, {x, y + h}}, degrees, x + w * 0.5f, y + h * 0.5f);
        DrawPolygon(pts, color);
    }

    void Renderer::DrawRotatedRect(float x, float y, float w, float h, float degrees, Color tl, Color tr, Color bl, Color br)
    {
        auto pts = Rotate({{x, y}, {x + w, y}, {x + w, y + h}, {x, y + h}}, degrees, x + w * 0.5f, y + h * 0.5f);
        DrawPolygon(pts, {tl, tr, br, bl});
    }

    void Renderer::DrawRotatedRectOutline(float x, float y, float w, float h, float thickness, float degrees, Color color)
    {
        auto pts = Rotate({{x, y}, {x + w, y}, {x + w, y + h}, {x, y + h}}, degrees, x + w * 0.5f, y + h * 0.5f);
        DrawPolygonOutline(pts, thickness, color);
    }

    void Renderer::DrawRotatedRectOutline(float x, float y, float w, float h, float thickness, float degrees, Color tl, Color tr, Color bl, Color br)
    {
        auto pts = Rotate({{x, y}, {x + w, y}, {x + w, y + h}, {x, y + h}}, degrees, x + w * 0.5f, y + h * 0.5f);
        DrawPolygonOutline(pts, thickness, {tl, tr, br, bl});
    }

    void Renderer::DrawCircle(float x, float y, float radius, Color color)
    {
        assert(in_frame && "Draw called outside BeginFrame/EndFrame");

        float r = color.r, g = color.g, b = color.b, a = color.a;

        verts.push_back({x - radius, y - radius, r, g, b, a, -1, -1, 1, 0});
        verts.push_back({x + radius, y - radius, r, g, b, a, 1, -1, 1, 0});
        verts.push_back({x - radius, y + radius, r, g, b, a, -1, 1, 1, 0});

        verts.push_back({x + radius, y - radius, r, g, b, a, 1, -1, 1, 0});
        verts.push_back({x + radius, y + radius, r, g, b, a, 1, 1, 1, 0});
        verts.push_back({x - radius, y + radius, r, g, b, a, -1, 1, 1, 0});
    }

    void Renderer::DrawCircleOutline(float x, float y, float radius, float thickness, Color color)
    {
        assert(in_frame && "Draw called outside BeginFrame/EndFrame");

        float r = color.r, g = color.g, b = color.b, a = color.a;
        float inner = (radius - thickness) / radius;
        inner = std::max(inner, 0.0f);

        verts.push_back({x - radius, y - radius, r, g, b, a, -1, -1, 1, inner});
        verts.push_back({x + radius, y - radius, r, g, b, a, 1, -1, 1, inner});
        verts.push_back({x - radius, y + radius, r, g, b, a, -1, 1, 1, inner});

        verts.push_back({x + radius, y - radius, r, g, b, a, 1, -1, 1, inner});
        verts.push_back({x + radius, y + radius, r, g, b, a, 1, 1, 1, inner});
        verts.push_back({x - radius, y + radius, r, g, b, a, -1, 1, 1, inner});
    }

    void Renderer::DrawPolygon(std::vector<glm::vec2> points, Color color)
    {
        assert(in_frame && "Draw called outside BeginFrame/EndFrame");
        assert(points.size() >= 3 && "Polygon needs at least 3 points");

        std::vector<std::vector<std::array<float, 2>>> poly;
        auto &ring = poly.emplace_back();
        for (auto &p : points)
            ring.push_back({p.x, p.y});

        auto indices = mapbox::earcut<uint32_t>(poly);

        for (size_t i = 0; i + 2 < indices.size(); i += 3)
        {
            auto &p0 = points[indices[i]];
            auto &p1 = points[indices[i + 1]];
            auto &p2 = points[indices[i + 2]];

            verts.push_back({p0.x, p0.y, color.r, color.g, color.b, color.a, 0, 0, 0, 0});
            verts.push_back({p1.x, p1.y, color.r, color.g, color.b, color.a, 0, 0, 0, 0});
            verts.push_back({p2.x, p2.y, color.r, color.g, color.b, color.a, 0, 0, 0, 0});
        }
    }

    void Renderer::DrawPolygon(std::vector<glm::vec2> points, std::vector<Color> colors)
    {
        assert(in_frame && "Draw called outside BeginFrame/EndFrame");
        assert(points.size() >= 3 && "Polygon needs at least 3 points");
        assert(points.size() == colors.size() && "Point and color count must match");

        std::vector<std::vector<std::array<float, 2>>> poly;
        auto &ring = poly.emplace_back();
        for (auto &p : points)
            ring.push_back({p.x, p.y});

        auto indices = mapbox::earcut<uint32_t>(poly);

        for (size_t i = 0; i + 2 < indices.size(); i += 3)
        {
            auto &p0 = points[indices[i]];
            auto &p1 = points[indices[i + 1]];
            auto &p2 = points[indices[i + 2]];

            Color c0 = colors[indices[i]];
            Color c1 = colors[indices[i + 1]];
            Color c2 = colors[indices[i + 2]];

            verts.push_back({p0.x, p0.y, c0.r, c0.g, c0.b, c0.a, 0, 0, 0, 0});
            verts.push_back({p1.x, p1.y, c1.r, c1.g, c1.b, c1.a, 0, 0, 0, 0});
            verts.push_back({p2.x, p2.y, c2.r, c2.g, c2.b, c2.a, 0, 0, 0, 0});
        }
    }

    void Renderer::DrawPolygonOutline(std::vector<glm::vec2> points, float thickness, Color color)
    {
        assert(in_frame && "Draw called outside BeginFrame/EndFrame");
        assert(points.size() >= 3 && "Polygon needs at least 3 points");

        std::vector<glm::vec2> outer(points.size()), inner(points.size());
        ComputePolygonMiter(points.data(), (int)points.size(), thickness * 0.5f, outer.data(), inner.data());

        for (size_t i = 0; i < points.size(); i++)
        {
            size_t j = (i + 1) % points.size();
            verts.push_back({outer[i].x, outer[i].y, color.r, color.g, color.b, color.a, 0, 0, 0, 0});
            verts.push_back({inner[i].x, inner[i].y, color.r, color.g, color.b, color.a, 0, 0, 0, 0});
            verts.push_back({outer[j].x, outer[j].y, color.r, color.g, color.b, color.a, 0, 0, 0, 0});
            verts.push_back({inner[i].x, inner[i].y, color.r, color.g, color.b, color.a, 0, 0, 0, 0});
            verts.push_back({inner[j].x, inner[j].y, color.r, color.g, color.b, color.a, 0, 0, 0, 0});
            verts.push_back({outer[j].x, outer[j].y, color.r, color.g, color.b, color.a, 0, 0, 0, 0});
        }
    }

    void Renderer::DrawPolygonOutline(std::vector<glm::vec2> points, float thickness, std::vector<Color> colors)
    {
        assert(in_frame && "Draw called outside BeginFrame/EndFrame");
        assert(points.size() >= 3 && "Polygon needs at least 3 points");
        assert(points.size() == colors.size() && "Point and color count must match");

        std::vector<glm::vec2> outer(points.size()), inner(points.size());
        ComputePolygonMiter(points.data(), (int)points.size(), thickness * 0.5f, outer.data(), inner.data());

        for (size_t i = 0; i < points.size(); i++)
        {
            size_t j = (i + 1) % points.size();
            Color ci = colors[i], cj = colors[j];
            verts.push_back({outer[i].x, outer[i].y, ci.r, ci.g, ci.b, ci.a, 0, 0, 0, 0});
            verts.push_back({inner[i].x, inner[i].y, ci.r, ci.g, ci.b, ci.a, 0, 0, 0, 0});
            verts.push_back({outer[j].x, outer[j].y, cj.r, cj.g, cj.b, cj.a, 0, 0, 0, 0});
            verts.push_back({inner[i].x, inner[i].y, ci.r, ci.g, ci.b, ci.a, 0, 0, 0, 0});
            verts.push_back({inner[j].x, inner[j].y, cj.r, cj.g, cj.b, cj.a, 0, 0, 0, 0});
            verts.push_back({outer[j].x, outer[j].y, cj.r, cj.g, cj.b, cj.a, 0, 0, 0, 0});
        }
    }

    void Renderer::DrawRotatedPolygon(std::vector<glm::vec2> points, float degrees, Color color)
    {
        auto c = Centroid(points);
        DrawPolygon(Rotate(points, degrees, c.x, c.y), color);
    }

    void Renderer::DrawRotatedPolygon(std::vector<glm::vec2> points, float degrees, std::vector<Color> colors)
    {
        auto c = Centroid(points);
        DrawPolygon(Rotate(points, degrees, c.x, c.y), colors);
    }

    void Renderer::DrawRotatedPolygonOutline(std::vector<glm::vec2> points, float thickness, float degrees, Color color)
    {
        auto c = Centroid(points);
        DrawPolygonOutline(Rotate(points, degrees, c.x, c.y), thickness, color);
    }

    void Renderer::DrawRotatedPolygonOutline(std::vector<glm::vec2> points, float thickness, float degrees, std::vector<Color> colors)
    {
        auto c = Centroid(points);
        DrawPolygonOutline(Rotate(points, degrees, c.x, c.y), thickness, colors);
    }
}