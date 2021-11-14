
#include <GL/gl3w.h>
#include "LabImgui/LabImGui-gl-glfw.h"

#include "imgui.h"


#include <exception>
#include <iostream>

int main(char** argc, int argv) try
{
    lab_imgui_init();
    lab_imgui_create_window("Hello LabImgui", 1024, 768);

    while (lab_imgui_update(1.f/60.f, true))
    {
        lab_WindowState ws;
        lab_imgui_window_state("Hello LabImgui", &ws);
        if (!ws.valid)
            break;

        lab_imgui_new_docking_frame(&ws);
        lab_imgui_begin_fullscreen_docking(&ws);

        //------------ custom begin


        ImGui::Begin("Hello LabImgui");
        ImGui::Button("hi!");
        ImGui::End();

        //------------ custom end

        lab_imgui_end_fullscreen_docking(&ws);

        // render the user interface
        lab_imgui_render(&ws);

        // present swaps the frame buffer to be visible
        // so all frame rendering must be complete before the present call
        lab_imgui_present(&ws);
    }

    lab_imgui_shutdown();
    return 0;
}
catch (std::exception& exc)
{
    std::cerr << exc.what();
    return -1;
}
