#pragma once
#include <yage/extras/serializer.h>
#include <asio.hpp>
#include <functional>
#include <map>
#include <memory>
#include <cstdint>
#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include <queue>
#include <cstring>
#include <typeinfo>

namespace yage
{
    class Transport
    {
    public:
        virtual ~Transport() = default;

        virtual void Listen(uint16_t port) = 0;
        virtual std::unique_ptr<Transport> Accept() = 0;
        virtual bool IsListening() const = 0;

        virtual void Connect(const std::string &host, uint16_t port) = 0;

        virtual void Send(const std::vector<uint8_t> &data) = 0;
        virtual std::vector<uint8_t> Receive() = 0;
        virtual bool IsConnected() const = 0;

        virtual void Disconnect() = 0;
    };

    class TcpTransport : public Transport
    {
    public:
        TcpTransport();
        ~TcpTransport();

        void Listen(uint16_t port) override;
        std::unique_ptr<Transport> Accept() override;
        bool IsListening() const override;

        void Connect(const std::string &host, uint16_t port) override;

        void Send(const std::vector<uint8_t> &data) override;
        std::vector<uint8_t> Receive() override;
        bool IsConnected() const override;
        void Disconnect() override;

    private:
        explicit TcpTransport(asio::ip::tcp::socket::native_handle_type native_fd);

        void DoAccept();
        void StartReceiving();
        void ReadSize();
        void ReadBody(uint32_t size);

        using WorkGuard = asio::executor_work_guard<asio::io_context::executor_type>;

        asio::io_context io_ctx;
        WorkGuard work_guard;
        std::thread io_thread;

        std::unique_ptr<asio::ip::tcp::socket> socket;
        std::unique_ptr<asio::ip::tcp::acceptor> acceptor;

        std::queue<std::vector<uint8_t>> recv_queue;
        std::mutex recv_mutex;
        std::queue<std::unique_ptr<Transport>> accept_queue;
        std::mutex accept_mutex;

        std::vector<uint8_t> size_buf;
        bool connected = false;
        bool listening = false;
    };

#ifndef _WIN32
    class IpcTransport : public Transport
    {
    public:
        IpcTransport();
        ~IpcTransport();

        void Listen(uint16_t port) override;
        std::unique_ptr<Transport> Accept() override;
        bool IsListening() const override;

        void Connect(const std::string &host, uint16_t port) override;

        void Send(const std::vector<uint8_t> &data) override;
        std::vector<uint8_t> Receive() override;
        bool IsConnected() const override;
        void Disconnect() override;

    private:
        using Proto = asio::local::stream_protocol;
        using IpcSock = Proto::socket;
        using IpcAccep = Proto::acceptor;
        using WorkGuard = asio::executor_work_guard<asio::io_context::executor_type>;

        explicit IpcTransport(IpcSock::native_handle_type native_fd);

        void DoAccept();
        void StartReceiving();
        void ReadSize();
        void ReadBody(uint32_t size);

        asio::io_context io_ctx;
        WorkGuard work_guard;
        std::thread io_thread;

        std::unique_ptr<IpcSock> socket;
        std::unique_ptr<IpcAccep> acceptor;

        std::queue<std::vector<uint8_t>> recv_queue;
        std::mutex recv_mutex;
        std::queue<std::unique_ptr<Transport>> accept_queue;
        std::mutex accept_mutex;

        std::vector<uint8_t> size_buf;
        std::string socket_path;
        bool connected = false;
        bool listening = false;
    };
#endif

    class Client
    {
    public:
        explicit Client(std::unique_ptr<Transport> transport);
        ~Client() = default;

        void Connect(const std::string &host, uint16_t port);
        void Send(const std::vector<uint8_t> &data);
        void Tick(float dt);

        bool IsConnected() const { return transport && transport->IsConnected(); }
        void Disconnect()
        {
            if (transport)
                transport->Disconnect();
        }

        void OnReceive(std::function<void(const std::vector<uint8_t> &)> cb) { on_receive = cb; }
        void OnConnected(std::function<void()> cb) { on_connected = cb; }
        void OnDisconnected(std::function<void()> cb) { on_disconnected = cb; }

    private:
        std::unique_ptr<Transport> transport;
        std::function<void(const std::vector<uint8_t> &)> on_receive;
        std::function<void()> on_connected;
        std::function<void()> on_disconnected;
        bool was_connected = false;
    };

    class Server
    {
    public:
        Server() = default;
        ~Server() = default;

        void Listen(uint16_t port, std::unique_ptr<Transport> transport);
        void Stop();
        void Tick(float dt);

        void Broadcast(const std::vector<uint8_t> &data);
        void SendTo(uint32_t client_id, const std::vector<uint8_t> &data);

        void OnReceive(std::function<void(uint32_t, const std::vector<uint8_t> &)> cb) { on_receive = cb; }
        void OnClientConnected(std::function<void(uint32_t)> cb) { on_connected = cb; }
        void OnClientDisconnected(std::function<void(uint32_t)> cb) { on_disconnected = cb; }

    private:
        std::map<uint32_t, std::unique_ptr<Transport>> clients;
        std::unique_ptr<Transport> listener;
        std::function<void(uint32_t, const std::vector<uint8_t> &)> on_receive;
        std::function<void(uint32_t)> on_connected;
        std::function<void(uint32_t)> on_disconnected;
        uint32_t next_id = 1;
        bool listening = false;
    };

    class Peer
    {
    public:
        explicit Peer(std::function<std::unique_ptr<Transport>()> transport_factory);
        ~Peer() = default;

        void Listen(uint16_t port);
        void Connect(const std::string &host, uint16_t port);
        void Stop();
        void Tick(float dt);

        void Broadcast(const std::vector<uint8_t> &data);
        void SendTo(uint32_t peer_id, const std::vector<uint8_t> &data);

        void OnReceive(std::function<void(uint32_t, const std::vector<uint8_t> &)> cb) { on_receive = cb; }
        void OnPeerConnected(std::function<void(uint32_t)> cb) { on_connected = cb; }
        void OnPeerDisconnected(std::function<void(uint32_t)> cb) { on_disconnected = cb; }

    private:
        std::function<std::unique_ptr<Transport>()> factory;
        std::map<uint32_t, std::unique_ptr<Transport>> peers;
        std::unique_ptr<Transport> listener;
        std::function<void(uint32_t, const std::vector<uint8_t> &)> on_receive;
        std::function<void(uint32_t)> on_connected;
        std::function<void(uint32_t)> on_disconnected;
        uint32_t next_id = 1;
        bool listening = false;
    };

    class MessageRouter
    {
    public:
        template <typename T>
        void On(std::function<void(const T &)> cb)
        {
            uint32_t tid = TypeId<T>();
            handlers[tid] = [cb](const std::vector<uint8_t> &payload)
            {
                try
                {
                    cb(DeserializeFromBytes<T>(payload));
                }
                catch (...)
                {
                }
            };
        }

        void Dispatch(const std::vector<uint8_t> &frame)
        {
            if (frame.size() < 4)
                return;
            uint32_t tid = 0;
            std::memcpy(&tid, frame.data(), 4);
            auto it = handlers.find(tid);
            if (it != handlers.end())
                it->second({frame.begin() + 4, frame.end()});
        }

        template <typename T>
        static std::vector<uint8_t> Pack(const T &msg)
        {
            auto payload = SerializeToBytes(msg);
            uint32_t tid = TypeId<T>();
            std::vector<uint8_t> frame(4 + payload.size());
            std::memcpy(frame.data(), &tid, 4);
            std::memcpy(frame.data() + 4, payload.data(), payload.size());
            return frame;
        }

    private:
        template <typename T>
        static uint32_t TypeId()
        {
            return static_cast<uint32_t>(typeid(T).hash_code());
        }

        std::map<uint32_t, std::function<void(const std::vector<uint8_t> &)>> handlers;
    };
}