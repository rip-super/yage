#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class Shader
{
public:
    unsigned int id;

    Shader(const char *vertPath, const char *fragPath)
    {
        std::string vertCode, fragCode;
        std::ifstream vShaderFile, fShaderFile;

        vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

        try
        {
            vShaderFile.open(vertPath);
            fShaderFile.open(fragPath);
            std::stringstream vss, fss;
            vss << vShaderFile.rdbuf();
            fss << fShaderFile.rdbuf();
            vertCode = vss.str();
            fragCode = fss.str();
        }
        catch (std::ifstream::failure e)
        {
            std::cout << "shader file not read successfully" << std::endl;
        }

        compile(vertCode.c_str(), fragCode.c_str());
    }

    static Shader FromSource(const char *vertSrc, const char *fragSrc)
    {
        Shader s;
        s.compile(vertSrc, fragSrc);
        return s;
    }

    void bind() const { glUseProgram(id); }

    void setBool(const std::string &name, bool value) const { glUniform1i(glGetUniformLocation(id, name.c_str()), (int)value); }
    void setInt(const std::string &name, int value) const { glUniform1i(glGetUniformLocation(id, name.c_str()), value); }
    void setFloat(const std::string &name, float value) const { glUniform1f(glGetUniformLocation(id, name.c_str()), value); }
    void setVec2(const std::string &name, const glm::vec2 &value) const { glUniform2fv(glGetUniformLocation(id, name.c_str()), 1, &value[0]); }
    void setVec3(const std::string &name, const glm::vec3 &value) const { glUniform3fv(glGetUniformLocation(id, name.c_str()), 1, &value[0]); }
    void setVec4(const std::string &name, const glm::vec4 &value) const { glUniform4fv(glGetUniformLocation(id, name.c_str()), 1, &value[0]); }
    void setMat4(const std::string &name, const glm::mat4 &mat) const { glUniformMatrix4fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, &mat[0][0]); }

private:
    Shader() : id(0) {}

    void compile(const char *vertSrc, const char *fragSrc)
    {
        unsigned int vert = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vert, 1, &vertSrc, NULL);
        glCompileShader(vert);
        checkCompileErrors(vert, "vertex");

        unsigned int frag = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(frag, 1, &fragSrc, NULL);
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

    void checkCompileErrors(GLuint shader, std::string type)
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
};