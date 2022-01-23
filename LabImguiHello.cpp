
//#include <GL/gl3w.h>
#include "LabImgui/LabImGui.h"

#include "imgui.h"
#include "implot.h"

#include <exception>
#include <iostream>

void imgui_frame()
{
    lab_WindowState ws;
    lab_imgui_window_state("Hello LabImGui", &ws);
    if (!ws.valid)
        return;

    //------------ start the Dear ImGui portion

    lab_imgui_new_docking_frame(&ws);
    lab_imgui_begin_fullscreen_docking(&ws);

    //------------ custom begin

    ImGui::Begin("Hello LabImgui");
    ImGui::Button("hi!");
    ImGui::End();

    ImGui::Begin("Another Window");
    ImGui::Button("hi!###1");
    ImGui::End();

    static bool demo_window = true;
    ImPlot::ShowDemoWindow(&demo_window);

    //------------ custom end

    lab_imgui_end_fullscreen_docking(&ws);
}

int main(int argv, char** argc) try
{
    lab_imgui_init();
    lab_imgui_create_window("Hello LabImGui", 1024, 768, imgui_frame);
    lab_imgui_shutdown();
    return 0;
}
catch (std::exception& exc)
{
    std::cerr << exc.what();
    return -1;
}
