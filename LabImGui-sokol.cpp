
#include "LabImgui/LabImGui.h"
#include "sokol_app.h"
#include "sokol_gfx.h"
#include "sokol_time.h"
#include "sokol_glue.h"
#include "imgui.h"
#include "sokol_imgui.h"
#include "imgui_internal.h"

static sg_pass_action pass_action;
static uint64_t last_time = 0;
static bool show_test_window = true;
static bool show_another_window = false;

extern "C"
bool lab_imgui_init()
{
    return true;
}

extern "C"
bool lab_imgui_update(float timeout_seconds, bool auto_close)
{
   return true;
}


extern "C"
void lab_imgui_render(const lab_WindowState*)
{
}

extern "C"
void lab_imgui_present(const lab_WindowState*)
{
}

extern "C"
void lab_imgui_window_state(const char* window_name, lab_WindowState * s)
{
    if (!s)
        return;

    s->width = sapp_width();
    s->height = sapp_height();
    s->valid = s->width > 0 && s->height > 0;
}


extern "C"
void lab_imgui_shutdown()
{
}

extern "C"
void lab_imgui_new_docking_frame(const lab_WindowState* ws)
{
    if (!ws->valid)
        return;

    // simgui will have called ImGui::NewFrame(); already
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

static void init(void) {
    // setup sokol-gfx, sokol-time and sokol-imgui
    sg_desc desc = { };
    desc.context = sapp_sgcontext();
    sg_setup(&desc);
    stm_setup();

    // use sokol-imgui with all default-options (we're not doing
    // multi-sampled rendering or using non-default pixel formats)
    simgui_desc_t simgui_desc = { };
    simgui_setup(&simgui_desc);
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    // initial clear color
    pass_action.colors[0].action = SG_ACTION_CLEAR;
    pass_action.colors[0].value = { 0.0f, 0.5f, 0.7f, 1.0f };
}

static void (*frame_cb)(void) = nullptr;

static void frame(void) {
    const int width = sapp_width();
    const int height = sapp_height();
    const double delta_time = stm_sec(stm_laptime(&last_time));
    simgui_new_frame(width, height, delta_time);

    if (frame_cb)
        frame_cb();
    else {
        // 1. Show a simple window
        // Tip: if we don't call ImGui::Begin()/ImGui::End() the widgets appears in a window automatically called "Debug"
        static float f = 0.0f;
        ImGui::Text("Hello, world!");
        ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
        ImGui::ColorEdit3("clear color", &pass_action.colors[0].value.r);
        if (ImGui::Button("Test Window"))
            show_test_window ^= 1;
        if (ImGui::Button("Another Window"))
            show_another_window ^= 1;
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

        // 2. Show another simple window, this time using an explicit Begin/End pair
        if (show_another_window)
        {
            ImGui::SetNextWindowSize(ImVec2(200, 100), ImGuiCond_FirstUseEver);
            ImGui::Begin("Another Window", &show_another_window);
            ImGui::Text("Hello");
            ImGui::End();
        }

        // 3. Show the ImGui test window. Most of the sample code is in ImGui::ShowDemoWindow()
        if (show_test_window)
        {
            ImGui::SetNextWindowPos(ImVec2(460, 20), ImGuiCond_FirstUseEver);
            ImGui::ShowDemoWindow();
        }
    }

    // the sokol_gfx draw pass
    sg_begin_default_pass(&pass_action, width, height);
    simgui_render();
    sg_end_pass();
    sg_commit();
}

static void cleanup(void) {
    simgui_shutdown();
    sg_shutdown();
}

static void input(const sapp_event* event) {
    simgui_handle_event(event);
}

static sapp_desc app_desc = { };

extern "C"
bool lab_imgui_create_window(const char* window_name, int width, int height,
    void (*custom_frame)(void))
{
    frame_cb = custom_frame;
    app_desc.init_cb = init;
    app_desc.frame_cb = frame;
    app_desc.cleanup_cb = cleanup;
    app_desc.event_cb = input;
    app_desc.width = width;
    app_desc.height = height;
    app_desc.gl_force_gles2 = true;
    app_desc.window_title = window_name;
    app_desc.ios_keyboard_resizes_canvas = false;
    app_desc.icon.sokol_default = true;
    sapp_run(&app_desc);
    return true;
}


