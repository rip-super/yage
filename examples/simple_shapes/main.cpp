#include <yage/yage.h>
using namespace yage;

int main()
{
    WindowConfig cfg;
    cfg.title = "Simple Shapes";
    cfg.width = 600;
    cfg.height = 550;
    cfg.gl_version = GLVersion::GL_4_1;

    Window window(cfg);
    Renderer renderer(window);
    renderer.Clear({0.08f, 0.08f, 0.12f, 1.0f});

    while (!window.ShouldClose())
    {
        window.Poll();
        renderer.BeginFrame();
        renderer.Clear({0.08f, 0.08f, 0.12f, 1.0f});

        renderer.DrawLine(30, 30, 200, 30, 2.0f, Color::RGB(255, 255, 255));
        renderer.DrawLine(30, 50, 200, 50, 4.0f, Color::RGB(255, 80, 0), Color::RGB(0, 150, 255));
        renderer.DrawRotatedLine(30, 70, 200, 70, 2.0f, 8.0f, Color::RGB(100, 255, 180));

        renderer.DrawTriangle(240, 70, 310, 20, 380, 70, Color::RGB(0, 200, 255));
        renderer.DrawTriangleOutline(400, 70, 470, 20, 540, 70, 2.0f, Color::RGB(255, 180, 0));

        renderer.DrawRect(30, 120, 140, 90, Color::RGB(50, 180, 100));
        renderer.DrawRotatedRectOutline(210, 140, 140, 90, 2.0f, 12.0f, Color::RGB(200, 80, 255));

        renderer.DrawCircle(430, 165, 55, Color::RGB(255, 60, 180));
        renderer.DrawCircleOutline(530, 165, 45, 3.0f, Color::RGB(60, 220, 255));

        renderer.DrawPolygon(MakeRegularPolygon(80, 330, 55, 5), Color::RGB(255, 200, 0));
        renderer.DrawRotatedPolygonOutline(MakeRegularPolygon(220, 330, 55, 6), 2.0f, 15.0f, Color::RGB(100, 200, 255));

        renderer.DrawRotatedPolygon({{330, 280}, {420, 270}, {460, 330}, {430, 390}, {340, 390}, {300, 330}}, 10.0f, Color::RGB(180, 80, 255));
        renderer.DrawRotatedPolygonOutline({{490, 280}, {560, 270}, {590, 340}, {550, 390}, {480, 380}}, 2.0f, -10.0f, Color::RGB(100, 255, 160));

        renderer.DrawTriangle(30, 480, 130, 420, 220, 480, Color::RGB(255, 0, 80), Color::RGB(255, 220, 0), Color::RGB(0, 200, 255));
        renderer.DrawRect(250, 430, 150, 90, Color::RGB(255, 0, 0), Color::RGB(255, 255, 0), Color::RGB(0, 0, 255), Color::RGB(0, 255, 255));
        renderer.DrawRotatedPolygon(MakeRegularPolygon(490, 480, 60, 7), 8.0f, {Color::RGB(255, 50, 50), Color::RGB(255, 150, 0), Color::RGB(255, 255, 0), Color::RGB(0, 255, 100), Color::RGB(0, 150, 255), Color::RGB(150, 0, 255), Color::RGB(255, 0, 150)});

        renderer.EndFrame();
        window.SwapBuffers();
    }
}