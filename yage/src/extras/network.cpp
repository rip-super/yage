#include <yage/extras/network.h>
#include <iostream>

namespace yage
{
    // clang-format off

    TcpTransport::TcpTransport()
        : work_guard(asio::make_work_guard(io_ctx)), connected(false), listening(false)
    {
        size_buf.resize(4);
        io_thread = std::thread([this] { io_ctx.run(); });
    }

    TcpTransport::TcpTransport(asio::ip::tcp::socket::native_handle_type native_fd)
        : work_guard(asio::make_work_guard(io_ctx)), connected(true), listening(false)
    {
        size_buf.resize(4);
        socket = std::make_unique<asio::ip::tcp::socket>(io_ctx);
        asio::error_code ec;
        socket->assign(asio::ip::tcp::v4(), native_fd, ec);
        io_thread = std::thread([this] { io_ctx.run(); });
        asio::post(io_ctx, [this] { StartReceiving(); });
    }

    TcpTransport::~TcpTransport()
    {
        Disconnect();
        work_guard.reset();
        if (io_thread.joinable())
            io_thread.join();
    }

    void TcpTransport::Listen(uint16_t port)
    {
        asio::post(io_ctx, 
            [this, port] {
                acceptor = std::make_unique<asio::ip::tcp::acceptor>(
                    io_ctx,
                    asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)
                );
                listening = true;
                DoAccept(); 
            }
        );
    }

    void TcpTransport::DoAccept()
    {
        if (!acceptor || !acceptor->is_open())
            return;

        acceptor->async_accept(
            [this](const asio::error_code &ec, asio::ip::tcp::socket sock) {
                if (!ec) {
                    auto native_fd = sock.release();
                    auto transport = std::unique_ptr<Transport>(new TcpTransport(native_fd));
                    std::lock_guard<std::mutex> lock(accept_mutex);
                    accept_queue.push(std::move(transport));
                }
                DoAccept(); 
            }
        );
    }

    std::unique_ptr<Transport> TcpTransport::Accept()
    {
        std::lock_guard<std::mutex> lock(accept_mutex);
        if (accept_queue.empty())
            return nullptr;
        auto t = std::move(accept_queue.front());
        accept_queue.pop();
        return t;
    }

    bool TcpTransport::IsListening() const { return listening; }

    void TcpTransport::Connect(const std::string &host, uint16_t port)
    {
        std::promise<asio::error_code> promise;
        auto future = promise.get_future();

        asio::post(io_ctx, 
            [this, host, port, &promise] {
                asio::ip::tcp::resolver resolver(io_ctx);
                asio::error_code ec;
                auto endpoints = resolver.resolve(host, std::to_string(port), ec);
                if (ec) { promise.set_value(ec); return; }

                socket = std::make_unique<asio::ip::tcp::socket>(io_ctx);
                asio::connect(*socket, endpoints, ec);
                if (!ec) {
                    connected = true;
                    StartReceiving();
                }
                promise.set_value(ec); 
            }
        );

        auto ec = future.get();
        if (ec)
            std::cerr << "[TcpTransport] Connect failed: " << ec.message() << "\n";
    }

    void TcpTransport::Send(const std::vector<uint8_t> &data)
    {
        if (!connected)
            return;
        auto packet = std::make_shared<std::vector<uint8_t>>(4 + data.size());
        uint32_t sz = static_cast<uint32_t>(data.size());
        std::memcpy(packet->data(), &sz, 4);
        std::memcpy(packet->data() + 4, data.data(), data.size());

        asio::post(io_ctx, 
            [this, packet] { 
                asio::async_write(
                    *socket, asio::buffer(*packet),
                    [this, packet](const asio::error_code &ec, std::size_t) {
                        if (ec)
                            connected = false;
                    }
                ); 
            }
        );
    }

    void TcpTransport::StartReceiving() { ReadSize(); }

    void TcpTransport::ReadSize()
    {
        if (!socket || !connected)
            return;
    
        asio::async_read(*socket, asio::buffer(size_buf),
            [this](const asio::error_code &ec, std::size_t) {
                if (ec)
                {
                    connected = false;
                    return;
                }

                uint32_t sz = 0;
                std::memcpy(&sz, size_buf.data(), 4);
                ReadBody(sz);
            }
        );
    }

    void TcpTransport::ReadBody(uint32_t size)
    {
        auto buf = std::make_shared<std::vector<uint8_t>>(size);
        asio::async_read(*socket, asio::buffer(*buf),
            [this, buf](const asio::error_code &ec, std::size_t) {
                if (ec)
                {
                    connected = false;
                    return;
                }

                {
                    std::lock_guard<std::mutex> lock(recv_mutex);
                    recv_queue.push(std::move(*buf));
                }

                ReadSize();
            }
        );
    }

    std::vector<uint8_t> TcpTransport::Receive()
    {
        std::lock_guard<std::mutex> lock(recv_mutex);
        if (recv_queue.empty())
            return {};
        auto msg = std::move(recv_queue.front());
        recv_queue.pop();
        return msg;
    }

    bool TcpTransport::IsConnected() const { return connected; }

    void TcpTransport::Disconnect()
    {
        if (!connected && !listening)
            return;
        connected = false;
        listening = false;
        asio::post(io_ctx, 
            [this] {
                asio::error_code ec;
                if (socket)   socket->close(ec);
                if (acceptor) acceptor->close(ec); 
            }
        );
    }

#ifndef _WIN32

    static std::string IpcPath(uint16_t port)
    {
        return "/tmp/yage_ipc_" + std::to_string(port);
    }

    IpcTransport::IpcTransport()
        : work_guard(asio::make_work_guard(io_ctx)), connected(false), listening(false)
    {
        size_buf.resize(4);
        io_thread = std::thread([this] { io_ctx.run(); });
    }

    IpcTransport::IpcTransport(IpcSock::native_handle_type native_fd)
        : work_guard(asio::make_work_guard(io_ctx)), connected(true), listening(false)
    {
        size_buf.resize(4);
        socket = std::make_unique<IpcSock>(io_ctx);
        asio::error_code ec;
        socket->assign(Proto{}, native_fd, ec);
        io_thread = std::thread([this] { io_ctx.run(); });
        asio::post(io_ctx, [this] { StartReceiving(); });
    }

    IpcTransport::~IpcTransport() 
    {
        Disconnect();
        work_guard.reset();
        if (io_thread.joinable()) io_thread.join();
    }


    void IpcTransport::Listen(uint16_t port) 
    {
        socket_path = IpcPath(port);
        ::unlink(socket_path.c_str());
        asio::post(io_ctx, [this] {
            acceptor = std::make_unique<IpcAccep>(
                io_ctx, Proto::endpoint(socket_path));
            listening = true;
            DoAccept();
        });
    }

    void IpcTransport::DoAccept() 
    {
        if (!acceptor || !acceptor->is_open()) return;
        acceptor->async_accept([this](const asio::error_code& ec, IpcSock sock) {
            if (!ec) {
                auto native_fd = sock.release();
                auto transport = std::unique_ptr<Transport>(new IpcTransport(native_fd));
                std::lock_guard<std::mutex> lock(accept_mutex);
                accept_queue.push(std::move(transport));
            }
            DoAccept();
        });
    }

    std::unique_ptr<Transport> IpcTransport::Accept()
    {
        std::lock_guard<std::mutex> lock(accept_mutex);
        if (accept_queue.empty())
            return nullptr;
        auto t = std::move(accept_queue.front());
        accept_queue.pop();
        return t;
    }

    bool IpcTransport::IsListening() const { return listening; }

    void IpcTransport::Connect(const std::string& host, uint16_t port) 
    {
        std::promise<asio::error_code> promise;
        auto future = promise.get_future();

        std::string path = IpcPath(port);
        asio::post(io_ctx, [this, path, &promise] {
            asio::error_code ec;
            socket = std::make_unique<IpcSock>(io_ctx);
            socket->connect(Proto::endpoint(path), ec);
            if (!ec) {
                connected = true;
                StartReceiving();
            }
            promise.set_value(ec);
        });

        auto ec = future.get();
        if (ec)
            std::cerr << "[IpcTransport] Connect failed: " << ec.message() << "\n";
    }

    void IpcTransport::Send(const std::vector<uint8_t>& data) 
    {
        if (!connected) return;
        auto packet = std::make_shared<std::vector<uint8_t>>(4 + data.size());
        uint32_t sz = static_cast<uint32_t>(data.size());
        std::memcpy(packet->data(),     &sz,         4);
        std::memcpy(packet->data() + 4, data.data(), data.size());

        asio::post(io_ctx, [this, packet] {
            asio::async_write(*socket, asio::buffer(*packet),
                [this, packet](const asio::error_code& ec, std::size_t) {
                    if (ec) connected = false;
                });
        });
    }

    void IpcTransport::StartReceiving() { ReadSize(); }

    void IpcTransport::ReadSize() 
    {
        if (!socket || !connected) return;
        asio::async_read(*socket, asio::buffer(size_buf),
            [this](const asio::error_code& ec, std::size_t) {
                if (ec) { connected = false; return; }
                uint32_t sz = 0;
                std::memcpy(&sz, size_buf.data(), 4);
                ReadBody(sz);
            });
    }

    void IpcTransport::ReadBody(uint32_t size) 
    {
        auto buf = std::make_shared<std::vector<uint8_t>>(size);
        asio::async_read(*socket, asio::buffer(*buf),
            [this, buf](const asio::error_code& ec, std::size_t) {
                if (ec) { connected = false; return; }
                {
                    std::lock_guard<std::mutex> lock(recv_mutex);
                    recv_queue.push(std::move(*buf));
                }
                ReadSize();
            });
    }

    std::vector<uint8_t> IpcTransport::Receive()
    {
        std::lock_guard<std::mutex> lock(recv_mutex);
        if (recv_queue.empty())
            return {};
        auto msg = std::move(recv_queue.front());
        recv_queue.pop();
        return msg;
    }

    bool IpcTransport::IsConnected() const { return connected; }

    void IpcTransport::Disconnect() {
        if (!connected && !listening) return;
        connected = false;
        listening  = false;
        asio::post(io_ctx, [this] {
            asio::error_code ec;
            if (socket)   socket->close(ec);
            if (acceptor) acceptor->close(ec);
        });
        if (!socket_path.empty()) ::unlink(socket_path.c_str());
    }
#endif

    Client::Client(std::unique_ptr<Transport> t)
        : transport(std::move(t)) {}

    void Client::Connect(const std::string &host, uint16_t port)
    {
        transport->Connect(host, port);
    }

    void Client::Send(const std::vector<uint8_t> &data)
    {
        if (transport && transport->IsConnected())
            transport->Send(data);
    }

    void Client::Tick(float dt)
    {
        bool now = transport && transport->IsConnected();

        if (now && !was_connected)
        {
            was_connected = true;
            if (on_connected)
                on_connected();
        }
        else if (!now && was_connected)
        {
            was_connected = false;
            if (on_disconnected)
                on_disconnected();
        }

        if (!transport)
            return;
        while (true)
        {
            auto data = transport->Receive();
            if (data.empty())
                break;
            if (on_receive)
                on_receive(data);
        }
    }

    void Server::Listen(uint16_t port, std::unique_ptr<Transport> t)
    {
        listener = std::move(t);
        listener->Listen(port);
        listening = true;
    }

    void Server::Stop()
    {
        listening = false;
        if (listener)
            listener->Disconnect();
    }

    void Server::Broadcast(const std::vector<uint8_t> &data)
    {
        for (auto &[id, t] : clients)
            t->Send(data);
    }

    void Server::SendTo(uint32_t client_id, const std::vector<uint8_t> &data)
    {
        auto it = clients.find(client_id);
        if (it != clients.end())
            it->second->Send(data);
    }

    void Server::Tick(float dt)
    {
        if (listening && listener)
            while (auto conn = listener->Accept())
            {
                uint32_t id = next_id++;
                clients[id] = std::move(conn);
                if (on_connected)
                    on_connected(id);
            }

        for (auto it = clients.begin(); it != clients.end();)
        {
            auto &[id, t] = *it;
            while (true)
            {
                auto data = t->Receive();
                if (data.empty())
                    break;
                if (on_receive)
                    on_receive(id, data);
            }
            if (!t->IsConnected())
            {
                if (on_disconnected)
                    on_disconnected(id);
                it = clients.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }

    Peer::Peer(std::function<std::unique_ptr<Transport>()> f)
        : factory(std::move(f)) {}

    void Peer::Listen(uint16_t port)
    {
        listener = factory();
        listener->Listen(port);
        listening = true;
    }

    void Peer::Connect(const std::string &host, uint16_t port)
    {
        auto t = factory();
        t->Connect(host, port);
        uint32_t id = next_id++;
        peers[id] = std::move(t);
        if (on_connected)
            on_connected(id);
    }

    void Peer::Stop()
    {
        listening = false;
        if (listener)
            listener->Disconnect();
        for (auto &[id, t] : peers)
            t->Disconnect();
    }

    void Peer::Broadcast(const std::vector<uint8_t> &data)
    {
        for (auto &[id, t] : peers)
            t->Send(data);
    }

    void Peer::SendTo(uint32_t peer_id, const std::vector<uint8_t> &data)
    {
        auto it = peers.find(peer_id);
        if (it != peers.end())
            it->second->Send(data);
    }

    void Peer::Tick(float dt)
    {
        if (listening && listener)
            while (auto conn = listener->Accept())
            {
                uint32_t id = next_id++;
                peers[id] = std::move(conn);
                if (on_connected)
                    on_connected(id);
            }

        for (auto it = peers.begin(); it != peers.end();)
        {
            auto &[id, t] = *it;
            while (true)
            {
                auto data = t->Receive();
                if (data.empty())
                    break;
                if (on_receive)
                    on_receive(id, data);
            }
            if (!t->IsConnected())
            {
                if (on_disconnected)
                    on_disconnected(id);
                it = peers.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }

    // clang-format on
}