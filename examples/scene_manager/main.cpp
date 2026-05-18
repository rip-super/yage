#include <yage/yage.h>
#include <yage/extras/scene.h>
#include <yage/extras/scene_manager.h>
#include <cmath>
#include <vector>
using namespace yage;

class MenuScene : public Scene
{
    float time = 0.0f;
    int hovered = -1;

public:
    MenuScene()
    {
        window_config.title = "Scene Manager Demo";
        window_config.width = 800;
        window_config.height = 600;
        window_config.gl_version = GLVersion::GL_4_1;
    }

    void OnStart() override { time = 0.0f; }

    void OnUpdate(float dt) override
    {
        time += dt;
        renderer->Clear({0.05f, 0.04f, 0.10f, 1.0f});

        glm::vec2 m = input->MousePos();

        renderer->DrawRect(0, 0, 800, 115, Color::RGB(14, 11, 32));
        for (int i = 0; i < 11; i++)
        {
            float phase = time + i * 0.57f;
            float cx = 37.0f + i * 73.0f;
            float cy = 57.0f + sinf(phase * 1.2f) * 22.0f;
            float radius = 20.0f + sinf(phase * 2.1f) * 5.0f;
            int sides = 3 + (i % 5);
            float bright = (sinf(phase) + 1.0f) * 0.5f;
            uint8_t r = (uint8_t)(80 + 90 * bright);
            uint8_t b = (uint8_t)(160 + 70 * bright);
            renderer->DrawRotatedPolygon(
                MakeRegularPolygon(cx, cy, radius, sides),
                phase * 45.0f, Color::RGB(r, 50, b));
        }
        renderer->DrawLine(0, 113, 800, 113, 2.0f, Color::RGB(70, 50, 160));

        struct BtnDef
        {
            float y;
            Color base, hover, accent;
        };
        BtnDef btns[3] = {
            {150, Color::RGB(38, 78, 158), Color::RGB(58, 108, 208), Color::RGB(110, 185, 255)},
            {268, Color::RGB(38, 115, 68), Color::RGB(55, 155, 95), Color::RGB(100, 230, 155)},
            {386, Color::RGB(128, 48, 48), Color::RGB(168, 65, 65), Color::RGB(255, 90, 90)},
        };

        hovered = -1;
        for (int i = 0; i < 3; i++)
        {
            float bx = 130, by = btns[i].y, bw = 540, bh = 90;

            bool over = m.x >= bx && m.x <= bx + bw && m.y >= by && m.y <= by + bh;
            if (over)
                hovered = i;

            Color fill = over ? btns[i].hover : btns[i].base;
            Color border = over ? Color::RGB(220, 220, 255) : Color::RGB(70, 70, 115);
            renderer->DrawRect(bx, by, bw, bh, fill);
            renderer->DrawRectOutline(bx, by, bw, bh, 2.5f, border);

            float icx = bx + 44, icy = by + 45;
            renderer->DrawRect(bx + 8, by + 8, 72, 74, Color::RGBA(0, 0, 0, 70));
            if (i == 0)
            {
                renderer->DrawPolygon(MakeRegularPolygon(icx - 10, icy + 10, 16, 6), btns[i].accent);
                renderer->DrawCircle(icx + 18, icy - 12, 12, Color::RGB(255, 210, 80));
                renderer->DrawRect(icx - 24, icy - 24, 18, 18, Color::RGB(255, 100, 155));
            }
            else if (i == 1)
            {
                renderer->DrawRotatedPolygon(MakeRegularPolygon(icx, icy, 22, 5), time * 75.0f, btns[i].accent);
                renderer->DrawRotatedPolygon(MakeRegularPolygon(icx, icy, 11, 3), -time * 120.0f, Color::RGB(255, 175, 80));
            }
            else
            {
                renderer->DrawLine(icx - 18, icy - 18, icx + 18, icy + 18, 4.0f, btns[i].accent);
                renderer->DrawLine(icx + 18, icy - 18, icx - 18, icy + 18, 4.0f, btns[i].accent);
            }

            int dotCount[3] = {9, 10, 4};
            for (int d = 0; d < dotCount[i]; d++)
                renderer->DrawCircle(bx + 105 + d * 16.0f, by + 32, 3.5f, btns[i].accent);
            for (int d = 0; d < dotCount[i] / 2; d++)
                renderer->DrawCircle(bx + 105 + d * 16.0f, by + 56, 2.5f, Color::RGB(120, 120, 155));

            float kx = bx + bw - 58, ky = by + 30;
            renderer->DrawRect(kx, ky, 38, 32, Color::RGB(18, 18, 40));
            renderer->DrawRectOutline(kx, ky, 38, 32, 1.5f, border);
            int kSides[3] = {4, 5, 3};
            renderer->DrawPolygon(MakeRegularPolygon(kx + 19, ky + 16, 9, kSides[i]), btns[i].accent);
        }

        renderer->DrawRect(0, 505, 800, 95, Color::RGB(14, 11, 32));
        renderer->DrawLine(0, 506, 800, 506, 1.5f, Color::RGB(50, 38, 100));
        for (int g = 0; g < 3; g++)
        {
            float gx = 225.0f + g * 175.0f;
            for (int d = 0; d < 3; d++)
                renderer->DrawCircle(gx + d * 14.0f, 553, 4, Color::RGB(75, 75, 120));
        }

        if (input->MouseReleased(MouseButton::Left))
        {
            if (hovered == 0)
                scenes->Set("shapes");
            else if (hovered == 1)
                scenes->Set("demo");
            else if (hovered == 2)
                scenes->Quit();
        }
        if (input->KeyPressed(Key::Num1))
            scenes->Set("shapes");
        if (input->KeyPressed(Key::Num2))
            scenes->Set("demo");
        if (input->KeyPressed(Key::Q))
            scenes->Quit();
    }

    void OnEnd() override {}
};

class ShapesScene : public Scene
{
    float time = 0.0f;

public:
    ShapesScene()
    {
        window_config.title = "Shapes Showcase  |  P: Pause    M: Menu";
        window_config.width = 800;
        window_config.height = 600;
        window_config.gl_version = GLVersion::GL_4_1;
    }

    void OnStart() override { time = 0.0f; }

    void OnUpdate(float dt) override
    {
        time += dt;
        renderer->Clear({0.06f, 0.06f, 0.11f, 1.0f});
        glm::vec2 m = input->MousePos();

        const Color div = Color::RGB(45, 45, 65);
        renderer->DrawLine(0, 200, 800, 200, 1.0f, div);
        renderer->DrawLine(0, 400, 800, 400, 1.0f, div);
        renderer->DrawLine(267, 0, 267, 600, 1.0f, div);
        renderer->DrawLine(534, 0, 534, 600, 1.0f, div);

        renderer->DrawLine(35, 64, 232, 64, 2.0f, Color::RGB(210, 210, 210));
        renderer->DrawLine(35, 88, 232, 88, 5.0f, Color::RGB(255, 120, 60));
        renderer->DrawLine(35, 112, 232, 112, 4.0f, Color::RGB(60, 100, 255), Color::RGB(255, 60, 200));
        renderer->DrawRotatedLine(35, 136, 232, 136, 3.0f, 9.0f, Color::RGB(80, 255, 180));

        renderer->DrawRotatedRect(400 - 75, 100 - 34, 150, 68, time * 30.0f, Color::RGB(100, 80, 255));
        renderer->DrawRotatedRectOutline(400 - 75, 100 - 34, 150, 68, 2.0f, time * 30.0f, Color::RGB(200, 180, 255));

        renderer->DrawCircle(614, 100, 52, Color::RGB(255, 80, 140));
        renderer->DrawCircleOutline(726, 100, 45, 3.0f, Color::RGB(60, 220, 255));

        renderer->DrawTriangle(
            133, 225, 38, 385, 228, 385,
            Color::RGB(255, 50, 50), Color::RGB(50, 50, 255), Color::RGB(255, 220, 0));

        renderer->DrawRotatedPolygon(MakeRegularPolygon(400, 300, 78, 6), time * 42.0f, Color::RGB(255, 200, 80));
        renderer->DrawRotatedPolygonOutline(MakeRegularPolygon(400, 300, 78, 6), 2.5f, time * 42.0f, Color::RGB(255, 240, 180));

        renderer->DrawRect(564, 230, 206, 140, Color::RGB(255, 0, 0), Color::RGB(255, 255, 0), Color::RGB(0, 0, 255), Color::RGB(0, 255, 0));

        renderer->DrawPolygon(
            {{62, 445}, {158, 428}, {228, 468}, {238, 540}, {172, 578}, {78, 572}, {40, 522}},
            Color::RGB(175, 80, 255));

        renderer->DrawRotatedPolygonOutline(
            MakeRegularPolygon(400, 500, 75, 5), 2.5f, time * 55.0f, Color::RGB(80, 255, 205));

        renderer->DrawRotatedPolygon(
            MakeRegularPolygon(667, 500, 75, 7), time * 22.0f,
            {Color::RGB(255, 70, 70), Color::RGB(255, 155, 50), Color::RGB(200, 255, 50),
             Color::RGB(50, 255, 155), Color::RGB(50, 155, 255), Color::RGB(155, 50, 255),
             Color::RGB(255, 50, 200)});

        float mx = m.x, my = m.y;
        renderer->DrawLine(mx - 28, my, mx + 28, my, 1.5f, Color::RGB(255, 255, 80));
        renderer->DrawLine(mx, my - 28, mx, my + 28, 1.5f, Color::RGB(255, 255, 80));
        renderer->DrawCircleOutline(mx, my, 16, 2.0f, Color::RGB(255, 255, 80));

        if (input->KeyPressed(Key::P))
            scenes->Push("pause");
        if (input->KeyPressed(Key::M))
            scenes->Set("menu");
    }

    void OnEnd() override {}
};

struct Ball
{
    float x, y, vx, vy;
    float r;
    float angle, spin;
    int sides;
    Color color;
    Color rimColor;
};

class DemoScene : public Scene
{
    float time = 0.0f;
    std::vector<Ball> balls;

    static const int W = 800, H = 600;

    static float fabs_f(float v) { return v < 0 ? -v : v; }

public:
    DemoScene()
    {
        window_config.title = "Animation Demo  |  P: Pause    M: Menu";
        window_config.width = W;
        window_config.height = H;
        window_config.gl_version = GLVersion::GL_4_1;
    }

    void OnStart() override
    {
        time = 0.0f;
        balls = {
            {150, 150, 165, 210, 42, 0, 60, 0, Color::RGB(255, 80, 100), Color::RGB(255, 200, 210)},
            {600, 100, -145, 185, 36, 0, -85, 4, Color::RGB(80, 200, 255), Color::RGB(200, 240, 255)},
            {400, 300, 95, -225, 52, 0, 48, 5, Color::RGB(255, 205, 50), Color::RGB(255, 240, 180)},
            {200, 450, 205, -92, 30, 0, -105, 6, Color::RGB(155, 80, 255), Color::RGB(215, 180, 255)},
            {650, 400, -185, 125, 46, 0, 72, 3, Color::RGB(80, 255, 155), Color::RGB(200, 255, 220)},
            {100, 300, 130, 250, 26, 0, 0, 0, Color::RGB(255, 150, 50), Color::RGB(255, 220, 180)},
            {700, 300, -105, -205, 56, 0, 28, 7, Color::RGB(255, 60, 200), Color::RGB(255, 190, 240)},
            {350, 500, 220, -80, 32, 0, -65, 8, Color::RGB(100, 220, 255), Color::RGB(200, 245, 255)},
        };
    }

    void OnUpdate(float dt) override
    {
        time += dt;
        renderer->Clear({0.05f, 0.04f, 0.09f, 1.0f});

        for (int i = 0; i < 4; i++)
        {
            float cx = (i & 1) ? (float)W : 0.0f;
            float cy = (i & 2) ? (float)H : 0.0f;
            for (int ring = 1; ring <= 3; ring++)
            {
                float r = 50.0f + ring * 40.0f;
                float alpha = 0.12f + 0.06f * sinf(time * 0.8f + i + ring);
                uint8_t a = (uint8_t)(alpha * 255);
                renderer->DrawCircleOutline(cx, cy, r, 1.0f, Color::RGBA(90, 80, 180, a));
            }

            float a1 = time * (0.9f + i * 0.15f);
            float or1 = 70.0f;
            renderer->DrawCircle(cx + cosf(a1) * or1, cy + sinf(a1) * or1, 7, Color::RGB(200, 200, 255));
        }

        for (auto &b : balls)
        {
            b.x += b.vx * dt;
            b.y += b.vy * dt;
            b.angle += b.spin * dt;

            if (b.x - b.r < 0)
            {
                b.x = b.r;
                b.vx = fabs_f(b.vx);
            }
            if (b.x + b.r > W)
            {
                b.x = W - b.r;
                b.vx = -fabs_f(b.vx);
            }
            if (b.y - b.r < 0)
            {
                b.y = b.r;
                b.vy = fabs_f(b.vy);
            }
            if (b.y + b.r > H)
            {
                b.y = H - b.r;
                b.vy = -fabs_f(b.vy);
            }
        }

        for (auto &b : balls)
        {
            if (b.sides >= 3)
            {
                renderer->DrawRotatedPolygon(MakeRegularPolygon(b.x, b.y, b.r, b.sides), b.angle, b.color);
                renderer->DrawRotatedPolygonOutline(MakeRegularPolygon(b.x, b.y, b.r, b.sides), 2.0f, b.angle, b.rimColor);
            }
            else
            {
                renderer->DrawCircle(b.x, b.y, b.r, b.color);
                renderer->DrawCircleOutline(b.x, b.y, b.r, 2.0f, b.rimColor);
            }
        }

        if (input->KeyPressed(Key::P))
            scenes->Push("pause");
        if (input->KeyPressed(Key::M))
            scenes->Set("menu");
    }

    void OnEnd() override {}
};

class PauseScene : public Scene
{
    float time = 0.0f;

public:
    void OnStart() override { time = 0.0f; }

    void OnUpdate(float dt) override
    {
        time += dt;

        renderer->Clear({0.04f, 0.03f, 0.09f, 1.0f});

        renderer->DrawRect(215, 155, 370, 290, Color::RGB(20, 18, 48));
        renderer->DrawRectOutline(215, 155, 370, 290, 3.0f, Color::RGB(120, 100, 255));

        renderer->DrawPolygon(MakeRegularPolygon(400, 263, 55, 6), Color::RGB(72, 62, 168));
        renderer->DrawRotatedPolygonOutline(MakeRegularPolygon(400, 263, 55, 6), 2.5f, 0.0f, Color::RGB(180, 160, 255));
        renderer->DrawRect(378, 235, 17, 55, Color::RGB(220, 205, 255));
        renderer->DrawRect(405, 235, 17, 55, Color::RGB(220, 205, 255));

        float pulse = (sinf(time * 3.2f) + 1.0f) * 0.5f;
        uint8_t g = (uint8_t)(85 + 85 * pulse);
        renderer->DrawRect(248, 355, 135, 55, Color::RGB(30, g, 50));
        renderer->DrawRectOutline(248, 355, 135, 55, 2.0f, Color::RGB(80, 205, 110));
        renderer->DrawPolygon(MakeRegularPolygon(315, 382, 14, 5), Color::RGB(160, 255, 185));

        renderer->DrawRect(417, 355, 135, 55, Color::RGB(30, 42, 95));
        renderer->DrawRectOutline(417, 355, 135, 55, 2.0f, Color::RGB(100, 135, 220));
        renderer->DrawPolygon(MakeRegularPolygon(484, 382, 14, 4), Color::RGB(160, 185, 255));

        if (input->KeyPressed(Key::P))
            scenes->Pop();
        if (input->KeyPressed(Key::M))
        {
            scenes->Pop();
            scenes->Set("menu");
        }
    }

    void OnEnd() override {}
};

int main()
{
    SceneManager manager;
    manager.RegisterScene("menu", new MenuScene());
    manager.RegisterScene("shapes", new ShapesScene());
    manager.RegisterScene("demo", new DemoScene());
    manager.RegisterScene("pause", new PauseScene());

    manager.Run("menu");
    return 0;
}