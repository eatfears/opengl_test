#version 330 core
layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;
layout (location = 5) in mat4 aInstanceMatrix;

layout (std140) uniform Matrices
{
    mat4 projection;
    mat4 view;
};

uniform mat4 model;

uniform bool instance;
uniform float time;
uniform mat4 planet_model;

out vec3 FragPos;
out vec2 TexCoords;
out mat3 TBN;

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

        vec3 direction = vec3(0.0, 0.0, 0.0);
        vec3 transformed_direction = vec3(aInstanceMatrix * vec4(direction, 1.0));
        float f = 5.0/length(transformed_direction);
        model_loc = rotationMatrix(vec3(0.0, 1.0, 0.0), -f*f*time/2) * model_loc;

        model_loc = planet_model * model_loc;
    }

    mat4 modelView = /*view **/ model_loc;
    mat3 modelViewInverseTranspose = mat3(transpose(inverse(modelView)));
    FragPos = vec3(modelView * vec4(aPosition, 1.0f));
    TexCoords = aTexCoords;


    vec3 T = normalize(modelViewInverseTranspose * aTangent);
    vec3 B = normalize(modelViewInverseTranspose * aBitangent);
    vec3 N = normalize(modelViewInverseTranspose * aNormal);
//    if (dot(cross(N, T), B) < 0.0)
//    {
//        TexCoords = vec2(0.0, 0.0);
//        T = T * -1.0;
//    }
    TBN = mat3(T, B, N);

    gl_Position = projection * view * model_loc * vec4(aPosition, 1.0f);
}
