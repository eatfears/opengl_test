#ifndef GUI_H
#define GUI_H

#include "imgui/imgui.h"
#include "imgui/examples/imgui_impl_glfw.h"
#include "imgui/examples/imgui_impl_opengl3.h"


class Gui
{
public:
    Gui();
    ~Gui();

    void init(GLFWwindow *window, const char *glsl_version);
    void render();

    bool show_another_window = false;
    bool show_demo_window = false;

    bool flashlight = false;
    bool blinn = true;
    bool normal = true;


    float clear_color[3] = {};
};

#endif // GUI_H
