#include <yage/yage.h>
#include <yage/extras/camera.h>
#include <yage/extras/serializer.h>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <random>
#include <set>

using namespace std;
using namespace yage;

struct Settings
{
    float volume = 1.0f;
    float zoom = 1.0f;
    float player_x = 0.0f;
    float player_y = 0.0f;
    int high_score = 0;
};

// clang-format off
template <>
struct yage::Serializable<Settings>
{
    static void Write(Serializer& s, const Settings& v)
    {
        s.BeginField("volume");      s.Write(v.volume);      s.EndField();
        s.BeginField("zoom");        s.Write(v.zoom);        s.EndField();
        s.BeginField("player_x");    s.Write(v.player_x);    s.EndField();
        s.BeginField("player_y");    s.Write(v.player_y);    s.EndField();
        s.BeginField("high_score");  s.Write(v.high_score);  s.EndField();
    }

    static Settings Read(Serializer& s)
    {
        Settings v;
        s.BeginField("volume");      v.volume      = s.ReadFloat();  s.EndField();
        s.BeginField("zoom");        v.zoom        = s.ReadFloat();  s.EndField();
        s.BeginField("player_x");    v.player_x    = s.ReadFloat();  s.EndField();
        s.BeginField("player_y");    v.player_y    = s.ReadFloat();  s.EndField();
        s.BeginField("high_score");  v.high_score  = s.ReadInt();    s.EndField();
        return v;
    }
};
// clang-format on

static void Save(const Settings &settings, const string &path)
{
    auto bytes = SerializeToBytes(settings, SerializationFormat::Binary);
    // auto bytes = SerializeToBytes(settings, SerializationFormat::JSON);

    ofstream file(path, ios::binary);
    if (!file)
    {
        cerr << "[SAVE] Failed to open " << path << "\n";
        return;
    }
    file.write(reinterpret_cast<const char *>(bytes.data()), bytes.size());
    cout << "[SAVE] Saved to " << path << "\n";
}

static Settings Load(const string &path)
{
    if (!filesystem::exists(path))
    {
        cout << "[LOAD] No save file found at " << path << ", using defaults\n";
        return {};
    }

    ifstream file(path, ios::binary);
    if (!file)
    {
        cerr << "[LOAD] Failed to open " << path << "\n";
        return {};
    }

    vector<uint8_t> bytes{istreambuf_iterator<char>(file), istreambuf_iterator<char>()};

    Settings settings = DeserializeFromBytes<Settings>(bytes, SerializationFormat::Binary);
    // Settings settings = DeserializeFromBytes<Settings>(bytes, SerializationFormat::JSON);

    cout << "[LOAD] Loaded from " << path << "\n";
    return settings;
}

struct Coin
{
    glm::vec2 pos;
    bool collected = false;
};

static const float CHUNK_SIZE = 400.0f;
static const int COINS_PER_CHUNK = 1;
static const int SPAWN_RADIUS = 3;

static vector<Coin> GenerateChunk(int cx, int cy)
{
    uint32_t seed = (uint32_t)(cx * 2654435761u ^ cy * 2246822519u);
    mt19937 rng(seed);

    const float margin = 24.0f;
    uniform_real_distribution<float> rx(cx * CHUNK_SIZE + margin, (cx + 1) * CHUNK_SIZE - margin);
    uniform_real_distribution<float> ry(cy * CHUNK_SIZE + margin, (cy + 1) * CHUNK_SIZE - margin);

    vector<Coin> coins;
    for (int i = 0; i < COINS_PER_CHUNK; ++i)
        coins.push_back({{rx(rng), ry(rng)}});
    return coins;
}

int main(int argc, char **argv)
{
    const string save_path = argc >= 2 ? argv[1] : "save.bin";
    // const string save_path = argc >= 2 ? argv[1] : "save.json";

    Settings settings = Load(save_path);

    WindowConfig cfg;
    cfg.title = "Serializer Demo";
    cfg.width = 1024;
    cfg.height = 768;
    cfg.gl_version = GLVersion::GL_4_1;

    Window window(cfg);
    Input input(window);
    Renderer renderer(window);
    Camera camera;

    const float player_size = 40.0f;
    const float speed = 300.0f;
    float zoom = settings.zoom;
    glm::vec2 player_pos = {settings.player_x, settings.player_y};
    int high_score = settings.high_score;
    int score = 0;

    vector<Coin> coins;
    set<pair<int, int>> spawned_chunks;

    cout << "[INFO] WASD to move, Q/E to zoom, Esc to save and quit\n";
    cout << "[INFO] Save path: " << save_path << "\n";
    cout << "[INFO] High score: " << high_score << "\n";

    while (!window.ShouldClose())
    {
        window.Poll(input);
        float dt = window.GetDt();

        if (input.KeyDown(Key::W) || input.KeyDown(Key::Up))
            player_pos.y -= speed * dt;
        if (input.KeyDown(Key::S) || input.KeyDown(Key::Down))
            player_pos.y += speed * dt;
        if (input.KeyDown(Key::A) || input.KeyDown(Key::Left))
            player_pos.x -= speed * dt;
        if (input.KeyDown(Key::D) || input.KeyDown(Key::Right))
            player_pos.x += speed * dt;

        if (input.KeyDown(Key::Q))
            zoom = max(0.5f, zoom - 1.0f * dt);
        if (input.KeyDown(Key::E))
            zoom = min(3.0f, zoom + 1.0f * dt);
        float scroll = input.ScrollDelta();
        if (scroll != 0)
            zoom = clamp(zoom + scroll * 0.025f, 0.5f, 3.0f);

        int pcx = (int)floor(player_pos.x / CHUNK_SIZE);
        int pcy = (int)floor(player_pos.y / CHUNK_SIZE);

        for (int cx = pcx - SPAWN_RADIUS; cx <= pcx + SPAWN_RADIUS; ++cx)
        {
            for (int cy = pcy - SPAWN_RADIUS; cy <= pcy + SPAWN_RADIUS; ++cy)
            {
                auto key = make_pair(cx, cy);
                if (spawned_chunks.find(key) == spawned_chunks.end())
                {
                    spawned_chunks.insert(key);
                    auto new_coins = GenerateChunk(cx, cy);
                    coins.insert(coins.end(), new_coins.begin(), new_coins.end());
                }
            }
        }

        for (auto &coin : coins)
        {
            if (coin.collected)
                continue;
            float dx = player_pos.x - coin.pos.x;
            float dy = player_pos.y - coin.pos.y;
            if (dx * dx + dy * dy < 40.0f * 40.0f)
            {
                coin.collected = true;
                score++;
                if (score > high_score)
                    high_score = score;
            }
        }

        if (coins.size() > 500)
        {
            // clang-format off
            coins.erase(
                remove_if(coins.begin(), coins.end(), 
                    [&](const Coin &c) {
                        if (!c.collected) return false;
                        float dx = c.pos.x - player_pos.x;
                        float dy = c.pos.y - player_pos.y;
                        return dx * dx + dy * dy > (CHUNK_SIZE * SPAWN_RADIUS * 2) * (CHUNK_SIZE * SPAWN_RADIUS * 2); 
                    }),
                coins.end()
            );
            // clang-format on
        }

        if (input.KeyPressed(Key::Escape))
        {
            settings.player_x = player_pos.x;
            settings.player_y = player_pos.y;
            settings.zoom = zoom;
            settings.high_score = high_score;
            Save(settings, save_path);
            break;
        }

        camera.Follow(player_pos, 5.0f, dt);
        camera.SetZoom(zoom);

        window.SetTitle("Serializer Demo  |  Score: " + to_string(score) + "  |  Best: " + to_string(high_score));

        renderer.BeginFrame(camera.GetViewProjection(window));
        renderer.Clear({0.06f, 0.06f, 0.1f, 1.0f});

        const float grid = 100.0f;
        glm::vec2 cam = camera.GetPosition();
        glm::ivec2 win = window.GetSize();
        float vw = win.x / zoom, vh = win.y / zoom;
        int sx = (int)floor((cam.x - vw / 2) / grid);
        int sy = (int)floor((cam.y - vh / 2) / grid);
        int ex = (int)ceil((cam.x + vw / 2) / grid);
        int ey = (int)ceil((cam.y + vh / 2) / grid);
        for (int x = sx; x < ex; ++x)
            for (int y = sy; y < ey; ++y)
                renderer.DrawRect(x * grid, y * grid, grid, grid, ((x + y) % 2 == 0) ? Color::RGB(40, 40, 50) : Color::RGB(25, 25, 35));

        for (const auto &coin : coins)
            if (!coin.collected)
                renderer.DrawRect(coin.pos.x - 12, coin.pos.y - 12, 24, 24, Color::RGB(255, 220, 50));

        renderer.DrawRect(player_pos.x - player_size / 2, player_pos.y - player_size / 2, player_size, player_size, Color::RGB(100, 200, 255));

        renderer.EndFrame();
        window.SwapBuffers();
    }

    cout << "[INFO] Final score: " << score << "  |  High score: " << high_score << "\n";
}