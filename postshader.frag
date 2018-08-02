#version 330 core
out vec4 color;

in vec2 TexCoords;

uniform sampler2D screenTexture;

const float offset = 1.0 / 300.0;
const float gray = 0.4;
const float strength = 0.2;
const float gamma = 2.2; //2.2

void main()
{
    vec2 offsets[9] = vec2[](
        vec2(-offset,  offset), // top-left
        vec2( 0.0f,    offset), // top-center
        vec2( offset,  offset), // top-right
        vec2(-offset,  0.0f),   // center-left
        vec2( 0.0f,    0.0f),   // center-center
        vec2( offset,  0.0f),   // center-right
        vec2(-offset, -offset), // bottom-left
        vec2( 0.0f,   -offset), // bottom-center
        vec2( offset, -offset)  // bottom-right
    );

//    float kernel[9] = float[](
//        -1, -1, -1,
//        -1,  9, -1,
//        -1, -1, -1
//    );
    float kernel[9] = float[](
        1.0 / 16, 2.0 / 16, 1.0 / 16,
        2.0 / 16, 4.0 / 16, 2.0 / 16,
        1.0 / 16, 2.0 / 16, 1.0 / 16
    );
//    float kernel[9] = float[](
//        -1, -1, -1,
//        -1,  8, -1,
//        -1, -1, -1
//    );
//    float kernel[9] = float[](
//         0, -1,  0,
//        -1,  5, -1,
//         0, -1,  0
//    );



    vec3 sampleTex[9];
    for(int i = 0; i < 9; i++)
    {
        sampleTex[i] = vec3(texture2D(screenTexture, TexCoords.st + offsets[i]));
    }
    vec3 col = vec3(0.0);
    for(int i = 0; i < 9; i++)
        col += sampleTex[i] * kernel[i];

//    vec4 color2 = ;
    vec4 color2 = vec4(col, 1.0)*strength + vec4(sampleTex[4], 1.0)*(1.0 - strength);

    float average = (color2.r + color2.g + color2.b) / 3.0;

    color = vec4(average, average, average, 1.0)*gray + color2*(1.0 - gray);

    // применяем гамма-коррекцию
    color.rgb = pow(color.rgb, vec3(1.0/gamma));
}
