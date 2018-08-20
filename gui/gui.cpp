#include "gui.h"

#include "../imgui/imgui.h"


Gui::Gui()
{
}

void Gui::init(GLFWwindow *window, const char *glsl_version)
{
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO(); (void) io;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    io.Fonts->AddFontFromFileTTF("./imgui/misc/fonts/Cousine-Regular.ttf", 15.0f);

    ImGui::StyleColorsDark();
}

Gui::~Gui()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();

    ImGui::DestroyContext();
}

void Gui::render()
{
    // GUI
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    if (show_demo_window)
    {
        ImGui::ShowDemoWindow();
    }
    {
        ImGui::Begin("OpenGL test");

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

        ImGui::Checkbox("Flashlight", &flashlight);
        ImGui::Checkbox("Blinn", &blinn);
        ImGui::Checkbox("Normal", &normal);
        ImGui::SameLine();
        ImGui::Checkbox("Paralax", &paralax);
        ImGui::Checkbox("Rotate", &rotate);

        const char* items[] = { "Standart", "Normales", "Reflect", "Refract", "Fresnel", "Diffuse", "Specular", "Ambient", "Bump", "Displ" };
        const int item_modes[] = { display_mode::standart, display_mode::normales, display_mode::reflect, display_mode::refract, display_mode::fresnel,
                                   display_mode::texture_diffuse, display_mode::texture_specular,
                                   display_mode::texture_ambient, display_mode::texture_bump, display_mode::texture_displ };
        static const char* item_current = items[0];
        if (ImGui::BeginCombo("Display mode", item_current))
        {
            for (int i = 0; i < IM_ARRAYSIZE(items); i++)
            {
                bool is_selected = (item_current == items[i]);
                if (ImGui::Selectable(items[i], is_selected))
                {
                    item_current = items[i];
                    m_DisplayMode = item_modes[i];
                }
                if (is_selected)
                {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }


        ImGui::SliderFloat("Refract ratio", &refractRatio, 0.0f, 1.0f);
        ImGui::SliderFloat("Height scale", &heightScale, 0.0f, 0.7f);

        ImGui::End();
    }
    if (show_another_window)
    {
        ImGui::Begin("Another Window", &show_another_window);
        ImGui::Text("Hello from another window!");
        if (ImGui::Button("Close Me"))
            show_another_window = false;
        ImGui::End();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
