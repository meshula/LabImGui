
#ifndef LABIMGUI_H
#define LABIMGUI_H

#ifdef __cplusplus
#  define LABIMGUI_API extern "C"
#else
#  define LABIMGUI_API
#endif

typedef struct
{
    bool clicked, active, hovered;
} lab_FullScreenMouseState;

typedef struct
{
    int width, height;
    int fb_width, fb_height;
    bool valid;
} lab_WindowState;

// this must be called before OpenGL is invoked, otherwise
// on Windows, only GL 1.1 will be available.
LABIMGUI_API
bool lab_imgui_init(const char* arg0, const char* asset_root);

// create a glfw Imgui window and initialize the Imgui context
LABIMGUI_API
bool lab_imgui_create_window(const char* window_name, int width, int height,
    void (*custom_frame)(void));

LABIMGUI_API
void lab_imgui_window_state(const char* window_name, lab_WindowState * s);

LABIMGUI_API
void lab_imgui_shutdown();
LABIMGUI_API
void lab_imgui_new_docking_frame(const lab_WindowState*);
LABIMGUI_API
lab_FullScreenMouseState lab_imgui_begin_fullscreen_docking(const lab_WindowState*);
LABIMGUI_API
void lab_imgui_end_fullscreen_docking(const lab_WindowState*);

// update is to be called to pump the event loop before running the UI, or rendering
LABIMGUI_API
bool lab_imgui_update(float timeout_seconds, bool auto_close);
LABIMGUI_API
void lab_imgui_render(const lab_WindowState*);
LABIMGUI_API
void lab_imgui_present(const lab_WindowState*);

#endif
