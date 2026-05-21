#include <yage/yage.h>
#include <yage/extras/camera.h>
#include <yage/extras/serializer.h>
#include <yage/extras/network.h>

#include <algorithm>
#include <iostream>
#include <map>
#include <string>

using namespace std;
using namespace yage;

struct PlayerJoined
{
    uint32_t player_id;
    float x;
    float y;
};
struct PlayerLeft
{
    uint32_t player_id;
};
struct PlayerMove
{
    uint32_t player_id;
    float x;
    float y;
};

template <>
struct yage::Serializable<PlayerJoined>
{
    static void Write(Serializer &s, const PlayerJoined &m)
    {
        s.Write(m.player_id);
        s.Write(m.x);
        s.Write(m.y);
    }
    static PlayerJoined Read(Serializer &s)
    {
        return {s.ReadUInt32(), s.ReadFloat(), s.ReadFloat()};
    }
};

template <>
struct yage::Serializable<PlayerLeft>
{
    static void Write(Serializer &s, const PlayerLeft &m) { s.Write(m.player_id); }
    static PlayerLeft Read(Serializer &s) { return {s.ReadUInt32()}; }
};

template <>
struct yage::Serializable<PlayerMove>
{
    static void Write(Serializer &s, const PlayerMove &m)
    {
        s.Write(m.player_id);
        s.Write(m.x);
        s.Write(m.y);
    }
    static PlayerMove Read(Serializer &s)
    {
        return {s.ReadUInt32(), s.ReadFloat(), s.ReadFloat()};
    }
};

static Color PlayerColor(uint32_t id)
{
    static const Color palette[] = {
        Color::RGB(100, 200, 255),
        Color::RGB(255, 160, 80),
        Color::RGB(120, 255, 140),
        Color::RGB(255, 100, 180),
        Color::RGB(255, 230, 80),
    };
    return palette[id % 5];
}

static void DrawGrid(Renderer &renderer, Camera &camera, Window &window, float zoom)
{
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
}

struct RemotePlayer
{
    float x, y;
    float target_x, target_y;
};

// clang-format off

void RunServer(uint16_t port)
{
    cout << "[SERVER] Listening on port " << port << "...\n";

    Server server;
    MessageRouter router;
    map<uint32_t, pair<float, float>> positions;

    server.OnClientConnected([&](uint32_t id)
    {
        cout << "[SERVER] Player " << id << " joined\n";
        float spawn_x = (float)((id % 5) * 120) - 240.0f;
        positions[id] = { spawn_x, 0.0f };

        server.Broadcast(MessageRouter::Pack(PlayerJoined{ id, spawn_x, 0.0f }));

        for (auto& [eid, epos] : positions)
            if (eid != id)
                server.SendTo(id, MessageRouter::Pack(PlayerJoined{ eid, epos.first, epos.second }));
    });

    server.OnClientDisconnected([&](uint32_t id)
    {
        cout << "[SERVER] Player " << id << " left\n";
        positions.erase(id);
        server.Broadcast(MessageRouter::Pack(PlayerLeft{ id }));
    });

    router.On<PlayerMove>([&](const PlayerMove& msg)
    {
        positions[msg.player_id] = { msg.x, msg.y };
        server.Broadcast(MessageRouter::Pack(msg));
    });

    server.OnReceive([&](uint32_t id, const vector<uint8_t>& data)
    {
        router.Dispatch(data);
    });

    server.Listen(port, make_unique<TcpTransport>());

    cout << "[SERVER] Running (Ctrl+C to stop)\n";
    while (true)
        server.Tick(0.016f);
}

void RunClient(const string& host, uint16_t port)
{
    WindowConfig cfg;
    cfg.title = "Networking Demo - Client";
    cfg.width = 1024;
    cfg.height = 768;
    cfg.gl_version = GLVersion::GL_4_1;

    Window window(cfg);
    Input input(window);
    Renderer renderer(window);
    Camera camera;

    const float size = 40.0f;
    const float speed = 300.0f;
    const float lerp_speed = 15.0f;
    float zoom = 1.0f;

    uint32_t my_id = 0;
    float my_x = 0.0f, my_y = 0.0f;
    bool first_join = true;
    map<uint32_t, RemotePlayer> others;

    Client client(make_unique<TcpTransport>());
    MessageRouter router;

    router.On<PlayerJoined>([&](const PlayerJoined& msg)
    {
        if (first_join)
        {
            first_join = false;
            my_id = msg.player_id;
            my_x  = msg.x;
            my_y  = msg.y;
            cout << "[CLIENT] Joined as player " << my_id << "\n";
        }
        else
        {
            others[msg.player_id] = { msg.x, msg.y, msg.x, msg.y };
        }
    });

    router.On<PlayerLeft>([&](const PlayerLeft& msg)
    {
        others.erase(msg.player_id);
    });

    router.On<PlayerMove>([&](const PlayerMove& msg)
    {
        if (msg.player_id != my_id)
        {
            auto& p = others[msg.player_id];
            p.target_x = msg.x;
            p.target_y = msg.y;
        }
    });

    client.OnReceive ([&](const vector<uint8_t>& data) { router.Dispatch(data); });
    client.OnConnected ([] { cout << "[CLIENT] Connected\n"; });
    client.OnDisconnected([] { cout << "[CLIENT] Disconnected\n"; });

    client.Connect(host, port);

    float send_timer = 0.0f;

    while (!window.ShouldClose())
    {
        window.Poll(input);
        float dt = window.GetDt();

        if (input.KeyDown(Key::W) || input.KeyDown(Key::Up))
            my_y -= speed * dt;
        if (input.KeyDown(Key::S) || input.KeyDown(Key::Down))
            my_y += speed * dt;
        if (input.KeyDown(Key::A) || input.KeyDown(Key::Left))
            my_x -= speed * dt;
        if (input.KeyDown(Key::D) || input.KeyDown(Key::Right))
            my_x += speed * dt;

        if (input.KeyDown(Key::Q))
            zoom = max(0.5f, zoom - 1.0f * dt);
        if (input.KeyDown(Key::E))
            zoom = min(3.0f, zoom + 1.0f * dt);
        float scroll = input.ScrollDelta();
        if (scroll != 0)
            zoom = clamp(zoom + scroll * 0.025f, 0.5f, 3.0f);

        send_timer += dt;
        if (send_timer >= 0.033f && my_id != 0)
        {
            client.Send(MessageRouter::Pack(PlayerMove{my_id, my_x, my_y}));
            send_timer = 0.0f;
        }

        for (auto& [id, p] : others)
        {
            p.x += (p.target_x - p.x) * lerp_speed * dt;
            p.y += (p.target_y - p.y) * lerp_speed * dt;
        }

        window.SetTitle("Networking Demo - Player " + to_string(my_id) + "  |  Others: " + to_string(others.size()));

        camera.Follow({my_x, my_y}, 5.0f, dt);
        camera.SetZoom(zoom);

        renderer.BeginFrame(camera.GetViewProjection(window));
        renderer.Clear({0.06f, 0.06f, 0.1f, 1.0f});

        DrawGrid(renderer, camera, window, zoom);

        for (auto& [id, p] : others)
            renderer.DrawRect(p.x - size / 2, p.y - size / 2, size, size, PlayerColor(id));

        if (my_id != 0)
            renderer.DrawRect(my_x - size / 2, my_y - size / 2, size, size, PlayerColor(my_id));

        renderer.EndFrame();
        window.SwapBuffers();

        client.Tick(dt);
    }
}

void RunPeer(uint16_t listen_port, const string& connect_host, uint16_t connect_port)
{
    uint32_t my_id = listen_port;
    float my_x = (float)(listen_port % 5) * 120.0f - 240.0f;
    float my_y = 0.0f;

    WindowConfig cfg;
    cfg.title = "Networking Demo - Peer " + to_string(my_id);
    cfg.width = 1024;
    cfg.height = 768;
    cfg.gl_version = GLVersion::GL_4_1;

    Window window(cfg);
    Input input(window);
    Renderer renderer(window);
    Camera camera;

    const float size = 40.0f;
    const float speed = 300.0f;
    const float lerp_speed = 15.0f;
    float zoom = 1.0f;

    map<uint32_t, RemotePlayer> peers_pos;

    Peer peer([]
              { return make_unique<TcpTransport>(); });
    MessageRouter router;

    router.On<PlayerJoined>([&](const PlayerJoined& msg)
    {
        peers_pos[msg.player_id] = { msg.x, msg.y, msg.x, msg.y };
        cout << "[PEER] Player " << msg.player_id << " announced\n";
    });

    router.On<PlayerMove>([&](const PlayerMove& msg)
    {
        auto& p = peers_pos[msg.player_id];
        p.target_x = msg.x;
        p.target_y = msg.y;
    });

    peer.OnReceive([&](uint32_t id, const vector<uint8_t>& data)
    {
        router.Dispatch(data);
    });

    peer.OnPeerConnected([&](uint32_t slot)
    {
        cout << "[PEER] Slot " << slot << " connected - announcing\n";
        peer.SendTo(slot, MessageRouter::Pack(PlayerJoined{ my_id, my_x, my_y }));
    });

    peer.OnPeerDisconnected([&](uint32_t slot)
    {
        cout << "[PEER] Slot " << slot << " disconnected\n";
    });

    peer.Listen(listen_port);
    cout << "[PEER] Listening on port " << listen_port << "\n";

    if (!connect_host.empty() && connect_port != 0)
    {
        peer.Connect(connect_host, connect_port);
        cout << "[PEER] Connected to " << connect_host << ":" << connect_port << "\n";
    }

    float send_timer = 0.0f;

    while (!window.ShouldClose())
    {
        window.Poll(input);
        float dt = window.GetDt();

        if (input.KeyDown(Key::W) || input.KeyDown(Key::Up))
            my_y -= speed * dt;
        if (input.KeyDown(Key::S) || input.KeyDown(Key::Down))
            my_y += speed * dt;
        if (input.KeyDown(Key::A) || input.KeyDown(Key::Left))
            my_x -= speed * dt;
        if (input.KeyDown(Key::D) || input.KeyDown(Key::Right))
            my_x += speed * dt;

        if (input.KeyDown(Key::Q))
            zoom = max(0.5f, zoom - 1.0f * dt);
        if (input.KeyDown(Key::E))
            zoom = min(3.0f, zoom + 1.0f * dt);
        float scroll = input.ScrollDelta();
        if (scroll != 0)
            zoom = clamp(zoom + scroll * 0.025f, 0.5f, 3.0f);

        send_timer += dt;
        if (send_timer >= 0.033f)
        {
            peer.Broadcast(MessageRouter::Pack(PlayerMove{my_id, my_x, my_y}));
            send_timer = 0.0f;
        }

        for (auto& [id, p] : peers_pos)
        {
            p.x += (p.target_x - p.x) * lerp_speed * dt;
            p.y += (p.target_y - p.y) * lerp_speed * dt;
        }

        window.SetTitle("Networking - Peer " + to_string(my_id) + "  |  Connected: " + to_string(peers_pos.size()));

        camera.Follow({my_x, my_y}, 5.0f, dt);
        camera.SetZoom(zoom);

        renderer.BeginFrame(camera.GetViewProjection(window));
        renderer.Clear({0.06f, 0.06f, 0.1f, 1.0f});

        DrawGrid(renderer, camera, window, zoom);

        for (auto& [id, p] : peers_pos)
            renderer.DrawRect(p.x - size / 2, p.y - size / 2, size, size, PlayerColor(id));

        renderer.DrawRect(my_x - size / 2, my_y - size / 2, size, size, PlayerColor(my_id));

        renderer.EndFrame();
        window.SwapBuffers();

        peer.Tick(dt);
    }
}
// clang-format on

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        cout << "Usage:\n"
             << "  " << argv[0] << " server [port] (default port 8000)\n"
             << "  " << argv[0] << " client [host] [port] (default localhost 8000)\n"
             << "  " << argv[0] << " peer <listen_port> [host connect_port]\n\n"
             << "Examples:\n"
             << "  ./networking server\n"
             << "  ./networking client\n"
             << "  ./networking client localhost 8000\n"
             << "  ./networking peer 8001\n"
             << "  ./networking peer 8002 localhost 8001\n";
        return 1;
    }

    string mode = argv[1];

    if (mode == "server")
    {
        uint16_t port = argc >= 3 ? (uint16_t)stoi(argv[2]) : 8000;
        RunServer(port);
    }
    else if (mode == "client")
    {
        string host = argc >= 3 ? argv[2] : "localhost";
        uint16_t port = argc >= 4 ? (uint16_t)stoi(argv[3]) : 8000;
        RunClient(host, port);
    }
    else if (mode == "peer")
    {
        if (argc < 3)
        {
            cerr << "peer requires a listen port\n";
            return 1;
        }
        uint16_t listen = (uint16_t)stoi(argv[2]);
        string host = argc >= 5 ? argv[3] : "";
        uint16_t connect_p = argc >= 5 ? (uint16_t)stoi(argv[4]) : 0;
        RunPeer(listen, host, connect_p);
    }
    else
    {
        cerr << "Unknown mode: " << mode << "\n";
        return 1;
    }
}