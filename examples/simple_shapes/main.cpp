#include <yage/yage.h>
using namespace yage;

int main()
{
    WindowConfig cfg;
    cfg.title = "Shapes!!";
    cfg.width = 800;
    cfg.height = 600;
    cfg.gl_version = GLVersion::GL_4_1;

    Window window(cfg);
    Renderer renderer(window);

    while (!window.ShouldClose())
    {
        window.Poll();
        renderer.BeginFrame();

        renderer.Clear({0.1f, 0.1f, 0.15f, 1.0f});

        renderer.DrawLine(100, 100, 700, 100, 8.0f, {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f, 1.0f});
        renderer.DrawLine(400, 50, 400, 550, 4.0f, {0.0f, 1.0f, 0.0f, 1.0f});
        renderer.DrawLine(100, 500, 700, 200, 8.0f, {0.0f, 0.0f, 1.0f, 1.0f});

        renderer.EndFrame();
        window.SwapBuffers();
    }
}