
//#include <GL/gl3w.h>
#include "LabImgui/LabImGui.h"

#include "imgui.h"

#include <exception>
#include <iostream>

void frame()
{
    lab_WindowState ws;
    lab_imgui_window_state("Hello LabImGui", &ws);
    if (!ws.valid)
        return;

    //------------ draw to the frame buffer

    // custom stuff

    //------------ start the Dear ImGui portion

    lab_imgui_new_docking_frame(&ws);
    lab_imgui_begin_fullscreen_docking(&ws);

    //------------ custom begin


    ImGui::Begin("Hello LabImgui");
    ImGui::Button("hi!");
    ImGui::End();

    //------------ custom end

    lab_imgui_end_fullscreen_docking(&ws);
}

int main(int argv, char** argc) try
{
    lab_imgui_init();
    lab_imgui_create_window("Hello LabImGui", 1024, 768, frame);
    lab_imgui_shutdown();
    return 0;
}
catch (std::exception& exc)
{
    std::cerr << exc.what();
    return -1;
}
