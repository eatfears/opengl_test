#ifndef ARRAYS_H
#define ARRAYS_H

#include <vector>
#include <string>
#include <glm/glm.hpp>


float skyboxVertices[] = {
    // positions
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,
    1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    1.0f, -1.0f, -1.0f,
    1.0f, -1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,
    1.0f,  1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,
    1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
    1.0f,  1.0f, -1.0f,
    1.0f,  1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
    1.0f, -1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
    1.0f, -1.0f,  1.0f
};

// Set up vertex data (and buffer(s)) and attribute pointers
GLfloat vertices[] = {
    // Back face
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f, // Bottom-left
    0.5f,  0.5f, -0.5f, 0.0f,  0.0f, -1.0f,   1.0f, 1.0f, // top-right
    0.5f, -0.5f, -0.5f, 0.0f,  0.0f, -1.0f,   1.0f, 0.0f, // bottom-right
    0.5f,  0.5f, -0.5f, 0.0f,  0.0f, -1.0f,   1.0f, 1.0f, // top-right
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f, // bottom-left
    -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f, // top-left
    // Front face
    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f, // bottom-left
    0.5f, -0.5f,  0.5f, 0.0f,  0.0f,  1.0f,   1.0f, 0.0f, // bottom-right
    0.5f,  0.5f,  0.5f, 0.0f,  0.0f,  1.0f,   1.0f, 1.0f, // top-right
    0.5f,  0.5f,  0.5f, 0.0f,  0.0f,  1.0f,   1.0f, 1.0f, // top-right
    -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 1.0f, // top-left
    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f, // bottom-left
    // Left face
    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f, // top-right
    -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f, // top-left
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f, // bottom-left
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f, // bottom-left
    -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f, // bottom-right
    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f, // top-right
    // Right face
    0.5f,  0.5f,  0.5f, 1.0f,  0.0f,  0.0f,   1.0f, 0.0f, // top-left
    0.5f, -0.5f, -0.5f, 1.0f,  0.0f,  0.0f,   0.0f, 1.0f, // bottom-right
    0.5f,  0.5f, -0.5f, 1.0f,  0.0f,  0.0f,   1.0f, 1.0f, // top-right
    0.5f, -0.5f, -0.5f, 1.0f,  0.0f,  0.0f,   0.0f, 1.0f, // bottom-right
    0.5f,  0.5f,  0.5f, 1.0f,  0.0f,  0.0f,   1.0f, 0.0f, // top-left
    0.5f, -0.5f,  0.5f, 1.0f,  0.0f,  0.0f,   0.0f, 0.0f, // bottom-left
    // Bottom face
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f, // top-right
    0.5f, -0.5f, -0.5f, 0.0f, -1.0f,  0.0f,   1.0f, 1.0f, // top-left
    0.5f, -0.5f,  0.5f, 0.0f, -1.0f,  0.0f,   1.0f, 0.0f, // bottom-left
    0.5f, -0.5f,  0.5f, 0.0f, -1.0f,  0.0f,   1.0f, 0.0f, // bottom-left
    -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f, // bottom-right
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f, // top-right
    // Top face
    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f, // top-left
    0.5f,  0.5f,  0.5f, 0.0f,  1.0f,  0.0f,   1.0f, 0.0f, // bottom-right
    0.5f,  0.5f, -0.5f, 0.0f,  1.0f,  0.0f,   1.0f, 1.0f, // top-right
    0.5f,  0.5f,  0.5f, 0.0f,  1.0f,  0.0f,   1.0f, 0.0f, // bottom-right
    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f, // top-left
    -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f, // bottom-left
};

GLfloat quad_vertices[] = {
    -1.0f,  1.0f,  0.0f, 1.0f,
    -1.0f, -1.0f,  0.0f, 0.0f,
    1.0f, -1.0f,  1.0f, 0.0f,

    1.0f, -1.0f,  1.0f, 0.0f,
    1.0f,  1.0f,  1.0f, 1.0f,
    -1.0f,  1.0f,  0.0f, 1.0f,
};

glm::vec3 cubePositions[] = {
    glm::vec3( 0.0f,  0.0f,  0.0f),
    glm::vec3( 2.0f,  5.0f, -15.0f),
    glm::vec3(-1.5f, -2.2f, -2.5f),
    glm::vec3(-3.8f, -2.0f, -12.3f),
    glm::vec3( 2.4f, -0.4f, -3.5f),
    glm::vec3(-1.7f,  3.0f, -7.5f),
    glm::vec3( 1.3f, -2.0f, -2.5f),
    glm::vec3( 1.5f,  2.0f, -2.5f),
    glm::vec3( 1.5f,  0.2f, -1.5f),
    glm::vec3(-1.3f,  1.0f, -1.5f)
};

glm::vec3 pointLightPositions[] = {
    glm::vec3( 0.7f,  0.2f,  2.0f),
    glm::vec3( 2.3f, -3.3f, -4.0f),
    glm::vec3(-4.0f,  2.0f, -12.0f),
    glm::vec3( 0.0f,  0.0f, -3.0f)
};


std::vector<glm::vec3> vegetation = {
    glm::vec3(-1.5f,  0.0f, -0.48f),
    glm::vec3( 1.5f,  0.0f,  0.51f),
    glm::vec3( 0.0f,  0.0f,  0.7f),
    glm::vec3(-0.3f,  0.0f, -2.3f),
    glm::vec3( 0.5f,  0.0f, -0.6f),
};

std::vector<glm::vec3> windows = {
    glm::vec3(-1.5f,  -1.0f, -0.48f),
    glm::vec3( 1.5f,  -1.0f,  0.51f),
    glm::vec3( 0.0f,  -1.0f,  0.7f),
    glm::vec3(-0.3f,  -1.0f, -2.3f),
    glm::vec3( 0.5f,  -1.0f, -0.6f),
};

std::vector<std::string> cubemapFaces = {
    "resources/textures/skybox/lake/right.jpg",
    "resources/textures/skybox/lake/left.jpg",
    "resources/textures/skybox/lake/top.jpg",
    "resources/textures/skybox/lake/bottom.jpg",
    "resources/textures/skybox/lake/front.jpg",
    "resources/textures/skybox/lake/back.jpg"
};

#endif // ARRAYS_H
