# LabImGui Prototyping framework

LabImGui wraps up creating a window, GL bindings, and a full screen docking
set up with ImGui so that all of the boilerplate involved is out of the way.

It's the minimal amount of visible API to pull it off.

Note that until Dear ImGui supports multiple windows on Linux, the multiple
window support on this project is not implemented.

Tested on

| Platform/SDK | GL |
| ------------ | -- |
| Windows      | :heavy_check_mark: |
| Mac          | :heavy_check_mark: |

```cpp
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
```

