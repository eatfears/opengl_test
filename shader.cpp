#include "shader.h"

Shader::Shader(const GLchar* vertex_path, const GLchar* fragment_path)
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

void Shader::use()
{
    glUseProgram(this->m_Program);
}
