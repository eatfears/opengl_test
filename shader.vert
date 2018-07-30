#version 330 core
layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 FragPos;
out vec3 Normal;
out vec3 LightPos;

out vec2 TexCoords;

uniform vec3 lightPos;

void main()
{
    gl_Position = projection * view * model * vec4(aPosition, 1.0f);
    FragPos = vec3(view * model * vec4(aPosition, 1.0f));
    Normal = mat3(transpose(inverse(view * model))) * aNormal;
    LightPos = vec3(view * vec4(lightPos, 1.0));

    TexCoords = aTexCoords;
}
