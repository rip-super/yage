#include <yage/yage.h>
#include <yage/extras/shader.h>
using namespace yage;

int main()
{
    WindowConfig cfg;
    cfg.title = "Custom Shader";
    cfg.width = 800;
    cfg.height = 500;
    cfg.gl_version = GLVersion::GL_4_1;

    Window window(cfg);
    Renderer renderer(window);
    renderer.Clear({0.08f, 0.08f, 0.12f, 1.0f});

    Shader wave("custom.vert", "custom.frag");

    while (!window.ShouldClose())
    {
        window.Poll();
        float t = window.GetTime();

        renderer.BeginFrame();
        renderer.Clear({0.08f, 0.08f, 0.12f, 1.0f});

        renderer.SetShader(wave);
        wave.setFloat("u_time", t);

        renderer.DrawRect(80, 60, 280, 140, Color::RGB(255, 80, 80));
        renderer.DrawRect(440, 60, 280, 140, Color::RGB(255, 0, 0), Color::RGB(0, 255, 0), Color::RGB(0, 0, 255), Color::RGB(255, 255, 0));
        renderer.DrawCircle(200, 340, 80, Color::RGB(80, 180, 255));
        renderer.DrawCircleOutline(600, 340, 80, 5.0f, Color::RGB(80, 255, 150));
        renderer.DrawPolygon(MakeRegularPolygon(400, 350, 70, 6), Color::RGB(255, 180, 80));

        renderer.ResetShader();
        renderer.EndFrame();
        window.SwapBuffers();
    }
}