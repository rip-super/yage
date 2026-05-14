#include <yage/yage.h>
using namespace yage;

int main()
{
    WindowConfig cfg;
    cfg.title = "Simple Shapes";
    cfg.width = 1280;
    cfg.height = 720;
    cfg.gl_version = GLVersion::GL_4_1;

    Window window(cfg);
    Renderer renderer(window);
    renderer.Clear({0.08f, 0.08f, 0.12f, 1.0f});

    while (!window.ShouldClose())
    {
        window.Poll();
        renderer.BeginFrame();
        renderer.Clear({0.08f, 0.08f, 0.12f, 1.0f});

        renderer.DrawLine(40, 50, 280, 50, 3.0f, Color::RGB(255, 255, 255));
        renderer.DrawLine(600, 40, 840, 80, 3.0f, Color::RGB(255, 80, 0), Color::RGB(0, 150, 255));
        renderer.DrawRotatedLine(1020, 60, 1240, 60, 3.0f, 12.0f, Color::RGB(100, 255, 180));

        renderer.DrawTriangle(60, 220, 200, 130, 320, 230, Color::RGB(255, 0, 80), Color::RGB(255, 220, 0), Color::RGB(0, 200, 255));
        renderer.DrawRectOutline(60, 320, 180, 110, 3.0f, Color::RGB(255, 200, 80));
        renderer.DrawCircleOutline(150, 560, 70, 5.0f, Color::RGB(60, 220, 255));

        renderer.DrawRotatedRect(460, 180, 190, 110, 15.0f, Color::RGB(50, 180, 255));
        renderer.DrawRotatedPolygon(MakeRegularPolygon(520, 420, 90, 6), 10.0f, {Color::RGB(255, 50, 50), Color::RGB(255, 180, 0), Color::RGB(255, 255, 0), Color::RGB(0, 255, 100), Color::RGB(0, 150, 255), Color::RGB(180, 0, 255)});
        renderer.DrawRotatedTriangleOutline(400, 640, 540, 560, 620, 660, 3.0f, -15.0f, Color::RGB(200, 100, 255));

        renderer.DrawCircle(950, 180, 90, Color::RGB(255, 60, 180));
        renderer.DrawRotatedPolygonOutline({{820, 380}, {960, 330}, {1060, 410}, {1020, 530}, {860, 550}, {780, 460}}, 3.0f, 8.0f, Color::RGB(100, 255, 200));
        renderer.DrawRotatedRect(870, 595, 200, 100, -12.0f, Color::RGB(255, 0, 0), Color::RGB(255, 255, 0), Color::RGB(0, 0, 255), Color::RGB(0, 255, 255));

        renderer.EndFrame();
        window.SwapBuffers();
    }
}