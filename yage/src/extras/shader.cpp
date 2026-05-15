#include <yage/extras/shader.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>

#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif

namespace yage
{
    static std::string resolveShaderPath(const std::string &path)
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

    Shader::Shader() : id(0) {}

    Shader::Shader(const std::string &vert_path, const std::string &frag_path)
    {
        std::string vertCode, fragCode;
        std::ifstream vFile, fFile;

        vFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        fFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

        try
        {
            vFile.open(resolveShaderPath(vert_path));
            fFile.open(resolveShaderPath(frag_path));
            std::stringstream vss, fss;
            vss << vFile.rdbuf();
            fss << fFile.rdbuf();
            vertCode = vss.str();
            fragCode = fss.str();
        }
        catch (std::ifstream::failure &)
        {
            std::cout << "shader file not read successfully" << std::endl;
        }

        compile(vertCode.c_str(), fragCode.c_str());
    }

    Shader Shader::FromSource(const std::string &vert_src, const std::string &frag_src)
    {
        Shader s;
        s.compile(vert_src.c_str(), frag_src.c_str());
        return s;
    }

    Shader::~Shader()
    {
        if (id != 0)
            glDeleteProgram(id);
    }

    void Shader::bind() const { glUseProgram(id); }

    void Shader::setBool(const std::string &name, bool value) const { glUniform1i(glGetUniformLocation(id, name.c_str()), (int)value); }
    void Shader::setInt(const std::string &name, int value) const { glUniform1i(glGetUniformLocation(id, name.c_str()), value); }
    void Shader::setFloat(const std::string &name, float value) const { glUniform1f(glGetUniformLocation(id, name.c_str()), value); }
    void Shader::setVec2(const std::string &name, const glm::vec2 &value) const { glUniform2fv(glGetUniformLocation(id, name.c_str()), 1, &value[0]); }
    void Shader::setVec3(const std::string &name, const glm::vec3 &value) const { glUniform3fv(glGetUniformLocation(id, name.c_str()), 1, &value[0]); }
    void Shader::setVec4(const std::string &name, const glm::vec4 &value) const { glUniform4fv(glGetUniformLocation(id, name.c_str()), 1, &value[0]); }
    void Shader::setMat4(const std::string &name, const glm::mat4 &mat) const { glUniformMatrix4fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, &mat[0][0]); }

    void Shader::compile(const char *vert_src, const char *frag_src)
    {
        unsigned int vert = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vert, 1, &vert_src, NULL);
        glCompileShader(vert);
        checkCompileErrors(vert, "vertex");

        unsigned int frag = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(frag, 1, &frag_src, NULL);
        glCompileShader(frag);
        checkCompileErrors(frag, "fragment");

        id = glCreateProgram();
        glAttachShader(id, vert);
        glAttachShader(id, frag);
        glLinkProgram(id);
        checkCompileErrors(id, "program");

        glDeleteShader(vert);
        glDeleteShader(frag);
    }

    void Shader::checkCompileErrors(unsigned int shader, const std::string &type)
    {
        GLint success;
        GLchar infoLog[1024];
        if (type != "program")
        {
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (!success)
            {
                glGetShaderInfoLog(shader, 1024, NULL, infoLog);
                std::cout << type << " shader compilation failed\n"
                          << infoLog << std::endl;
            }
        }
        else
        {
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            if (!success)
            {
                glGetProgramInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "shader program linking failed\n"
                          << infoLog << std::endl;
            }
        }
    }
}