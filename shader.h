#ifndef SHADER_H
#define SHADER_H

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include <GL/glew.h>;

class Shader
{
public:
    GLuint m_Program;

    Shader(const GLchar* vertex_path, const GLchar* fragment_path);

    void use();
};

#endif
