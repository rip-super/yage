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

        renderer.DrawRectOutline(150, 150, 500, 300, 4.0f, Color::RGB(255, 0, 0), Color::RGB(0, 255, 0), Color::RGB(0, 0, 255), Color::RGB(255, 0, 255));

        renderer.EndFrame();
        window.SwapBuffers();
    }
}