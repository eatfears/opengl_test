#version 330 core
layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 Normal;

void main()
{
    Normal = mat3(transpose(inverse(view * model))) * aNormal;
    gl_Position = projection * view * model * vec4(aPosition + aNormal*0.1, 1.0f) ;
}
