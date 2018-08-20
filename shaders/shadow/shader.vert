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

uniform vec3 viewPos;

uniform vec3 lightDir1;
uniform vec3 lightPos2[4];
uniform vec3 lightDir3;
uniform vec3 lightPos3;

out VS_OUT
{
    vec3 FragPos;
    vec2 TexCoords;
    mat3 WorldToTangent;
    mat3 TangentToWorld;

    vec3 TangentLightDir1;
    vec3 TangentLightPos2[4];
    vec3 TangentLightDir3;
    vec3 TangentLightPos3;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
} vs_out;

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
    vs_out.FragPos = vec3(modelView * vec4(aPosition, 1.0f));
    vs_out.TexCoords = aTexCoords;


    vec3 T = normalize(modelViewInverseTranspose * aTangent);
    vec3 B = normalize(modelViewInverseTranspose * aBitangent);
    vec3 N = normalize(modelViewInverseTranspose * aNormal);
//    if (dot(cross(N, T), B) < 0.0)
//    {
//        TexCoords = vec2(0.0, 0.0);
//        T = T * -1.0;
//    }
    B = cross(N, T)*dot(cross(N, T), B);
    mat3 TBN = mat3(T, B, N);
    mat3 TBN_inv = transpose(TBN);
    vs_out.TangentToWorld = TBN;
    vs_out.WorldToTangent = TBN_inv;

    vs_out.TangentLightDir1 = TBN_inv * lightDir1;
    for (int i = 0; i < 4; i++)
    {
        vs_out.TangentLightPos2[i] = TBN_inv * lightPos2[i];
    }
    vs_out.TangentLightDir3 = TBN_inv * lightDir3;
    vs_out.TangentLightPos3 = TBN_inv * lightPos3;


    vs_out.TangentViewPos  = TBN_inv * viewPos;
    vs_out.TangentFragPos  = TBN_inv * vec3(modelView * vec4(aPosition, 1.0f));



    gl_Position = projection * view * model_loc * vec4(aPosition, 1.0f);
}
