#include <yage/yage.h>
#include <yage/extras/texture.h>
using namespace yage;

int main()
{
    WindowConfig cfg;
    cfg.title = "Textures";
    cfg.width = 900;
    cfg.height = 600;
    cfg.gl_version = GLVersion::GL_4_1;

    Window window(cfg);
    Renderer renderer(window);

    Texture face = Texture::FromFile("awesomeface.png");
    Texture container = Texture::FromFile("container.jpg");

    while (!window.ShouldClose())
    {
        window.Poll();

        renderer.BeginFrame();
        renderer.Clear({0.08f, 0.08f, 0.12f, 1.0f});

        renderer.SetTexture(face);
        renderer.DrawRect(50, 50, 180, 180, Color::RGB(255, 255, 255));
        renderer.DrawRect(260, 50, 180, 180, Color::RGB(255, 100, 100));
        renderer.DrawRect(470, 50, 180, 180, Color::RGB(100, 150, 255));

        renderer.ResetTexture();
        renderer.DrawRect(680, 50, 180, 180, Color::RGB(80, 180, 255));

        renderer.SetTexture(container);
        renderer.DrawRect(50, 280, 180, 180, Color::RGB(255, 255, 255));
        renderer.DrawRotatedRect(470, 280, 180, 180, 45.0, Color::RGB(255, 255, 255));

        renderer.SetTexture(face);
        renderer.DrawRect(260, 280, 180, 180, Color::RGB(255, 255, 255));
        renderer.DrawRect(680, 280, 180, 180, Color::RGB(255, 200, 100));

        renderer.EndFrame();
        window.SwapBuffers();
    }
}