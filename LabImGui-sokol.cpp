
#include "LabImgui/LabImGui.h"
#include "sokol_app.h"
#include "sokol_gfx.h"
#include "sokol_time.h"
#include "sokol_glue.h"
#include "sokol_gl.h"
#include "sokol_gp.h"
#include "imgui.h"
#include "sokol_imgui.h"
#include "imgui_internal.h"
#include "implot.h"

#include "cute_aseprite.h"
#include "cute_png.h"
#include "cute_spritebatch.h"

void font_demo_init(const char* asset_root);
void fontDemo(float& dx, float& dy, float sx, float sy);

static sg_pass_action pass_action;
static uint64_t last_time = 0;
static bool show_test_window = true;
static bool show_another_window = false;
static const char* arg0 = nullptr;
static const char* asset_root = nullptr;

static sgp_vec2 points_buffer[4096];
static const float PI = 3.14159265358979323846264338327f;

typedef struct {
    int w, h;
    uint8_t* rgba;
} Sprite;


struct SpriteEngine {
    cp_image_t png;
    spritebatch_config_t config;
    spritebatch_t batch;
    ase_t* robots_ase;
    sg_image robots_sheet;
    sg_image sprite_textures[32];
    sg_image bench_image;
    spritebatch_sprite_t sprite;
    int i = 0;
    int call_count = 0;

    int sprite_count = 0;
    Sprite** sprites = nullptr;
};

static SpriteEngine* sprite_engine = nullptr;


static sg_image create_image(int width, int height) {
    size_t num_pixels = (size_t)(width * height * 4);
    unsigned char* data = (unsigned char*)malloc(num_pixels);
    assert(data);
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            data[y * width * 4 + x * 4 + 0] = (x * 255) / width;
            data[y * width * 4 + x * 4 + 1] = (y * 255) / height;
            data[y * width * 4 + x * 4 + 2] = 255 - (x * y * 255) / (width * height);
            data[y * width * 4 + x * 4 + 3] = 255;
        }
    }
    sg_image_desc image_desc;
    memset(&image_desc, 0, sizeof(sg_image_desc));
    image_desc.width = width;
    image_desc.height = height;
    image_desc.wrap_u = SG_WRAP_CLAMP_TO_EDGE;
    image_desc.wrap_v = SG_WRAP_CLAMP_TO_EDGE;
    image_desc.data.subimage[0][0].ptr = data;
    image_desc.data.subimage[0][0].size = num_pixels;
    sg_image image = sg_make_image(&image_desc);
    free(data);
    assert(sg_query_image_state(image) == SG_RESOURCESTATE_VALID);
    return image;
}



// callbacks for cute_spritebatch.h
static void batch_report(spritebatch_sprite_t* sprites, int count, int texture_w, int texture_h, void* udata)
{
    SpriteEngine* se = (SpriteEngine*)udata;
    ++se->call_count;

    static bool once = true;
    if (once) {
        se->bench_image = create_image(32, 32);
        once = false;
    }

    (void)texture_w;
    (void)texture_h;

#if 0
    for (int i = 0; i < count; ++i) {
        spritebatch_sprite_t* spr = &sprites[i];

        sgp_set_color(1.0f, 1.0f, 1.0f, 1.0f);
        unsigned int count = 0;
        sgp_irect viewport = sgp_query_state()->viewport;
        sgp_vec2 c = { viewport.w / 2.0f, viewport.h / 2.0f };
        points_buffer[count++] = c;
        sgp_vec2 v = { spr->x, spr->y };
        points_buffer[count++] = v;
        v = { spr->x + spr->w, spr->y + spr->h };
        points_buffer[count++] = v;
        sgp_draw_lines_strip(points_buffer, count);
    }
#endif
    for (int i = 0; i < count; ++i) {
        spritebatch_sprite_t* spr = &sprites[i];
        //sgp_set_image(0, se->bench_image);
        sgp_set_image(0, se->sprite_textures[spr->texture_id]);
        // @todo set rotation
        sgp_draw_textured_rect(spr->x, spr->y, spr->w, spr->h);
    }
}

// given the user supplied image_id, copy the raw pixels for that image into buffer
static void get_pixels(SPRITEBATCH_U64 image_id, void* buffer, int bytes_to_fill, void* udata)
{
    SpriteEngine* se = (SpriteEngine*)udata;
    memcpy(buffer, se->sprites[image_id]->rgba, bytes_to_fill);
}

static SPRITEBATCH_U64 generate_texture_handle(void* pixels, int w, int h, void* udata)
{
    if (!pixels)
        return 0;

    SpriteEngine* se = (SpriteEngine*)udata;
    se->sprite_textures[se->i] = sg_alloc_image();
    sg_image_desc img_desc;
    memset(&img_desc, 0, sizeof(img_desc));
    img_desc.width = w;
    img_desc.height = h;
    img_desc.pixel_format = SG_PIXELFORMAT_RGBA8;
    img_desc.wrap_u = SG_WRAP_REPEAT;
    img_desc.wrap_v = SG_WRAP_REPEAT;
    img_desc.min_filter = SG_FILTER_LINEAR;
    img_desc.mag_filter = SG_FILTER_LINEAR;
    img_desc.data.subimage[0][0] = {
        pixels, (size_t)(img_desc.width * img_desc.height * 4)
    };
    sg_init_image(se->sprite_textures[se->i], &img_desc);
    ++se->i;
    return (SPRITEBATCH_U64) se->i - 1;
}

static void destroy_texture_handle(SPRITEBATCH_U64 texture_id, void* udata)
{
    SpriteEngine* se = (SpriteEngine*)udata;
    sg_dealloc_image(se->sprite_textures[texture_id]);
}


static void blit(
    uint8_t* src,  int src_x,  int src_y,  int src_stride,
    uint8_t* dest, int dest_x, int dest_y, int dest_stride,
    int w, int h, int bpp)
{
    char* src_ptr = (char*)(uintptr_t)src + (src_y * src_stride) + (src_x * bpp);
    char* dest_ptr = (char*)(uintptr_t)dest + (dest_y * dest_stride) + (dest_x * bpp);

    for (int y = 0; y < h; y++)
    {
        memcpy(dest_ptr, src_ptr, w * bpp);
        src_ptr += src_stride;
        dest_ptr += dest_stride;
    }
}


SpriteEngine* sprite_engine_init(const char* asset_root)
{
    SpriteEngine* se = (SpriteEngine*)malloc(sizeof(SpriteEngine));
    if (!se)
        return se;

    memset(se, 0, sizeof(SpriteEngine));

    char buf[1024];
    strncpy(buf, asset_root, sizeof(buf));
    buf[1023] = '\0';
    int sz = strlen(asset_root);
    if (sz < 1000) {
        if (buf[sz - 1] != '/') {
            buf[sz] = '/';
            buf[sz + 1] = '\0';
            ++sz;
        }

        //strncpy(&buf[sz], "robots1.png", sizeof("robots1.png"));
        //se->png = cp_load_png(buf);
        //strncpy(&buf[sz], "robots1.ase", sizeof("robots1.ase"));
        strncpy(&buf[sz], "invaders.ase", sizeof("invaders.ase"));
        se->robots_ase = cute_aseprite_load_from_file(buf, NULL);

        if (se->robots_ase) {
            printf("ase:\nframes %d\n", se->robots_ase->frame_count);
            for (int i = 0; i < se->robots_ase->frame_count; ++i) {
                auto& frame = se->robots_ase->frames[i];
                printf(" %d --- cels: %d %d\n", i, frame.cel_count, frame.duration_milliseconds);
            }
        }


        if (se->robots_ase && se->robots_ase->frame_count > 0) {
            int w = se->robots_ase->frame_count * se->robots_ase->w;
            int h = se->robots_ase->h;

            if (se->sprites) {
                for (int i = 0; i < se->sprite_count; ++i) {
                    if (se->sprites[i]) {
                        free(se->sprites[i]->rgba);
                        free(se->sprites[i]);
                    }
                }
                free(se->sprites);
            }

            // copy the ase frame data into sprites
            se->sprite_count = se->robots_ase->frame_count;
            se->sprites = (Sprite**)malloc(sizeof(Sprite*) * se->robots_ase->frame_count);
            for (int i = 0; i < se->robots_ase->frame_count; ++i) {
                se->sprites[i] = (Sprite*)malloc(sizeof(Sprite));
                memset(se->sprites[i], 0, sizeof(Sprite));
                se->sprites[i]->w = se->robots_ase->frames[i].cels[0].w;
                se->sprites[i]->h = se->robots_ase->frames[i].cels[0].h;
                int sz = 4 * se->sprites[i]->w * se->sprites[i]->h;
                se->sprites[i]->rgba = (uint8_t*) malloc(sz);
                memcpy(se->sprites[i]->rgba, se->robots_ase->frames[i].cels[0].pixels, sz);
            }
/*
            se->robots_sheet = sg_alloc_image();
            sg_image_desc img_desc;
            memset(&img_desc, 0, sizeof(img_desc));
            img_desc.width = w;
            img_desc.height = h;
            img_desc.pixel_format = SG_PIXELFORMAT_RGBA8;
            img_desc.wrap_u = SG_WRAP_REPEAT;
            img_desc.wrap_v = SG_WRAP_REPEAT;
            img_desc.min_filter = SG_FILTER_LINEAR;
            img_desc.mag_filter = SG_FILTER_LINEAR;
            img_desc.data.subimage[0][0] = {
                data, (size_t) (img_desc.width * img_desc.height * 4)
            };
            sg_init_image(se->robots_sheet, &img_desc);
*/
        }

        spritebatch_set_default_config(&se->config);
        se->config.batch_callback = batch_report;                       // report batches of sprites from `spritebatch_flush`
        se->config.get_pixels_callback = get_pixels;                    // used to retrieve image pixels from `spritebatch_flush` and `spritebatch_defrag`
        se->config.generate_texture_callback = generate_texture_handle; // used to generate a texture handle from `spritebatch_flush` and `spritebatch_defrag`
        se->config.delete_texture_callback = destroy_texture_handle;    // used to destroy a texture handle from `spritebatch_defrag`
        spritebatch_init(&se->batch, &se->config, se);
    }
    return se;
}

void sprite_engine_dealloc(SpriteEngine* se)
{
    spritebatch_term(&se->batch);
    cute_aseprite_free(se->robots_ase);
    // free (se->png)
    free(se);
}

void draw_sprite(SpriteEngine* se, float x, float y, float rot, int sprite)
{
    float sx = 1.f, sy = 1.f;
    float theta_radians = 0.f;
    int sortbits = 0;
    spritebatch_sprite_t spr{
        (SPRITEBATCH_U64) sprite, // image_id
        0, // texture_id, filled in
        se->sprites[sprite]->w, se->sprites[sprite]->h,
        x, y,
        sx, sy,
        cosf(theta_radians), sinf(theta_radians),
        0, 0, 0, 0, // internal values
        0, // sort_bits
        //(void*)se // userdata
    };
    spritebatch_push(&se->batch, spr);
}

void spriteengine_update(SpriteEngine* se)
{
    // Run tinyspritebatch to find sprite batches.
    // This is the most basic usage of tinypsritebatch, one defrag, tick and flush per game loop.
    // It is also possible to only use defrag once every N frames.
    // tick can also be called at different time intervals (for example, once per game update
    // but not necessarily once per screen render).
    spritebatch_defrag(&se->batch);
    spritebatch_tick(&se->batch);
    spritebatch_flush(&se->batch);
}

void sprite_demo(SpriteEngine* se)
{
    for (float x = 0; x < 300; x += 30)
        for (float y = 0; y < 300; y += 30)
            draw_sprite(se, x, y, 0, 0);
}
























static void draw_rects(void) {
    sgp_irect viewport = sgp_query_state()->viewport;
    int width = viewport.w, height = viewport.h;
    int size = 64;
    int hsize = size / 2;
    float time = sapp_frame_count() / 60.0f;
    float t = (1.0f + sinf(time)) / 2.0f;

    // left
    sgp_push_transform();
    sgp_translate(width * 0.25f - hsize, height * 0.5f - hsize);
    sgp_translate(0.0f, 2 * size * t - size);
    sgp_set_color(t, 0.3f, 1.0f - t, 1.0f);
    sgp_draw_filled_rect(0, 0, size, size);
    sgp_pop_transform();

    // middle
    sgp_push_transform();
    sgp_translate(width * 0.5f - hsize, height * 0.5f - hsize);
    sgp_rotate_at(time, hsize, hsize);
    sgp_set_color(t, 1.0f - t, 0.3f, 1.0f);
    sgp_draw_filled_rect(0, 0, size, size);
    sgp_pop_transform();

    // right
    sgp_push_transform();
    sgp_translate(width * 0.75f - hsize, height * 0.5f - hsize);
    sgp_scale_at(t + 0.25f, t + 0.5f, hsize, hsize);
    sgp_set_color(0.3f, t, 1.0f - t, 1.0f);
    sgp_draw_filled_rect(0, 0, size, size);
    sgp_pop_transform();
}

static void draw_points(void) {
    sgp_set_color(1.0f, 1.0f, 1.0f, 1.0f);
    sgp_irect viewport = sgp_query_state()->viewport;
    int width = viewport.w, height = viewport.h;
    unsigned int count = 0;
    for (int y = 64; y < height - 64 && count < 4096; y += 8) {
        for (int x = 64; x < width - 64 && count < 4096; x += 8) {
            sgp_vec2 v = { (float)x,(float)y };
            points_buffer[count++] = v;
        }
    }
    sgp_draw_points(points_buffer, count);
}

static void draw_lines(void) {
    sgp_set_color(1.0f, 1.0f, 1.0f, 1.0f);
    unsigned int count = 0;
    sgp_irect viewport = sgp_query_state()->viewport;
    sgp_vec2 c = { viewport.w / 2.0f, viewport.h / 2.0f };
    points_buffer[count++] = c;
    for (float theta = 0.0f; theta <= PI * 8.0f; theta += PI / 16.0f) {
        float r = 10.0f * theta;
        sgp_vec2 v = { c.x + r * cosf(theta), c.y + r * sinf(theta) };
        points_buffer[count++] = v;
    }
    sgp_draw_lines_strip(points_buffer, count);
}

static void draw_triangles(void) {
    sgp_irect viewport = sgp_query_state()->viewport;
    int width = viewport.w, height = viewport.h;
    float hw = width * 0.5f;
    float hh = height * 0.5f;
    float w = height * 0.2f;
    float ax = hw - w, ay = hh + w;
    float bx = hw, by = hh - w;
    float cx = hw + w, cy = hh + w;
    sgp_set_color(1.0f, 0.0f, 1.0f, 1.0f);
    sgp_push_transform();
    sgp_translate(-w * 1.5f, 0.0f);
    sgp_draw_filled_triangle(ax, ay, bx, by, cx, cy);
    sgp_translate(w * 3.0f, 0.0f);
    unsigned int count = 0;
    float step = (2.0f * PI) / 6.0f;
    for (float theta = 0.0f; theta <= 2.0f * PI + step * 0.5f; theta += step) {
        sgp_vec2 v = { hw + w * cosf(theta), hh - w * sinf(theta) };
        points_buffer[count++] = v;
        if (count % 3 == 1) {
            sgp_vec2 u = { hw, hh };
            points_buffer[count++] = u;
        }
    }
    sgp_set_color(0.0f, 1.0f, 1.0f, 1.0f);
    sgp_draw_filled_triangles_strip(points_buffer, count);
    sgp_pop_transform();
}









extern "C"
bool lab_imgui_init(const char* arg0_, const char* asset_root_)
{
    if (!asset_root_) {
        printf("asset root not defined\n");
        return false;
    }
    else
        printf("asset root:%s\n", asset_root_);
    arg0 = strdup(arg0_);
    asset_root = strdup(asset_root_);
    sprite_engine = sprite_engine_init(asset_root);
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
    s->fb_width = sapp_width();     // @TODO handle retina
    s->fb_height = sapp_height();   // @TODO handle retina
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

    /* setup sokol-gl */
    sgl_desc_t desc_t;
    memset(&desc_t, 0, sizeof(desc_t));
    sgl_setup(&desc_t);

    // initialize Sokol GP
    sgp_desc sgpdesc = { 0 };
    sgp_setup(&sgpdesc);
    if (!sgp_is_valid()) {
        fprintf(stderr, "Failed to create Sokol GP context: %s\n", sgp_get_error_message(sgp_get_last_error()));
    }

    // use sokol-imgui with all default-options (we're not doing
    // multi-sampled rendering or using non-default pixel formats)
    simgui_desc_t simgui_desc = { };
    simgui_setup(&simgui_desc);
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImPlot::CreateContext();

    font_demo_init(asset_root);


    // initial clear color
    pass_action.colors[0].action = SG_ACTION_CLEAR;
    pass_action.colors[0].value = { 0.0f, 0.5f, 0.7f, 1.0f };
}

static void (*frame_cb)(void) = nullptr;

static void frame(void) {
    const int width = sapp_width();
    const int height = sapp_height();
    const double delta_time = stm_sec(stm_laptime(&last_time));
    const float dpi_scale = 1.f;

    // sgp demo
    if (true)
    {
        // begin draw commands queue
        int width = sapp_width(), height = sapp_height();
        sgp_begin(width, height);

        int hw = width / 2;
        int hh = height / 2;

        // draw background
        sgp_set_color(0.05f, 0.05f, 0.05f, 1.0f);
        sgp_clear();
        sgp_reset_color();

        // top left
        sgp_viewport(0, 0, hw, hh);
        sgp_set_color(0.1f, 0.1f, 0.1f, 1.0f);
        sgp_clear();
        sgp_reset_color();
        sgp_push_transform();
        sgp_translate(0.0f, -hh / 4.0f);
        draw_rects();
        sgp_pop_transform();
        sgp_push_transform();
        sgp_translate(0.0f, hh / 4.0f);
        sgp_scissor(0, 0, hw, 3.0f * hh / 4.0f);
        draw_rects();
        sgp_reset_scissor();
        sgp_pop_transform();

        // top right
        sgp_viewport(hw, 0, hw, hh);
//        draw_triangles();

        // bottom left
        sgp_viewport(0, hh, hw, hh);
        //draw_points();

        // bottom right
        sgp_viewport(hw, hh, hw, hh);
        sgp_set_color(0.1f, 0.1f, 0.1f, 1.0f);
        sgp_clear();
        sgp_reset_color();
        draw_lines();

        sgp_viewport(hw, 0, hw, hh);
        sprite_demo(sprite_engine);
        spriteengine_update(sprite_engine);
    }

    // sokol_gl
    {
        sgl_defaults();
        sgl_matrix_mode_projection();
        sgl_viewport(0, 0, width, height, true);
        sgl_ortho(0.0f, (float)sapp_width(), (float)sapp_height(), 0.0f, -1.0f, +1.0f);
        sgl_scissor_rect(0, 0, sapp_width(), sapp_height(), true);

        float sx, sy, dx, dy, lh = 0.0f;
        const float dpis = 1.f;
        sx = 50 * dpis; sy = 50 * dpis;
        dx = sx; dy = sy;

        fontDemo(dx, dy, sx, sy);

    }

    simgui_frame_desc_t frame_desc{
        width, height, delta_time, dpi_scale
    };
    simgui_new_frame(&frame_desc);

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

    // sgp draw
    {
        sgp_flush();
        sgp_end();
    }

    sgl_draw();

    simgui_render();
    sg_end_pass();
    sg_commit();
}

static void cleanup(void) {
    ImPlot::DestroyContext();
    simgui_shutdown();
    sgp_shutdown();
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
