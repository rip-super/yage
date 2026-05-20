#include <yage/extras/serializer.h>
#include <cstring>
#include <stdexcept>

namespace
{
    template <typename T>
    void BinWrite(std::vector<uint8_t> &buf, T val)
    {
        const uint8_t *p = reinterpret_cast<const uint8_t *>(&val);
        buf.insert(buf.end(), p, p + sizeof(T));
    }

    template <typename T>
    T BinRead(const std::vector<uint8_t> &buf, size_t &pos)
    {
        if (pos + sizeof(T) > buf.size())
            throw std::runtime_error("Serializer: buffer underrun");
        T val;
        std::memcpy(&val, buf.data() + pos, sizeof(T));
        pos += sizeof(T);
        return val;
    }

    void JsonInsert(std::vector<nlohmann::json *> &stack, const std::string &field, nlohmann::json val)
    {
        auto &top = *stack.back();
        if (top.is_array())
            top.push_back(std::move(val));
        else
            top[field] = std::move(val);
    }

    nlohmann::json &JsonNext(std::vector<nlohmann::json *> &stack, std::vector<size_t> &indices, const std::string &field)
    {
        auto &top = *stack.back();
        if (top.is_array())
            return top.at(indices.back()++);
        else
            return top.at(field);
    }
}

namespace yage
{
    Serializer::Serializer(SerializationFormat fmt)
        : format(fmt), read_pos(0), reading(false)
    {
        if (format == SerializationFormat::JSON)
        {
            json_root = nlohmann::json::array();
            json_stack.push_back(&json_root);
        }
    }

    Serializer::~Serializer() = default;

    void Serializer::Write(bool value)
    {
        if (format == SerializationFormat::Binary)
            BinWrite(buffer, (uint8_t)(value ? 1 : 0));
        else
            JsonInsert(json_stack, current_field, value);
    }

    void Serializer::Write(int value)
    {
        if (format == SerializationFormat::Binary)
            BinWrite(buffer, value);
        else
            JsonInsert(json_stack, current_field, value);
    }

    void Serializer::Write(float value)
    {
        if (format == SerializationFormat::Binary)
            BinWrite(buffer, value);
        else
            JsonInsert(json_stack, current_field, value);
    }

    void Serializer::Write(double value)
    {
        if (format == SerializationFormat::Binary)
            BinWrite(buffer, value);
        else
            JsonInsert(json_stack, current_field, value);
    }

    void Serializer::Write(uint32_t value)
    {
        if (format == SerializationFormat::Binary)
            BinWrite(buffer, value);
        else
            JsonInsert(json_stack, current_field, value);
    }

    void Serializer::Write(uint64_t value)
    {
        if (format == SerializationFormat::Binary)
            BinWrite(buffer, value);
        else
            JsonInsert(json_stack, current_field, value);
    }

    void Serializer::Write(const std::string &value)
    {
        if (format == SerializationFormat::Binary)
        {
            uint32_t len = (uint32_t)(value.size());
            BinWrite(buffer, len);
            buffer.insert(buffer.end(), value.begin(), value.end());
        }
        else
        {
            JsonInsert(json_stack, current_field, value);
        }
    }

    bool Serializer::ReadBool()
    {
        if (format == SerializationFormat::Binary)
            return BinRead<uint8_t>(buffer, read_pos) != 0;
        else
            return JsonNext(json_stack, json_read_indices, current_field).get<bool>();
    }

    int Serializer::ReadInt()
    {
        if (format == SerializationFormat::Binary)
            return BinRead<int>(buffer, read_pos);
        else
            return JsonNext(json_stack, json_read_indices, current_field).get<int>();
    }

    float Serializer::ReadFloat()
    {
        if (format == SerializationFormat::Binary)
            return BinRead<float>(buffer, read_pos);
        else
            return JsonNext(json_stack, json_read_indices, current_field).get<float>();
    }

    double Serializer::ReadDouble()
    {
        if (format == SerializationFormat::Binary)
            return BinRead<double>(buffer, read_pos);
        else
            return JsonNext(json_stack, json_read_indices, current_field).get<double>();
    }

    uint32_t Serializer::ReadUInt32()
    {
        if (format == SerializationFormat::Binary)
            return BinRead<uint32_t>(buffer, read_pos);
        else
            return JsonNext(json_stack, json_read_indices, current_field).get<uint32_t>();
    }

    uint64_t Serializer::ReadUInt64()
    {
        if (format == SerializationFormat::Binary)
            return BinRead<uint64_t>(buffer, read_pos);
        else
            return JsonNext(json_stack, json_read_indices, current_field).get<uint64_t>();
    }

    std::string Serializer::ReadString()
    {
        if (format == SerializationFormat::Binary)
        {
            uint32_t len = BinRead<uint32_t>(buffer, read_pos);
            if (read_pos + len > buffer.size())
                throw std::runtime_error("Serializer: string overrun");
            std::string result(buffer.begin() + read_pos,
                               buffer.begin() + read_pos + len);
            read_pos += len;
            return result;
        }
        else
        {
            return JsonNext(json_stack, json_read_indices, current_field).get<std::string>();
        }
    }

    void Serializer::BeginArray(size_t size)
    {
        if (format == SerializationFormat::Binary)
            return;

        if (!reading)
        {
            auto &top = *json_stack.back();
            if (top.is_array())
            {
                top.push_back(nlohmann::json::array());
                json_stack.push_back(&top.back());
            }
            else
            {
                top[current_field] = nlohmann::json::array();
                json_stack.push_back(&top[current_field]);
            }
        }
        else
        {
            auto &node = JsonNext(json_stack, json_read_indices, current_field);
            json_stack.push_back(&node);
            json_read_indices.push_back(0);
        }
    }

    void Serializer::EndArray()
    {
        if (format == SerializationFormat::Binary)
            return;
        json_stack.pop_back();
        if (reading)
            json_read_indices.pop_back();
    }

    void Serializer::BeginObject()
    {
        if (format == SerializationFormat::Binary)
            return;

        if (!reading)
        {
            auto &top = *json_stack.back();
            if (top.is_array())
            {
                top.push_back(nlohmann::json::object());
                json_stack.push_back(&top.back());
            }
            else
            {
                top[current_field] = nlohmann::json::object();
                json_stack.push_back(&top[current_field]);
            }
        }
        else
        {
            auto &node = JsonNext(json_stack, json_read_indices, current_field);
            json_stack.push_back(&node);
        }
    }

    void Serializer::EndObject()
    {
        if (format == SerializationFormat::Binary)
            return;
        json_stack.pop_back();
    }

    void Serializer::BeginField(const std::string &name)
    {
        if (format == SerializationFormat::Binary)
            return;
        current_field = name;
    }

    void Serializer::EndField()
    {
        if (format == SerializationFormat::Binary)
            return;
        current_field.clear();
    }

    std::vector<uint8_t> Serializer::GetBytes() const
    {
        if (format == SerializationFormat::Binary)
            return buffer;

        std::string str = json_root.dump(2);
        return std::vector<uint8_t>(str.begin(), str.end());
    }

    void Serializer::SetBytes(const std::vector<uint8_t> &data)
    {
        if (format == SerializationFormat::Binary)
        {
            buffer = data;
            read_pos = 0;
            return;
        }

        std::string str(data.begin(), data.end());
        json_root = nlohmann::json::parse(str);

        json_stack.clear();
        json_stack.push_back(&json_root);

        json_read_indices.clear();
        if (json_root.is_array())
            json_read_indices.push_back(0);

        reading = true;
    }
}
