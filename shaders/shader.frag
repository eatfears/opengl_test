#version 330 core
out vec4 color;

in vec3 FragPos;
in vec2 TexCoords;

in mat3 WorldToTangent;
in mat3 TangentToWorld;

in vec3 TangentLightDir1;
in vec3 TangentLightPos2[4];
in vec3 TangentLightDir3;
in vec3 TangentLightPos3;

in vec3 TangentViewPos;
in vec3 TangentFragPos;

uniform vec3 viewPos;
uniform mat4 viewInv;

uniform bool flashlight;
uniform bool blinn;
uniform bool normal_mapping;
uniform bool paralax_mapping;
uniform int display_mode;
uniform float refractRatio;
uniform float heightScale;

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

Phong CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
Phong CalcPointLight(PointLight light, vec3 normal, vec3 viewDir, vec3 tanPos);
Phong CalcSpotLight(SpotLight light, vec3 normal, vec3 viewDir);

float zNear = 0.1;
float zFar  = 100.0;

float LinearizeDepth(float depth)
{
    // преобразуем обратно в NDC
    float z = depth * 2.0 - 1.0;
    return (2.0 * zNear * zFar) / (zFar + zNear - z * (zFar - zNear));
}

vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir)
{
    float height =  texture2D(material.texture_displ1, texCoords).r;
    vec2 p = viewDir.xy / viewDir.z * (height * heightScale);
    return texCoords - p;
}

void main()
{
    vec3 norm;
    if (normal_mapping)
    {
        norm = (texture2D(material.texture_bump1, TexCoords).rgb);
        if (length(norm) == 0.0)
        {
            norm = vec3(0.0, 0.0, 1.0);
        }
        else
        {
            norm = normalize(norm * 2.0 - 1.0);
        }
    }
    else
    {
        norm = vec3(0.0, 0.0, 1.0);
    }

    vec3 tangentViewDir = normalize(TangentViewPos-TangentFragPos);
    vec3 viewDir = normalize(viewPos-FragPos);
    vec3 result, R, Ref;

    switch (display_mode)
    {
    case 0:
        Phong p;
        vec3 diffuse = vec3(0.0);
        vec3 specular = vec3(0.0);

        p = CalcDirLight(dirLight, norm, tangentViewDir);
        diffuse += p.ambient + p.diffuse;
        specular += p.specular;

        for (int i = 0; i < 4; i++)
        {
            p = CalcPointLight(pointLights[i], norm, tangentViewDir, TangentLightPos2[i]);
            diffuse += p.ambient + p.diffuse;
            specular += p.specular;
        }

        if (flashlight)
        {
            p = CalcSpotLight(spotLight, norm, tangentViewDir);
            diffuse += p.ambient + p.diffuse;
            specular += p.specular;
        }

        vec2 texCoords;
        if (paralax_mapping)
        {
            texCoords = ParallaxMapping(TexCoords, tangentViewDir);
            if(texCoords.x > 1.0 || texCoords.y > 1.0 || texCoords.x < 0.0 || texCoords.y < 0.0)
                discard;
        }
        else
        {
            texCoords = TexCoords;
        }

        result = diffuse * texture2D(material.texture_diffuse1, texCoords).rgb +
                specular * texture2D(material.texture_specular1, texCoords).rgb;

        vec3 reflection_ratio = texture2D(material.texture_ambient1, texCoords).rgb;
        if (length(reflection_ratio) > 0)
        {
            R = /*mat3(viewInv) **/ reflect(-viewDir, TangentToWorld*norm);
            vec3 reflection = reflection_ratio * texture(reflectSample, R).rgb;
            result += reflection;
        }
        break;
    case 1:
        result = TangentToWorld*norm;
        break;
    case 2:
        R = /*mat3(viewInv) **/ reflect(-viewDir, TangentToWorld*norm);
        result = texture(reflectSample, R).rgb;
        break;
    case 3:
        Ref = /*mat3(viewInv) **/ refract(-viewDir, TangentToWorld*norm, refractRatio);
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


Phong CalcPhong(Phong light, vec3 normal, vec3 viewDir, vec3 lightDir, float att)
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
    p.ambient  = light.ambient  * 1.0f * att;
    p.diffuse  = light.diffuse  * diff * att;
    p.specular = light.specular * spec * att;
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

Phong CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-TangentLightDir1);
    return CalcPhong(light.phong, normal, viewDir, lightDir, 1.0);
}

Phong CalcPointLight(PointLight light, vec3 normal, vec3 viewDir, vec3 tanPos)
{
    vec3 lightDir = normalize(tanPos - TangentFragPos);
    return CalcPhong(light.phong, normal, viewDir, lightDir, CalcAttenuation(light.attenuation, length(tanPos - TangentFragPos)));
}

Phong CalcSpotLight(SpotLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(TangentLightPos3 - TangentFragPos);
    Phong p = CalcPhong(light.phong, normal, viewDir, lightDir, CalcAttenuation(light.attenuation, length(TangentLightPos3 - TangentFragPos)));

    float theta = dot(lightDir, normalize(-TangentLightDir3));
    float epsilon   = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

    p.diffuse *= intensity;
    p.specular *= intensity;

    return p;
}
