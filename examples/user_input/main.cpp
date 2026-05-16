#include <yage/yage.h>
using namespace yage;

int main()
{
    WindowConfig cfg;
    cfg.title = "User Input";
    cfg.width = 800;
    cfg.height = 600;
    cfg.gl_version = GLVersion::GL_4_1;

    Window window(cfg);
    Input input(window);
    Renderer renderer(window);

    float sq_x = 400.0f, sq_y = 300.0f;
    const float sq_size = 150.0f;
    const float sq_speed = 400.0f;
    Color circle_color = Color::RGB(255, 50, 50);

    while (!window.ShouldClose())
    {
        window.Poll(input);
        float dt = window.GetDt();

        // clang-format off
        if (input.KeyDown(Key::W) || input.KeyDown(Key::Up))    sq_y -= sq_speed * dt;
        if (input.KeyDown(Key::S) || input.KeyDown(Key::Down))  sq_y += sq_speed * dt;
        if (input.KeyDown(Key::A) || input.KeyDown(Key::Left))  sq_x -= sq_speed * dt;
        if (input.KeyDown(Key::D) || input.KeyDown(Key::Right)) sq_x += sq_speed * dt;

        if (input.MouseDown(MouseButton::Left))       circle_color = Color::RGB(255, 100, 200);
        if (input.MouseDown(MouseButton::Right))      circle_color = Color::RGB(50, 255, 100);
        if (input.MouseDown(MouseButton::Middle))     circle_color = Color::RGB(255, 200, 0);
        if (input.MouseReleased(MouseButton::Left))   circle_color = Color::RGB(255, 50, 50);
        if (input.MouseReleased(MouseButton::Right))  circle_color = Color::RGB(255, 50, 50);
        if (input.MouseReleased(MouseButton::Middle)) circle_color = Color::RGB(255, 50, 50);
        // clang-format on 

        glm::vec2 mouse = input.MousePos();

        renderer.BeginFrame();
        renderer.Clear({0.08f, 0.08f, 0.12f, 1.0f});

        renderer.DrawRect(sq_x - sq_size * 0.5f, sq_y - sq_size * 0.5f, sq_size, sq_size, Color::RGB(50, 120, 255));
        renderer.DrawCircle(mouse.x, mouse.y, 12.0f, circle_color);

        renderer.EndFrame();
        window.SwapBuffers();
    }
}