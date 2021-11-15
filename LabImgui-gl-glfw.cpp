
#include <GL/gl3w.h>

#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_opengl3_loader.h"

#include "LabImgui/LabImgui-gl-glfw.h"
#include <GLFW/glfw3.h>

#include <map>
#include <string>
#include <iostream>

using std::map;
using std::string;
static map<string, GLFWwindow*> _windows;
static GLFWwindow* _rootWindow = nullptr;
float highDPIscaleFactor = 1.0;

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error %d: %s\n", error, description);
}

extern "C"
bool lab_imgui_init()
{
    if (_rootWindow)
        return true;

    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) {
        fprintf(stderr, "Could not initialize glfw");
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
#if __APPLE__
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    if (!_rootWindow) {
        GLFWwindow* window = glfwCreateWindow(16, 16, "Root graphics context", NULL, NULL);
        glfwMakeContextCurrent(window);
        glfwSwapInterval(1); // must be set when a window's context is current
        gl3wInit();

        // get version info
        const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
        const GLubyte* version = glGetString(GL_VERSION); // version as a string
        const GLubyte* glsl_version = glGetString(GL_SHADING_LANGUAGE_VERSION);
        std::cout << "OpenGL Renderer: " << renderer << std::endl;
        std::cout << "OpenGL Version: " << version << std::endl;
        std::cout << "GLSL Version: " << glsl_version << std::endl;
        _rootWindow = window;
    }

    // leave the rootWindow bound.
    glfwMakeContextCurrent(_rootWindow);
    return _rootWindow != nullptr;
}

extern "C"
bool lab_imgui_create_window(const char* window_name, int width, int height)
{
    glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);

#ifdef _WIN32
    // if it's a HighDPI monitor, try to scale everything
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    float xscale, yscale;
    glfwGetMonitorContentScale(monitor, &xscale, &yscale);
    if (xscale > 1 || yscale > 1)
    {
        highDPIscaleFactor = xscale;
        glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);
    }
#elif __APPLE__
    // to prevent 1200x800 from becoming 2400x1600
    glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_FALSE);
#endif

    GLFWwindow* window = glfwCreateWindow(width, height, window_name, NULL, _rootWindow);
    glfwMakeContextCurrent(window);

    lab_imgui_init_window(window_name, window);
    glfwMakeContextCurrent(_rootWindow);    // leave the root window bound
    return true;
}

extern "C"
bool lab_imgui_update(float timeout_seconds, bool auto_close)
{
    glfwWaitEventsTimeout(timeout_seconds);
    int window_count = 0;
    int close_count = 0;

    for (auto w = _windows.begin(); w != _windows.end(); ++w) {
        ++window_count;
        if (glfwWindowShouldClose(w->second)) {
            ++close_count;
            if (auto_close) {
                glfwDestroyWindow(w->second);
                w = _windows.erase(w);
                if (w == _windows.end())
                    break;
            }
        }
    }

    if (window_count == close_count)
        return false;

    return true;
}


extern "C"
void lab_imgui_render(const lab_WindowState*)
{
    /// @TODO support multiple windows, via window state, not via loop over all windows

    glfwPollEvents();

    // Rendering
    ImGui::Render();

    // note: ImGui multi-window is still not ready for primetime, this loop
    // will have to change when ImGui multi-window really exists.
    // right now, this is only valid because only one window will ever be created.
    for (auto w = _windows.begin(); w != _windows.end(); ++w) {
        int display_w, display_h;
        glfwGetFramebufferSize(w->second, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }
}

extern "C"
void lab_imgui_present(const lab_WindowState*)
{
    /// @TODO support multiple windows, via window state, not via loop over all windows

    for (auto w = _windows.begin(); w != _windows.end(); ++w) {
        glfwSwapBuffers(w->second);
    }
}

extern "C"
void lab_imgui_window_state(const char* window_name, lab_WindowState * s)
{
    if (!s)
        return;

    s->width = 0;
    s->height = 0;
    s->valid = false;

    auto i = _windows.find(window_name);
    if (i == _windows.end())
        return;

    glfwGetWindowSize(i->second, &s->width, &s->height);
    s->valid = (s->width > 0) && (s->height > 0);

    if (s->valid)
        glfwMakeContextCurrent(i->second);
    else
        glfwMakeContextCurrent(_rootWindow);    // leave the root window bound
}


extern "C"
void lab_imgui_init_window(const char* window_name, GLFWwindow* window)
{
    _windows[std::string(window_name)] = window;

    // Setup Dear ImGui binding
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 430");

    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    // Setup style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Read 'misc/fonts/README.txt' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != NULL);

    /// @TODO - WindowState should be in the map, and it should include the dpi scale factor
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(highDPIscaleFactor);
}

extern "C"
void lab_imgui_shutdown()
{
    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    for (auto i : _windows)
        glfwDestroyWindow(i.second);
    glfwTerminate();
}

extern "C"
void lab_imgui_new_docking_frame(const lab_WindowState* ws)
{
    if (!ws->valid /* || !ws->bound */)
        return;

    // Start the ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // docking template from https://gist.github.com/PossiblyAShrub/0aea9511b84c34e191eaa90dd7225969

    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;

    // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
    // because it would be confusing to have two docking targets within each others.
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;


    // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background and handle the pass-thru hole, so we ask Begin() to not render a background.
    if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
        window_flags |= ImGuiWindowFlags_NoBackground;

    // Important: note that we proceed even if Begin() returns false (aka window is collapsed).
    // This is because we want to keep our DockSpace() active. If a DockSpace() is inactive, 
    // all active windows docked into it will lose their parent and become undocked.
    // We cannot preserve the docking relationship between an active window and an inactive docking, otherwise 
    // any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("DockSpace", nullptr, window_flags);
    ImGui::PopStyleVar();
    ImGui::PopStyleVar(2);

    // DockSpace
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
    {
        ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);

        static auto first_time = true;
        if (first_time)
        {
            first_time = false;

            ImGui::DockBuilderRemoveNode(dockspace_id); // clear any previous layout
            ImGui::DockBuilderAddNode(dockspace_id, dockspace_flags | ImGuiDockNodeFlags_DockSpace);
            ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->Size);

            // split the dockspace into 2 nodes -- DockBuilderSplitNode takes in the following args in the following order
            //   window ID to split, direction, fraction (between 0 and 1), the final two setting let's us choose which id we want (which ever one we DON'T set as NULL, will be returned by the function)
            //                                                              out_id_at_dir is the id of the node in the direction we specified earlier, out_id_at_opposite_dir is in the opposite direction
            auto dock_id_left = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Left, 0.2f, nullptr, &dockspace_id);
            auto dock_id_down = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Down, 0.25f, nullptr, &dockspace_id);

            // we now dock our windows into the docking node we made above
            ImGui::DockBuilderDockWindow("Down", dock_id_down);
            ImGui::DockBuilderDockWindow("Left", dock_id_left);
            ImGui::DockBuilderFinish(dockspace_id);
        }
    }

    ImGui::End();
}


extern "C"
lab_FullScreenMouseState lab_imgui_begin_fullscreen_docking(const lab_WindowState* ws)
{
    if (!ws->valid /* || !ws->bound */)
        return { false, false, false };

    float w = (float)ws->width;
    float h = (float)ws->height;

    ImGui::SetNextWindowPos({ 0, 0 });
    ImGui::SetNextWindowSize({ w, h });
    static bool begin_flag = false;
    ImGui::Begin("###FULLSCREEN", &begin_flag,
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBackground |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoBringToFrontOnFocus);

    ImGui::SetCursorScreenPos(ImVec2{ 0, 0 });
    ImGui::InvisibleButton("####FULL_SCREEN", { w, h });

    lab_FullScreenMouseState r = {
        ImGui::IsItemClicked(),
        ImGui::IsItemActive(),
        ImGui::IsItemHovered() };

    ImGui::DockSpace(123456);

    return r;
}

extern "C"
void lab_imgui_end_fullscreen_docking(const lab_WindowState* ws)
{
    /// @TODO support multiple windows
    ImGui::End(); // full screen window
}
