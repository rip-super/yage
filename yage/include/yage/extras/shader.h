#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>

namespace yage
{
    class Shader
    {
    public:
        unsigned int id = 0;

        Shader(const std::string &vert_path, const std::string &frag_path);
        static Shader FromSource(const std::string &vert_src, const std::string &frag_src);
        ~Shader();

        void bind() const;

        void setBool(const std::string &name, bool value) const;
        void setInt(const std::string &name, int value) const;
        void setFloat(const std::string &name, float value) const;
        void setVec2(const std::string &name, const glm::vec2 &value) const;
        void setVec3(const std::string &name, const glm::vec3 &value) const;
        void setVec4(const std::string &name, const glm::vec4 &value) const;
        void setMat4(const std::string &name, const glm::mat4 &mat) const;

    private:
        Shader();

        void compile(const char *vert_src, const char *frag_src);
        void checkCompileErrors(unsigned int shader, const std::string &type);
    };
}