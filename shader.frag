#version 330 core
in vec3 ourColor;
in vec2 TexCoord;

out vec4 color;

uniform sampler2D ourTexture1;
uniform sampler2D ourTexture2;

uniform float angle;

void main()
{
    float cos_factor = cos(angle);
    float sin_factor = sin(angle);
    vec2 coord = TexCoord;

    coord -= vec2(0.5f, 0.5f);
    coord = mat2(cos_factor, sin_factor, -sin_factor, cos_factor)*coord;
    coord += vec2(0.5f, 0.5f);

    color = mix(texture2D(ourTexture2, coord), texture2D(ourTexture1, TexCoord*2 - vec2(0.5,0.5)), 0.6) * vec4(ourColor, 1.0f);
}
