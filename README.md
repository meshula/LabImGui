# LabImGui Prototyping framework

LabImGui wraps up creating a window, GL bindings, and a full screen docking
set up with ImGui so that all of the boilerplate involved is out of the way.

It's the minimal amount of visible API to pull it off.

Note that until Dear ImGui supports multiple windows on Linux, the multiple
window support on this project is not implemented.

Tested on

| Platform/SDK | GLFW                  | Sokol                 | Metal              |
| ------------ | --------------------- | --------------------- | ------------------ |
| Windows      | :heavy_check_mark: GL | :heavy_check_mark: GL | :jack_o_lantern:   |
| Mac          | :heavy_check_mark: GL | :construction: Metal  | :heavy_check_mark: |
| iOS          | :jack_o_lantern:      | :construction: Metal  | :construction:     |

```cpp
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

    // custom graphics stuff

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

```

