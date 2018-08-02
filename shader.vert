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

uniform float time;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

mat4 rotationMatrix(vec3 axis, float angle)
{
    axis = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;

    return mat4(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,  0.0,
                oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,  0.0,
                oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c,           0.0,
                0.0,                                0.0,                                0.0,                                1.0);
}

void main()
{
    mat4 model_loc = model;
    if (instance)
    {
        model_loc =  aInstanceMatrix;

        vec3 axis = vec3(sin(gl_InstanceID), sin(gl_InstanceID), cos(gl_InstanceID));
        model_loc = model_loc * rotationMatrix(normalize(axis), time);
    }

    FragPos = vec3(view * model_loc * vec4(aPosition, 1.0f));
    Normal = mat3(transpose(inverse(view * model_loc))) * aNormal;
    TexCoords = aTexCoords;

    gl_Position = projection * view * model_loc * vec4(aPosition, 1.0f);
}
