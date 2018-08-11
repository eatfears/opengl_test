#version 330 core
out vec4 color;

in vec3 FragPos;
in vec2 TexCoords;
in mat3 TBN;

uniform vec3 viewPos;
uniform mat4 viewInv;

uniform bool flashlight;
uniform bool blinn;
uniform bool normal_mapping;
uniform int display_mode;
uniform float refractRatio;

struct Material
{
    sampler2D texture_diffuse1;
    sampler2D texture_specular1;
    sampler2D texture_ambient1;
    sampler2D texture_bump1;
    sampler2D texture_displ1;
    float shininess;
    sampler2D texture_diffuse2;
};
uniform Material material;

uniform samplerCube reflectSample;

struct Phong {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct Attenuation {
    float constant;
    float linear;
    float quadratic;
};

struct DirLight {
    vec3 direction;
    Phong phong;
};

uniform DirLight dirLight;

struct PointLight {
    vec3 position;
    Attenuation attenuation;
    Phong phong;
};

#define NR_POINT_LIGHTS 4
uniform PointLight pointLights[NR_POINT_LIGHTS];

struct SpotLight {
    vec3 position;
    vec3 direction;

    float cutOff;
    float outerCutOff;

    Attenuation attenuation;
    Phong phong;
};

uniform SpotLight spotLight;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 viewDir);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 viewDir);

float zNear = 0.1;
float zFar  = 100.0;

float LinearizeDepth(float depth)
{
    // преобразуем обратно в NDC
    float z = depth * 2.0 - 1.0;
    return (2.0 * zNear * zFar) / (zFar + zNear - z * (zFar - zNear));
}

void main()
{
    vec3 norm;
    if (normal_mapping)
    {
        norm = (texture2D(material.texture_bump1, TexCoords).rgb);
        if (length(norm) == 0.0)
        {
            norm = vec3(0.5, 0.5, 1.0);   //fake
        }
        norm = normalize(norm * 2.0 - 1.0);
        norm = normalize(TBN * norm);
    }
    else
    {
        norm = TBN[2];
    }

    vec3 viewDir = normalize(viewPos-FragPos);
    vec3 result;
    vec3 Ref = /*mat3(viewInv) **/ refract(-viewDir, norm, refractRatio);
    vec3 R = /*mat3(viewInv) **/ reflect(-viewDir, norm);

    switch (display_mode)
    {
    case 0:

        result = CalcDirLight(dirLight, norm, viewDir);

        for (int i = 0; i < NR_POINT_LIGHTS; i++)
        {
            result += CalcPointLight(pointLights[i], norm, viewDir);
        }

        if (flashlight)
        {
            result += CalcSpotLight(spotLight, norm, viewDir);
        }

        vec3 reflection_ratio = texture2D(material.texture_ambient1, TexCoords).rgb;
        vec3 reflection = reflection_ratio * texture(reflectSample, R).rgb;
        result += reflection;
//        result = TBN[1];
        break;
    case 1:
        result = norm;
        break;
    case 2:
        result = texture(reflectSample, R).rgb;
        break;
    case 3:
        result = texture(reflectSample, Ref).rgb;
        break;
    case 4:
        result = texture2D(material.texture_diffuse1, TexCoords).rgb;
        break;
    case 5:
        result = texture2D(material.texture_specular1, TexCoords).rgb;
        break;
    case 6:
        result = texture2D(material.texture_ambient1, TexCoords).rgb;
        break;
    case 7:
        result = texture2D(material.texture_bump1, TexCoords).rgb;
        break;
    case 8:
        result = texture2D(material.texture_displ1, TexCoords).rgb;
        break;
    default:
        result = vec3(1.0);
        break;
    }

//    result = texture(reflectSample, R).rgb;
//    result = vec3(LinearizeDepth(gl_FragCoord.z) / zFar);
//    result = texture2D(material.texture_bump1, TexCoords).rgb;
//    result = TBN[0];
//    result = TBN[1];
    color = vec4(result, 1.0);
}


Phong CalcPhong(Phong light, vec3 normal, vec3 viewDir, vec3 lightDir)
{
    // диффузное освещение
    float diff = max(dot(normal, lightDir), 0.0);

    // освещение зеркальных бликов
    float spec = 0.0f;

    if (blinn)
    {
        //blinn
        vec3 halfwayDir = normalize(lightDir + viewDir);
        spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess*3);
    }
    else
    {
        vec3 reflectDir = reflect(-lightDir, normal);
        spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    }

    Phong p;
    p.ambient  = light.ambient  * 1.0f * vec3(texture2D(material.texture_diffuse1, TexCoords));
    p.diffuse  = light.diffuse  * diff * vec3(texture2D(material.texture_diffuse1, TexCoords));
    p.specular = light.specular * spec * vec3(texture2D(material.texture_specular1, TexCoords));
    return p;
}

float CalcAttenuation(Attenuation attenuation, float distance)
{
    // затухание
    float ret = 1.0 / (attenuation.constant + attenuation.linear * distance +
                       attenuation.quadratic * (distance * distance));
    if (true) //gamma
    {
        ret = ret*ret;
    }
    return ret;
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);
    Phong p = CalcPhong(light.phong, normal, viewDir, lightDir);
    return (p.ambient + p.diffuse + p.specular);
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - FragPos);
    Phong p = CalcPhong(light.phong, normal, viewDir, lightDir);

    return (p.ambient + p.diffuse + p.specular) * CalcAttenuation(light.attenuation, length(light.position - FragPos));
}

vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - FragPos);
    Phong p = CalcPhong(light.phong, normal, viewDir, lightDir);

    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon   = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

    p.diffuse *= intensity;
    p.specular *= intensity;

    return (p.ambient + p.diffuse + p.specular) * CalcAttenuation(light.attenuation, length(light.position - FragPos));
}
