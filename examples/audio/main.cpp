#include <yage/yage.h>
#include <yage/extras/audio.h>
#include <algorithm>

using namespace yage;

int main()
{
    WindowConfig cfg;
    cfg.title = "Audio Example";
    cfg.width = 800;
    cfg.height = 600;
    cfg.gl_version = GLVersion::GL_4_1;

    Window window(cfg);
    Input input(window);
    Renderer renderer(window);
    Audio audio;

    audio.Load("music", "music.mp3");
    audio.Load("hit", "hit.wav");
    audio.Load("pickup", "pickup.wav");

    float master_vol = 1.0f;
    bool music_playing = false;
    bool music_paused = false;

    while (!window.ShouldClose())
    {
        window.Poll(input);

        if (input.KeyPressed(Key::M) && !music_playing)
        {
            AudioParams p;
            p.loop = true;
            p.volume = 0.6f;
            p.fade_in = 1.5f;
            audio.Play("music", p);
            music_playing = true;
            music_paused = false;
        }
        if (input.KeyPressed(Key::N) && music_playing)
        {
            audio.Stop("music");
            music_playing = false;
            music_paused = false;
        }
        if (input.KeyPressed(Key::P) && music_playing)
        {
            if (!music_paused)
            {
                audio.Pause("music");
                music_paused = true;
            }
            else
            {
                audio.Resume("music");
                music_paused = false;
            }
        }
        if (input.KeyPressed(Key::Space))
        {
            AudioParams p;
            p.volume = 0.8f;
            p.pitch = 1.0f;
            audio.Play("hit", p);
        }
        if (input.KeyPressed(Key::E))
        {
            AudioParams p;
            p.volume = 1.0f;
            p.pitch = 1.2f;
            audio.Play("pickup", p);
        }
        if (input.KeyPressed(Key::Up))
        {
            master_vol = std::min(1.0f, master_vol + 0.1f);
            audio.SetMasterVolume(master_vol);
        }
        if (input.KeyPressed(Key::Down))
        {
            master_vol = std::max(0.0f, master_vol - 0.1f);
            audio.SetMasterVolume(master_vol);
        }
        if (input.KeyPressed(Key::Escape))
        {
            audio.StopAll();
            music_playing = false;
            music_paused = false;
        }

        renderer.BeginFrame();
        renderer.Clear({0.05f, 0.05f, 0.09f, 1.0f});

        renderer.DrawRect(104, 74, 600, 492, Color::RGB(4, 4, 9));
        renderer.DrawRect(100, 70, 600, 492, Color::RGB(16, 17, 27));
        renderer.DrawRect(100, 70, 600, 4, Color::RGB(65, 125, 255));

        {
            const int N = 11;
            float heights[N] = {12, 22, 32, 44, 54, 60, 52, 40, 30, 20, 13};
            const float bw = 8.0f, gap = 9.0f;
            const float total_w = N * bw + (N - 1) * gap;
            const float base_x = 400.0f - total_w * 0.5f;
            const float cy = 118.0f;
            Color wc = (music_playing && !music_paused)
                           ? Color::RGB(65, 140, 255)
                           : Color::RGB(35, 40, 65);
            for (int i = 0; i < N; ++i)
                renderer.DrawRect(base_x + i * (bw + gap),
                                  cy - heights[i] * 0.5f, bw, heights[i], wc);
        }

        {
            Color orb, glow;
            if (music_playing && !music_paused)
            {
                orb = Color::RGB(55, 210, 95);
                glow = Color::RGB(18, 55, 28);
            }
            else if (music_playing && music_paused)
            {
                orb = Color::RGB(240, 175, 25);
                glow = Color::RGB(55, 42, 8);
            }
            else
            {
                orb = Color::RGB(210, 55, 70);
                glow = Color::RGB(50, 14, 18);
            }

            renderer.DrawCircle(400, 212, 46, glow);
            renderer.DrawCircle(400, 212, 36, Color::RGB(22, 23, 35));
            renderer.DrawCircle(400, 212, 28, orb);
        }

        {
            const float VX = 148.0f, VY = 276.0f, VW = 504.0f, VH = 14.0f;

            renderer.DrawRect(VX, VY, VW, VH, Color::RGB(20, 22, 36));

            float fw = VW * master_vol;
            if (fw > 0.5f)
                renderer.DrawRect(VX, VY, fw, VH, Color::RGB(70, 130, 255));

            renderer.DrawLine(VX, VY, VX + VW, VY, 1.0f, Color::RGB(38, 42, 68));
            renderer.DrawLine(VX, VY + VH, VX + VW, VY + VH, 1.0f, Color::RGB(38, 42, 68));
            renderer.DrawLine(VX, VY, VX, VY + VH, 1.0f, Color::RGB(38, 42, 68));
            renderer.DrawLine(VX + VW, VY, VX + VW, VY + VH, 1.0f, Color::RGB(38, 42, 68));

            renderer.DrawCircle(VX + fw, VY + VH * 0.5f, 10.0f, Color::RGB(200, 215, 255));
            renderer.DrawCircle(VX + fw, VY + VH * 0.5f, 5.0f, Color::RGB(80, 150, 255));
        }

        renderer.DrawRect(120, 308, 560, 1, Color::RGB(28, 30, 48));

        const float BW = 80.0f, BH = 68.0f, GAP = 18.0f;

        glm::vec2 mouse = input.MousePos();
        bool mouse_click = input.MousePressed(MouseButton::Left);

        auto clicked = [&](float bx, float by) -> bool
        {
            return mouse_click &&
                   mouse.x >= bx && mouse.x <= bx + BW &&
                   mouse.y >= by && mouse.y <= by + BH;
        };

        auto btn = [&](float bx, float by, Color base, Color stripe)
        {
            renderer.DrawRect(bx + 3, by + 4, BW, BH, Color::RGB(4, 4, 8));
            renderer.DrawRect(bx, by, BW, BH, base);
            renderer.DrawRect(bx, by, BW, 3, stripe);
        };

        {
            float rw = 3 * BW + 2 * GAP;
            float rx = 400.0f - rw * 0.5f, ry = 308.0f;

            btn(rx, ry, Color::RGB(24, 52, 138), Color::RGB(78, 138, 255));
            renderer.DrawTriangle(rx + 26, ry + 14,
                                  rx + 26, ry + 54,
                                  rx + 62, ry + 34,
                                  Color::RGB(120, 178, 255));
            if (clicked(rx, ry) && !music_playing)
            {
                AudioParams p;
                p.loop = true;
                p.volume = 0.6f;
                p.fade_in = 1.5f;
                audio.Play("music", p);
                music_playing = true;
                music_paused = false;
            }

            btn(rx + BW + GAP, ry, Color::RGB(108, 26, 42), Color::RGB(218, 68, 88));
            renderer.DrawRect(rx + BW + GAP + 22, ry + 18, 36, 32, Color::RGB(255, 108, 128));
            if (clicked(rx + BW + GAP, ry) && music_playing)
            {
                audio.Stop("music");
                music_playing = false;
                music_paused = false;
            }

            btn(rx + (BW + GAP) * 2, ry, Color::RGB(98, 76, 18), Color::RGB(238, 172, 28));
            float px = rx + (BW + GAP) * 2;
            renderer.DrawRect(px + 20, ry + 18, 12, 32, Color::RGB(255, 208, 78));
            renderer.DrawRect(px + 48, ry + 18, 12, 32, Color::RGB(255, 208, 78));
            if (clicked(px, ry) && music_playing)
            {
                if (!music_paused)
                {
                    audio.Pause("music");
                    music_paused = true;
                }
                else
                {
                    audio.Resume("music");
                    music_paused = false;
                }
            }
        }

        {
            float rw = 2 * BW + GAP;
            float rx = 400.0f - rw * 0.5f, ry = 394.0f;

            btn(rx, ry, Color::RGB(28, 98, 52), Color::RGB(68, 208, 108));
            renderer.DrawCircle(rx + BW * 0.5f, ry + BH * 0.5f, 18.0f, Color::RGB(78, 228, 128));
            renderer.DrawCircleOutline(rx + BW * 0.5f, ry + BH * 0.5f, 26.0f, 2.0f, Color::RGB(58, 188, 98));
            if (clicked(rx, ry))
            {
                AudioParams p;
                p.volume = 0.8f;
                p.pitch = 1.0f;
                audio.Play("hit", p);
            }

            btn(rx + BW + GAP, ry, Color::RGB(78, 28, 118), Color::RGB(172, 68, 228));
            renderer.DrawPolygon(
                MakeRegularPolygon(rx + BW + GAP + BW * 0.5f, ry + BH * 0.5f, 22.0f, 6),
                Color::RGB(198, 98, 255));
            if (clicked(rx + BW + GAP, ry))
            {
                AudioParams p;
                p.volume = 1.0f;
                p.pitch = 1.2f;
                audio.Play("pickup", p);
            }
        }

        {
            float rw = 3 * BW + 2 * GAP;
            float rx = 400.0f - rw * 0.5f, ry = 480.0f;

            btn(rx, ry, Color::RGB(24, 68, 108), Color::RGB(54, 148, 218));
            renderer.DrawTriangle(rx + 40, ry + 14,
                                  rx + 14, ry + 52,
                                  rx + 66, ry + 52,
                                  Color::RGB(88, 178, 255));
            if (clicked(rx, ry))
            {
                master_vol = std::min(1.0f, master_vol + 0.1f);
                audio.SetMasterVolume(master_vol);
            }

            btn(rx + BW + GAP, ry, Color::RGB(16, 48, 82), Color::RGB(38, 108, 168));
            float dx = rx + BW + GAP;
            renderer.DrawTriangle(dx + 40, ry + 52,
                                  dx + 14, ry + 16,
                                  dx + 66, ry + 16,
                                  Color::RGB(58, 138, 208));
            if (clicked(dx, ry))
            {
                master_vol = std::max(0.0f, master_vol - 0.1f);
                audio.SetMasterVolume(master_vol);
            }

            btn(rx + (BW + GAP) * 2, ry, Color::RGB(78, 22, 22), Color::RGB(198, 52, 52));
            float ex = rx + (BW + GAP) * 2;
            renderer.DrawLine(ex + 18, ry + 16, ex + 62, ry + 52, 4.0f, Color::RGB(255, 98, 98));
            renderer.DrawLine(ex + 62, ry + 16, ex + 18, ry + 52, 4.0f, Color::RGB(255, 98, 98));
            if (clicked(ex, ry))
            {
                audio.StopAll();
                music_playing = false;
                music_paused = false;
            }
        }

        renderer.EndFrame();
        window.SwapBuffers();
    }
}