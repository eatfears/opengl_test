#version 330 core
out vec4 color;

in vec2 TexCoords;

uniform sampler2D texture1;

void main()
{
    vec4 texColor = texture2D(texture1, TexCoords);
    if (texColor.a < 0.1f)
    {
        discard;
    }
    color = texColor;
}
