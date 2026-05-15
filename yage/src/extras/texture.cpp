#include <yage/extras/texture.h>
#include <yage/utils.h>
#include <filesystem>
#include <iostream>

#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace yage
{
    static std::string ResolvePath(const std::string &path)
    {
        if (std::filesystem::path(path).is_absolute() || std::filesystem::exists(path))
            return path;

#ifdef __APPLE__
        char buf[1024];
        uint32_t size = sizeof(buf);
        if (_NSGetExecutablePath(buf, &size) == 0)
        {
            auto exeDir = std::filesystem::canonical(buf).parent_path();
            auto candidate = exeDir / path;
            if (std::filesystem::exists(candidate))
                return candidate.string();
        }
#else
        auto exeDir = std::filesystem::canonical("/proc/self/exe").parent_path();
        auto candidate = exeDir / path;
        if (std::filesystem::exists(candidate))
            return candidate.string();
#endif
        return path;
    }

    Texture Texture::FromFile(const char *path, TextureConfig config)
    {
        Texture t;
        int channels;
        std::string resolved = ResolvePath(path);

        stbi_set_flip_vertically_on_load(config.flip_y);
        unsigned char *data = stbi_load(resolved.c_str(), &t.width, &t.height, &channels, 4);

        if (!data)
        {
            std::cerr << "failed to load texture: " << resolved << std::endl;
            return t;
        }

        glGenTextures(1, &t.id);
        glBindTexture(GL_TEXTURE_2D, t.id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, t.width, t.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, config.filter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, config.filter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, config.wrap);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, config.wrap);
        glBindTexture(GL_TEXTURE_2D, 0);

        stbi_image_free(data);
        return t;
    }

    Texture::~Texture()
    {
        if (id)
            glDeleteTextures(1, &id);
    }

    Texture::Texture(Texture &&o) noexcept
        : id(o.id), width(o.width), height(o.height)
    {
        o.id = 0;
    }

    Texture &Texture::operator=(Texture &&o) noexcept
    {
        if (this != &o)
        {
            if (id)
                glDeleteTextures(1, &id);
            id = o.id;
            width = o.width;
            height = o.height;
            o.id = 0;
        }
        return *this;
    }
}