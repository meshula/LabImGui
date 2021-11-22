
//#include <GL/gl3w.h>
//#include "LabImgui/LabImGui-gl-glfw.h"
#include "LabImgui/LabImGui-sokol.h"

#include "imgui.h"

#include <exception>
#include <iostream>

void frame()
{
    lab_WindowState ws;
    lab_imgui_window_state("Hello LabImgui", &ws);
    if (!ws.valid)
        return;

    //------------ draw to the frame buffer
    
//        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);

    //------------ start the Dear ImGui portion

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

int main(int argv, char** argc) try
{
    lab_imgui_init();
    lab_imgui_create_window("Hello LabImgui", 1024, 768, frame);
    lab_imgui_shutdown();
    return 0;
}
catch (std::exception& exc)
{
    std::cerr << exc.what();
    return -1;
}
