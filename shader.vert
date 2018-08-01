#version 330 core
layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in mat4 aInstanceMatrix;

layout (std140) uniform Matrices
{
    mat4 projection;
    mat4 view;
};

uniform mat4 model;

uniform bool instance;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;


void main()
{
    mat4 model_loc = model;
    if (instance)
    {
        model_loc =  aInstanceMatrix;
    }

    FragPos = vec3(view * model_loc * vec4(aPosition, 1.0f));
    Normal = mat3(transpose(inverse(view * model_loc))) * aNormal;
    TexCoords = aTexCoords;

    gl_Position = projection * view * model_loc * vec4(aPosition, 1.0f);
}
