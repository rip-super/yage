#include <yage/yage.h>
#include <yage/extras/camera.h>
#include <algorithm>

using namespace std;
using namespace yage;

int main()
{
    WindowConfig cfg;
    cfg.title = "Camera Example";
    cfg.width = 1024;
    cfg.height = 768;
    cfg.gl_version = GLVersion::GL_4_1;

    Window window(cfg);
    Input input(window);
    Renderer renderer(window);
    Camera camera;

    float player_x = 0.0f, player_y = 0.0f;
    const float player_size = 32.0f;
    const float player_speed = 300.0f;
    float current_zoom = 1.0f;

    while (!window.ShouldClose())
    {
        window.Poll(input);
        float dt = window.GetDt();

        if (input.KeyDown(Key::W) || input.KeyDown(Key::Up))
            player_y -= player_speed * dt;
        if (input.KeyDown(Key::S) || input.KeyDown(Key::Down))
            player_y += player_speed * dt;
        if (input.KeyDown(Key::A) || input.KeyDown(Key::Left))
            player_x -= player_speed * dt;
        if (input.KeyDown(Key::D) || input.KeyDown(Key::Right))
            player_x += player_speed * dt;

        if (input.KeyDown(Key::Q))
            current_zoom = max(0.5f, current_zoom - 1.0f * dt);
        if (input.KeyDown(Key::E))
            current_zoom = min(3.0f, current_zoom + 1.0f * dt);

        float scroll = input.ScrollDelta();
        if (scroll != 0.0f)
            current_zoom = clamp(current_zoom + scroll * 0.025f, 0.5f, 3.0f);

        camera.Follow({player_x, player_y}, 5.0f, dt);
        camera.SetZoom(current_zoom);

        renderer.BeginFrame(camera.GetViewProjection(window));
        renderer.Clear({0.08f, 0.08f, 0.12f, 1.0f});

        const float grid_size = 100.0f;
        const Color grid_light = Color::RGB(40, 40, 50);
        const Color grid_dark = Color::RGB(25, 25, 35);

        glm::vec2 camera_pos = camera.GetPosition();
        glm::ivec2 window_size = window.GetSize();
        float visible_width = window_size.x / current_zoom;
        float visible_height = window_size.y / current_zoom;

        int start_x = (float)(floor((camera_pos.x - visible_width / 2.0f) / grid_size));
        int start_y = (float)(floor((camera_pos.y - visible_height / 2.0f) / grid_size));
        int end_x = (float)(ceil((camera_pos.x + visible_width / 2.0f) / grid_size));
        int end_y = (float)(ceil((camera_pos.y + visible_height / 2.0f) / grid_size));

        for (int x = start_x; x < end_x; ++x)
        {
            for (int y = start_y; y < end_y; ++y)
            {
                Color color = ((x + y) % 2 == 0) ? grid_light : grid_dark;
                renderer.DrawRect(x * grid_size, y * grid_size, grid_size, grid_size, color);
            }
        }

        renderer.DrawRect(player_x - player_size / 2.0f, player_y - player_size / 2.0f, player_size, player_size, Color::RGB(100, 200, 255));

        renderer.EndFrame();
        window.SwapBuffers();
    }
}
