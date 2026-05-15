#pragma once
#include <glad/glad.h>
#include <yage/utils.h>

namespace yage
{
    class Texture
    {
    public:
        static Texture FromFile(const char *path, TextureConfig config = {});

        ~Texture();
        Texture(const Texture &) = delete;
        Texture &operator=(const Texture &) = delete;
        Texture(Texture &&) noexcept;
        Texture &operator=(Texture &&) noexcept;

        int GetWidth() const { return width; }
        int GetHeight() const { return height; }
        GLuint GetID() const { return id; }

    private:
        Texture() = default;
        GLuint id = 0;
        int width = 0, height = 0;
    };
}