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
    GLuint m_Program;
    std::string vertex_path;
    std::string fragment_path;
public:

    Shader(const std::string &vertex_path, const std::string &fragment_path)
        : vertex_path(vertex_path), fragment_path(fragment_path)
    {
        // 1. Получаем исходный код шейдера из filePath
        std::string vertex_code;
        std::string fragment_code;
        std::ifstream v_shader_file;
        std::ifstream f_shader_file;

        // Удостоверимся, что ifstream объекты могут выкидывать исключения
        v_shader_file.exceptions(std::ifstream::badbit);
        f_shader_file.exceptions(std::ifstream::badbit);
        try
        {
            // Открываем файлы
            v_shader_file.open(vertex_path);
            f_shader_file.open(fragment_path);
            std::stringstream v_shader_stream, f_shader_stream;
            // Считываем данные в потоки
            v_shader_stream << v_shader_file.rdbuf();
            f_shader_stream << f_shader_file.rdbuf();
            // Закрываем файлы
            v_shader_file.close();
            f_shader_file.close();
            // Преобразовываем потоки в массив GLchar
            vertex_code = v_shader_stream.str();
            fragment_code = f_shader_stream.str();
        }
        catch(std::ifstream::failure &e)
        {
            std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ: " << e.what() << std::endl;
        }
        const GLchar* v_shader_code = vertex_code.c_str();
        const GLchar* f_shader_code = fragment_code.c_str();

        /***********************/

        GLuint vertex, fragment;
        GLint success;
        GLchar infoLog[512];

        // Вершинный шейдер
        vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &v_shader_code, NULL);
        glCompileShader(vertex);
        // Если есть ошибки - вывести их
        glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(vertex, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
        };

        // Фрагментный шейдер
        fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &f_shader_code, NULL);
        glCompileShader(fragment);
        // Если есть ошибки - вывести их
        glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(vertex, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
        };


        /***********************/

        // Шейдерная программа
        this->m_Program = glCreateProgram();
        glAttachShader(this->m_Program, vertex);
        glAttachShader(this->m_Program, fragment);
        glLinkProgram(this->m_Program);
        //Если есть ошибки - вывести их
        glGetProgramiv(this->m_Program, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(this->m_Program, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
        }

        // Удаляем шейдеры, поскольку они уже в программу и нам больше не нужны.
        glDeleteShader(vertex);
        glDeleteShader(fragment);
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
};

#endif
