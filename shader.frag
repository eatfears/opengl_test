#version 330 core
out vec4 color;

uniform vec4 ourColor; // Входная переменная из вершинного шейдера (то же название и тот же тип)

void main()
{
    color = ourColor;
}
