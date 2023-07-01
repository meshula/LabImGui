# LabImGui Prototyping framework

LabImGui wraps up creating a window, GL bindings, and a full screen docking
set up with ImGui so that all of the boilerplate involved is out of the way.

It's the minimal amount of visible API to pull it off.

Note that until Dear ImGui supports multiple windows on Linux, the multiple
window support on this project is not implemented.

Windows | GL                 |
------- | ------------------ |
GLFW    | :white_check_mark: |
Sokol   | :white_check_mark: |

Mac     | GL                 | Metal              | WebGPU/native    |
------- | ------------------ | ------------------ | ---------------- |
GLFW    | :white_check_mark: | :hourglass:        | :jack_o_lantern: |
Sokol   | :white_check_mark: | :white_check_mark: | :jack_o_lantern: |
Cocoa   | :jack_o_lantern:   | :white_check_mark: | :hourglass:      |

iOS     | Metal              | WebGPU/native    |
------- | ------------------ | ---------------- |
Sokol   | :hourglass:        | :jack_o_lantern: |
Cocoa   | :construction:     | :hourglass:      |

Linux   | GL                 | Vulkan           |
------- | ------------------ | ---------------- |
GLFW    | :construction:     | :hourglass:      |


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

