#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <vector>
#include <string>
#include <map>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace yage
{
    enum class SerializationFormat
    {
        JSON,
        Binary
    };

    template <typename T>
    struct Serializable
    {
        static_assert(sizeof(T) == 0,
                      "yage::Serializable<T> is not specialized for this type.\n"
                      "Add a specialization with:\n"
                      "  static void Write(yage::Serializer& s, const T& value);\n"
                      "  static T    Read(yage::Serializer& s);");
    };

    class Serializer
    {
    public:
        explicit Serializer(SerializationFormat fmt = SerializationFormat::Binary);
        ~Serializer();

        void Write(bool value);
        void Write(int value);
        void Write(float value);
        void Write(double value);
        void Write(uint32_t value);
        void Write(uint64_t value);
        void Write(const std::string &value);

        bool ReadBool();
        int ReadInt();
        float ReadFloat();
        double ReadDouble();
        uint32_t ReadUInt32();
        uint64_t ReadUInt64();
        std::string ReadString();

        void BeginArray(size_t size = 0);
        void EndArray();
        void BeginObject();
        void EndObject();
        void BeginField(const std::string &name);
        void EndField();

        std::vector<uint8_t> GetBytes() const;
        void SetBytes(const std::vector<uint8_t> &data);

        SerializationFormat GetFormat() const { return format; }

    private:
        SerializationFormat format;
        std::vector<uint8_t> buffer;
        size_t read_pos = 0;
        nlohmann::json json_root;
        std::vector<nlohmann::json *> json_stack;
        std::string current_field;
        bool reading = false;
        std::vector<size_t> json_read_indices;
    };

    template <typename T>
    void Serialize(Serializer &s, const T &value) { Serializable<T>::Write(s, value); }

    template <typename T>
    T Deserialize(Serializer &s) { return Serializable<T>::Read(s); }

    template <>
    struct Serializable<bool>
    {
        static void Write(Serializer &s, bool v) { s.Write(v); }
        static bool Read(Serializer &s) { return s.ReadBool(); }
    };
    template <>
    struct Serializable<int>
    {
        static void Write(Serializer &s, int v) { s.Write(v); }
        static int Read(Serializer &s) { return s.ReadInt(); }
    };
    template <>
    struct Serializable<float>
    {
        static void Write(Serializer &s, float v) { s.Write(v); }
        static float Read(Serializer &s) { return s.ReadFloat(); }
    };
    template <>
    struct Serializable<double>
    {
        static void Write(Serializer &s, double v) { s.Write(v); }
        static double Read(Serializer &s) { return s.ReadDouble(); }
    };
    template <>
    struct Serializable<uint32_t>
    {
        static void Write(Serializer &s, uint32_t v) { s.Write(v); }
        static uint32_t Read(Serializer &s) { return s.ReadUInt32(); }
    };
    template <>
    struct Serializable<uint64_t>
    {
        static void Write(Serializer &s, uint64_t v) { s.Write(v); }
        static uint64_t Read(Serializer &s) { return s.ReadUInt64(); }
    };
    template <>
    struct Serializable<std::string>
    {
        static void Write(Serializer &s, const std::string &v) { s.Write(v); }
        static std::string Read(Serializer &s) { return s.ReadString(); }
    };

    template <>
    struct Serializable<glm::vec2>
    {
        static void Write(Serializer &s, const glm::vec2 &v)
        {
            s.BeginArray(2);
            s.Write(v.x);
            s.Write(v.y);
            s.EndArray();
        }
        static glm::vec2 Read(Serializer &s)
        {
            s.BeginArray();
            float x = s.ReadFloat(), y = s.ReadFloat();
            s.EndArray();
            return {x, y};
        }
    };
    template <>
    struct Serializable<glm::vec3>
    {
        static void Write(Serializer &s, const glm::vec3 &v)
        {
            s.BeginArray(3);
            s.Write(v.x);
            s.Write(v.y);
            s.Write(v.z);
            s.EndArray();
        }
        static glm::vec3 Read(Serializer &s)
        {
            s.BeginArray();
            float x = s.ReadFloat(), y = s.ReadFloat(), z = s.ReadFloat();
            s.EndArray();
            return {x, y, z};
        }
    };
    template <>
    struct Serializable<glm::vec4>
    {
        static void Write(Serializer &s, const glm::vec4 &v)
        {
            s.BeginArray(4);
            s.Write(v.x);
            s.Write(v.y);
            s.Write(v.z);
            s.Write(v.w);
            s.EndArray();
        }
        static glm::vec4 Read(Serializer &s)
        {
            s.BeginArray();
            float x = s.ReadFloat(), y = s.ReadFloat(), z = s.ReadFloat(), w = s.ReadFloat();
            s.EndArray();
            return {x, y, z, w};
        }
    };
    template <>
    struct Serializable<glm::quat>
    {
        static void Write(Serializer &s, const glm::quat &q)
        {
            s.BeginArray(4);
            s.Write(q.x);
            s.Write(q.y);
            s.Write(q.z);
            s.Write(q.w);
            s.EndArray();
        }
        static glm::quat Read(Serializer &s)
        {
            s.BeginArray();
            float x = s.ReadFloat(), y = s.ReadFloat(), z = s.ReadFloat(), w = s.ReadFloat();
            s.EndArray();
            return {w, x, y, z};
        }
    };

    template <typename T>
    struct Serializable<std::vector<T>>
    {
        static void Write(Serializer &s, const std::vector<T> &vec)
        {
            s.Write(static_cast<uint32_t>(vec.size()));
            s.BeginArray(vec.size());
            for (const auto &item : vec)
                Serialize(s, item);
            s.EndArray();
        }
        static std::vector<T> Read(Serializer &s)
        {
            uint32_t size = s.ReadUInt32();
            std::vector<T> result;
            result.reserve(size);
            s.BeginArray();
            for (uint32_t i = 0; i < size; ++i)
                result.push_back(Deserialize<T>(s));
            s.EndArray();
            return result;
        }
    };
    template <typename K, typename V>
    struct Serializable<std::map<K, V>>
    {
        static void Write(Serializer &s, const std::map<K, V> &map)
        {
            s.Write(static_cast<uint32_t>(map.size()));
            s.BeginObject();
            for (const auto &[k, v] : map)
            {
                s.BeginField(std::to_string(k));
                Serialize(s, v);
                s.EndField();
            }
            s.EndObject();
        }
        static std::map<K, V> Read(Serializer &s)
        {
            uint32_t size = s.ReadUInt32();
            std::map<K, V> result;
            s.BeginObject();
            for (uint32_t i = 0; i < size; ++i)
                result[Deserialize<K>(s)] = Deserialize<V>(s);
            s.EndObject();
            return result;
        }
    };

    template <typename T>
    std::vector<uint8_t> SerializeToBytes(const T &value,
                                          SerializationFormat fmt = SerializationFormat::Binary)
    {
        Serializer s(fmt);
        Serialize(s, value);
        return s.GetBytes();
    }

    template <typename T>
    T DeserializeFromBytes(const std::vector<uint8_t> &data,
                           SerializationFormat fmt = SerializationFormat::Binary)
    {
        Serializer s(fmt);
        s.SetBytes(data);
        return Deserialize<T>(s);
    }
}