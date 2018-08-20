#ifndef SHADER_H
#define SHADER_H

#include <string>
#include <map>
#include <fstream>
#include <sstream>
#include <iostream>

#include <GL/glew.h>

class Shader
{
    std::string vertex_path;
    std::string fragment_path;
    std::string geometry_path;

    GLint success;
    GLchar infoLog[512];

public:
    GLuint m_Program;

    Shader(const std::string &vertex_path, const std::string &fragment_path)
        : vertex_path(vertex_path), fragment_path(fragment_path)
    {
        // Шейдерная программа
        this->m_Program = glCreateProgram();

        createShader(vertex_path, GL_VERTEX_SHADER);
        createShader(fragment_path, GL_FRAGMENT_SHADER);
        linkProgram();
    }

    void setGeometryShader(const std::string &path)
    {
        geometry_path = path;
        createShader(geometry_path, GL_GEOMETRY_SHADER);
        linkProgram();
    }

    ~Shader()
    {
        glDeleteProgram(m_Program);
    }

    void use()
    {
        glUseProgram(this->m_Program);
    }

    void setBool(const std::string &name, bool value) const
    {
        glUniform1i(getUniformLocation(name), (int)value);
    }
    // ------------------------------------------------------------------------
    void setInt(const std::string &name, int value) const
    {
        glUniform1i(getUniformLocation(name), value);
    }
    // ------------------------------------------------------------------------
    void setFloat(const std::string &name, float value) const
    {
        glUniform1f(getUniformLocation(name), value);
    }
    // ------------------------------------------------------------------------
    void setVec2(const std::string &name, const glm::vec2 &value) const
    {
        glUniform2fv(getUniformLocation(name), 1, &value[0]);
    }
    void setVec2(const std::string &name, float x, float y) const
    {
        glUniform2f(getUniformLocation(name), x, y);
    }
    // ------------------------------------------------------------------------
    void setVec3(const std::string &name, const glm::vec3 &value) const
    {
        glUniform3fv(getUniformLocation(name), 1, &value[0]);
    }
    void setVec3(const std::string &name, float x, float y, float z) const
    {
        glUniform3f(getUniformLocation(name), x, y, z);
    }
    // ------------------------------------------------------------------------
    void setVec4(const std::string &name, const glm::vec4 &value) const
    {
        glUniform4fv(getUniformLocation(name), 1, &value[0]);
    }
    void setVec4(const std::string &name, float x, float y, float z, float w)
    {
        glUniform4f(getUniformLocation(name), x, y, z, w);
    }
    // ------------------------------------------------------------------------
    void setMat2(const std::string &name, const glm::mat2 &mat) const
    {
        glUniformMatrix2fv(getUniformLocation(name), 1, GL_FALSE, &mat[0][0]);
    }
    // ------------------------------------------------------------------------
    void setMat3(const std::string &name, const glm::mat3 &mat) const
    {
        glUniformMatrix3fv(getUniformLocation(name), 1, GL_FALSE, &mat[0][0]);
    }
    // ------------------------------------------------------------------------
    void setMat4(const std::string &name, const glm::mat4 &mat) const
    {
        glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, &mat[0][0]);
    }
private:
    mutable std::map<std::string, GLint> m_LocationMap;

    GLint getUniformLocation(const std::string &name) const
    {
        GLint location;
        auto it = m_LocationMap.find(name);
        if (it != m_LocationMap.end())
        {
            location = it->second;
        }
        else
        {
            location = glGetUniformLocation(m_Program, name.c_str());
            if (location == -1)
            {
                std::cout << "uniform " << name << " not found (" << vertex_path  << ", " << fragment_path << ")" << std::endl;
            }
            m_LocationMap[name] = location;
        }

        return location;
    }

    std::string readFile(const std::string &path)
    {
        std::string ret;
        std::ifstream file_stream;
        std::stringstream shader_stream;

        file_stream.exceptions(std::ifstream::badbit);
        file_stream.exceptions(std::ifstream::failbit);

        try
        {
            file_stream.open(path);
            shader_stream << file_stream.rdbuf();
            file_stream.close();

            ret = shader_stream.str();
        }
        catch(std::ifstream::failure &e)
        {
            std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ: " << e.what() << "\n" << path << std::endl;
            exit(1);
        }
        return ret;
    }

    GLuint createShader(const std::string &shader_path, GLuint shader_type)
    {
        std::string shader_str = readFile(shader_path);
        const GLchar* p_shader_code = shader_str.c_str();

        /***********************/

        GLuint shader_id;

        // Вершинный шейдер
        shader_id = glCreateShader(shader_type);
        glShaderSource(shader_id, 1, &p_shader_code, NULL);
        glCompileShader(shader_id);
        // Если есть ошибки - вывести их
        glGetShaderiv(shader_id, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(shader_id, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::COMPILATION_FAILED\n" << shader_path << "\n" << infoLog << std::endl;
            exit(1);
        };


        glAttachShader(this->m_Program, shader_id);
        // Удаляем шейдеры, поскольку они уже в программу и нам больше не нужны.
        glDeleteShader(shader_id);

        return shader_id;
    }

    void linkProgram()
    {
        glLinkProgram(this->m_Program);

        //Если есть ошибки - вывести их
        glGetProgramiv(this->m_Program, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(this->m_Program, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
            exit(1);
        }
    }
};

#endif
