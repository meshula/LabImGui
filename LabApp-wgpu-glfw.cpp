

#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_wgpu.h"

#include "LabImgui/LabImGui.h"
#include <GLFW/glfw3.h>
#include <dawn/dawn_proc.h>
#include <dawn/webgpu_cpp.h>
#include <dawn_native/DawnNative.h>
#include <webgpu/webgpu.h>

#if defined(_WIN32)
#    define GLFW_EXPOSE_NATIVE_WIN32
#elif defined(DAWN_USE_X11)
#    define GLFW_EXPOSE_NATIVE_X11
#endif
#include <GLFW/glfw3native.h>

#ifdef max
#undef max
#undef min
#endif

#include <map>
#include <memory>
#include <string>
#include <iostream>
#include <unordered_map>

using std::map;
using std::string;
float highDPIscaleFactor = 1.0;
static uint64_t last_time = 0;
static bool show_test_window = true;
static bool show_another_window = false;
static wgpu::BackendType _backendType = wgpu::BackendType::Null;

static dawn_native::Adapter chosenAdapter;
static std::unique_ptr<dawn_native::Instance> instance;
static wgpu::Device device;
static wgpu::Queue queue;


bool IsSameDescriptor(const wgpu::SwapChainDescriptor& a, const wgpu::SwapChainDescriptor& b) {
    return a.usage == b.usage && a.format == b.format && a.width == b.width &&
        a.height == b.height && a.presentMode == b.presentMode;
}

struct WindowData {
    std::string name;
    GLFWwindow* window = nullptr;
    uint64_t serial = 0;

    float clearCycle = 1.0f;
    bool latched = false;
    bool renderTriangle = true;
    uint32_t divisor = 1;

    wgpu::Surface surface = nullptr;
    wgpu::SwapChain swapchain = nullptr;

    wgpu::SwapChainDescriptor currentDesc;
    wgpu::SwapChainDescriptor targetDesc;
};

static std::unordered_map<GLFWwindow*, std::unique_ptr<WindowData>> _windows;
static uint64_t windowSerial = 0;

void SyncTargetSwapChainDescFromWindow(WindowData* data) {
    int width;
    int height;
    glfwGetFramebufferSize(data->window, &width, &height);

    data->targetDesc.width = std::max(1u, width / data->divisor);
    data->targetDesc.height = std::max(1u, height / data->divisor);
}

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error %d: %s\n", error, description);
}

extern "C"
bool lab_imgui_init()
{
    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) {
        fprintf(stderr, "Could not initialize glfw");
        return false;
    }

    // Choose an adapter.
    // TODO: allow switching the window between devices.
    DawnProcTable procs = dawn_native::GetProcs();
    dawnProcSetProcs(&procs);

    instance = std::make_unique<dawn_native::Instance>();
    instance->DiscoverDefaultAdapters();

    std::vector<dawn_native::Adapter> adapters = instance->GetAdapters();
    for (dawn_native::Adapter& adapter : adapters) {
        wgpu::AdapterProperties properties;
        adapter.GetProperties(&properties);
        if (properties.backendType != wgpu::BackendType::Null) {
            chosenAdapter = adapter;
            _backendType = properties.backendType;
            break;
        }
    }
    if (!chosenAdapter) {
        printf("No adapters for wgpu\n");
        return false;
    }

    return true;
}

static void (*frame_cb)(void) = nullptr;
static float clear_color[4] = { 0,0,0,1 };

extern "C"
void lab_imgui_init_window(const char* window_name, GLFWwindow* window);

extern "C"
bool lab_imgui_create_window(const char* window_name, int width, int height,
    void (*custom_frame)(void))
{
    frame_cb = custom_frame;

    glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
    if (_backendType == wgpu::BackendType::OpenGL) {
        // Ask for OpenGL 4.4 which is what the GL backend requires for compute shaders and
        // texture views.
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#if __APPLE__
        // shouldn't actually get here.
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    }
    else if (_backendType == wgpu::BackendType::OpenGLES) {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
        glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
        glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_EGL_CONTEXT_API);
    }
    else {
        // Without this GLFW will initialize a GL context on the window, which prevents using
        // the window with other APIs (by crashing in weird ways).
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    }

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

    GLFWwindow* window = glfwCreateWindow(width, height, window_name, NULL, NULL);
    if (_backendType == wgpu::BackendType::OpenGL || _backendType == wgpu::BackendType::OpenGLES)
        glfwMakeContextCurrent(window);

    lab_imgui_init_window(window_name, window);
    //glfwMakeContextCurrent(_rootWindow);    // leave the root window bound

    while (lab_imgui_update(1.f / 60.f, true)) {
        lab_WindowState ws;
        lab_imgui_window_state("Hello LabImGui", &ws);
        if (!ws.valid)
            return true;

        if (_backendType == wgpu::BackendType::OpenGL || _backendType == wgpu::BackendType::OpenGLES)
            glfwMakeContextCurrent(window);

        //glClearColor(0.25f, 0.25f, 0.25f, 1.f);
        //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        if (frame_cb)
            frame_cb();
        else {
            // 1. Show a simple window
            // Tip: if we don't call ImGui::Begin()/ImGui::End() the widgets appears in a window automatically called "Debug"
            static float f = 0.0f;
            ImGui::Text("Hello, world!");
            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
            ImGui::ColorEdit3("clear color", clear_color);
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
        lab_imgui_render(&ws);
        lab_imgui_present(&ws);
    }
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
        if (glfwWindowShouldClose(w->second->window)) {
            ++close_count;
            if (auto_close) {
                glfwDestroyWindow(w->second->window);
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
void lab_imgui_render(const lab_WindowState* ws)
{
    glfwPollEvents();

    // Rendering
    ImGui::Render();

    // create the swap chain if necessary
    WindowData* data = nullptr;
    for (auto& it : _windows) {
        data = it.second.get();

        SyncTargetSwapChainDescFromWindow(data);
        if (!IsSameDescriptor(data->currentDesc, data->targetDesc) && !data->latched) {
            data->swapchain = device.CreateSwapChain(data->surface, &data->targetDesc);
            data->currentDesc = data->targetDesc;
        }
        break;
    }

    if (!data)
        return;

    wgpu::TextureView view = data->swapchain.GetCurrentTextureView();

    if (ws)
    {
        static ImVec4 clear_color = { 0.25f, 0.25f, 0.25f, 1.f };
        //glViewport(0, 0, ws->fb_width, ws->fb_height);
        WGPURenderPassColorAttachment color_attachments = {};
        color_attachments.loadOp = WGPULoadOp_Clear;
        color_attachments.storeOp = WGPUStoreOp_Store;
        color_attachments.clearColor = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
        color_attachments.view = view.Get();
        //color_attachments.resolveTarget = wgpuSwapChainGetCurrentTextureView(wgpu_swap_chain);
        WGPURenderPassDescriptor render_pass_desc = {};
        render_pass_desc.colorAttachmentCount = 1;
        render_pass_desc.colorAttachments = &color_attachments;
        render_pass_desc.depthStencilAttachment = NULL;

        WGPUCommandEncoderDescriptor enc_desc = {};
        WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(device.Get(), &enc_desc);

        WGPURenderPassEncoder pass = wgpuCommandEncoderBeginRenderPass(encoder, &render_pass_desc);
        ImGui_ImplWGPU_RenderDrawData(ImGui::GetDrawData(), pass);
        wgpuRenderPassEncoderEndPass(pass);

        WGPUCommandBufferDescriptor cmd_buffer_desc = {};
        WGPUCommandBuffer cmd_buffer = wgpuCommandEncoderFinish(encoder, &cmd_buffer_desc);
        WGPUQueue queue = device.GetQueue().Get();
        wgpuQueueSubmit(queue, 1, &cmd_buffer);

        data->swapchain.Present();
        return;
    }

#if 0
    // note: ImGui multi-window is still not ready for primetime, this loop
    // will have to change when ImGui multi-window really exists.
    // right now, this is only valid because only one window will ever be created.
    for (auto w = _windows.begin(); w != _windows.end(); ++w) {
        int display_w, display_h;
        glfwGetFramebufferSize(w->second->window, &display_w, &display_h);
        //glViewport(0, 0, display_w, display_h);
        ImGui_ImplWGPU_RenderDrawData(ImGui::GetDrawData(), pass);
    }
#endif
}

extern "C"
void lab_imgui_present(const lab_WindowState*)
{
    /// @TODO support multiple windows, via window state, not via loop over all windows

    if (_backendType == wgpu::BackendType::OpenGL || _backendType == wgpu::BackendType::OpenGLES)
        for (auto w = _windows.begin(); w != _windows.end(); ++w) {
            glfwSwapBuffers(w->second->window);
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

    auto i = _windows.begin();
    for (; i != _windows.end(); ++i) {
        if (i->second->name == window_name)
            break;
    }
    if (i == _windows.end())
        return;

    glfwGetWindowSize(i->second->window, &s->width, &s->height);
    glfwGetFramebufferSize(i->second->window, &s->fb_height, &s->fb_height);
    s->valid = (s->width > 0) && (s->height > 0);
}


#if defined(_WIN32)
static std::unique_ptr<wgpu::ChainedStruct> SetupWindowAndGetSurfaceDescriptorForTesting(
    GLFWwindow* window) {
    std::unique_ptr<wgpu::SurfaceDescriptorFromWindowsHWND> desc =
        std::make_unique<wgpu::SurfaceDescriptorFromWindowsHWND>();
    desc->hwnd = glfwGetWin32Window(window);
    desc->hinstance = GetModuleHandle(nullptr);
    return std::move(desc);
}
#elif defined(DAWN_USE_X11)
static std::unique_ptr<wgpu::ChainedStruct> SetupWindowAndGetSurfaceDescriptorForTesting(
    GLFWwindow* window) {
    std::unique_ptr<wgpu::SurfaceDescriptorFromXlib> desc =
        std::make_unique<wgpu::SurfaceDescriptorFromXlib>();
    desc->display = glfwGetX11Display();
    desc->window = glfwGetX11Window(window);
    return std::move(desc);
}
#elif defined(DAWN_ENABLE_BACKEND_METAL)
// SetupWindowAndGetSurfaceDescriptorForTesting defined in GLFWUtils_metal.mm
#else
static std::unique_ptr<wgpu::ChainedStruct> SetupWindowAndGetSurfaceDescriptorForTesting(GLFWwindow*) {
    return nullptr;
}
#endif

static wgpu::Surface CreateSurfaceForWindow(wgpu::Instance instance, GLFWwindow* window) {
    std::unique_ptr<wgpu::ChainedStruct> chainedDescriptor =
        SetupWindowAndGetSurfaceDescriptorForTesting(window);

    wgpu::SurfaceDescriptor descriptor;
    descriptor.nextInChain = chainedDescriptor.get();
    wgpu::Surface surface = instance.CreateSurface(&descriptor);
    return surface;
}

extern "C"
void lab_imgui_init_window(const char* window_name, GLFWwindow* window)
{
    // Setup the device on that adapter.
    device = wgpu::Device::Acquire(chosenAdapter.CreateDevice());
    device.SetUncapturedErrorCallback(
        [](WGPUErrorType errorType, const char* message, void*) {
            const char* errorTypeName = "";
            switch (errorType) {
            case WGPUErrorType_Validation:
                errorTypeName = "Validation";
                break;
            case WGPUErrorType_OutOfMemory:
                errorTypeName = "Out of memory";
                break;
            case WGPUErrorType_Unknown:
                errorTypeName = "Unknown";
                break;
            case WGPUErrorType_DeviceLost:
                errorTypeName = "Device lost";
                break;
            default:
                printf("Shouldn't get here\n");
                return;
            }
            printf("error: %s\n", message);
        },
        nullptr);
    queue = device.GetQueue();

    wgpu::SwapChainDescriptor descriptor;
    descriptor.usage = wgpu::TextureUsage::RenderAttachment;
    descriptor.format = wgpu::TextureFormat::BGRA8Unorm;
    descriptor.width = 0;
    descriptor.height = 0;
    descriptor.presentMode = wgpu::PresentMode::Fifo;

    std::unique_ptr<WindowData> data = std::make_unique<WindowData>();
    data->name = window_name;
    data->window = window;
    data->serial = windowSerial++;
    data->surface = CreateSurfaceForWindow(instance->Get(), window);
    data->currentDesc = descriptor;
    data->targetDesc = descriptor;
    SyncTargetSwapChainDescFromWindow(data.get());

    _windows[window] = std::move(data);


    // Setup Dear ImGui binding
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls

    ImGui_ImplGlfw_InitForOther(window, true);
    ImGui_ImplWGPU_Init(device.Get(), 3, WGPUTextureFormat_RGBA8Unorm);

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
    ImGui_ImplWGPU_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    for (auto i = _windows.begin(); i != _windows.end(); ++i) {
        glfwDestroyWindow(i->second->window);
    }

    glfwTerminate();
}

extern "C"
void lab_imgui_new_docking_frame(const lab_WindowState* ws)
{
    if (!ws->valid /* || !ws->bound */)
        return;

    // Start the ImGui frame
    ImGui_ImplWGPU_NewFrame();
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
