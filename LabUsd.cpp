
#pragma warning(disable:4244)
#pragma warning(disable:4305)

#include <pxr/base/tf/weakBase.h>
#include <pxr/base/tf/weakPtr.h>
#include <pxr/usd/usd/notice.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usdGeom/xformable.h>
#include <pxr/base/plug/registry.h>
#include <pxr/usd/usd/primRange.h>
#include <pxr/usd/usdGeom/tokens.h>
#include <pxr/usd/ar/resolver.h>
#include <pxr/usd/usdGeom/basisCurves.h>
#include <pxr/usd/usdGeom/camera.h>
#include <pxr/usd/usdGeom/capsule.h>
#include <pxr/usd/usdGeom/cone.h>
#include <pxr/usd/usdGeom/cube.h>
#include <pxr/usd/usdGeom/cylinder.h>
#include <pxr/usd/usdGeom/mesh.h>
#include <pxr/usd/usdGeom/nurbsCurves.h>
#include <pxr/usd/usdGeom/nurbsPatch.h>
#include <pxr/usd/usdGeom/points.h>
#include <pxr/usd/usdGeom/pointInstancer.h>
#include <pxr/usd/usdGeom/scope.h>
#include <pxr/usd/usdGeom/sphere.h>
#include <pxr/usd/usdGeom/xform.h>

#include "stf.hpp"

#define IMGUI_DEFINE_MATH_OPERATORS 1
#include "LabImgui/LabImGui.h"
#include "LabDirectories.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "implot.h"


#include "cute_aseprite.h"
#include "cute_png.h"
#include "cute_spritebatch.h"

#include "tinycolormap.hpp"
#include <parallel_hashmap/phmap.h>
using phmap::flat_hash_map;
using phmap::flat_hash_set;

#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

using std::map;
using std::set;
using std::string;
using std::vector;

const float PI = 3.14159265358979323846264338327f;

PXR_NAMESPACE_OPEN_SCOPE

/// Sets the paths to the bootstrap plugInfo JSON files, also any diagnostic
/// messages that should be reported when plugins are registered (if any).
/// The priority order of elements of the path is honored if pathsAreOrdered.
/// Defined in registry.cpp.
void Plug_SetPaths(const std::vector<std::string>&,
                   const std::vector<std::string>&,
                   bool pathsAreOrdered);

PXR_NAMESPACE_CLOSE_SCOPE

PXR_NAMESPACE_USING_DIRECTIVE

struct usd_prim_data_t {
    int face_count = 0;
};

struct usd_statistics_t {
    flat_hash_set<UsdPrim> xforms;
    flat_hash_set<UsdPrim> cameras;
    flat_hash_set<UsdPrim> meshes;
    flat_hash_set<UsdPrim> curves;
    flat_hash_set<UsdPrim> cubes;
    flat_hash_set<UsdPrim> spheres;
    flat_hash_set<UsdPrim> cones;
    flat_hash_set<UsdPrim> cylinders;
    flat_hash_set<UsdPrim> capsules;
    flat_hash_set<UsdPrim> points;
    flat_hash_set<UsdPrim> nurbsPatches;
    flat_hash_set<UsdPrim> pointInstancers;

    flat_hash_map<UsdPrim, usd_prim_data_t> data;
    flat_hash_map<UsdPrim, usd_prim_data_t> accumulated_data;
};

struct usd_traverse_t {
    vector<UsdPrim> prims;
    vector<UsdPrim> instances;
    flat_hash_map<SdfPath, UsdPrim> prototypes;

    bool skip_guides = true;
    bool skip_invisible = true;
    int frame = 0;
    int invalid_instance_count = 0;
};

struct usd_filter_t {
    bool count_faces = false;
};


UsdPrim g_root;
usd_statistics_t g_stats;

class SceneProxy
: public TfWeakBase         // in order to register for Tf events
{
    // For changes from UsdStage.
    TfNotice::Key _objectsChangedNoticeKey;
public:
    UsdStageRefPtr stage;

    SceneProxy() = default;
    ~SceneProxy()
    {
        TfNotice::Revoke(_objectsChangedNoticeKey);
    }

    void _OnObjectsChanged(UsdNotice::ObjectsChanged const& notice, 
            UsdStageWeakPtr const& sender)
    {
        printf("GetResyncedPaths\n");
        auto pathsToResync = notice.GetResyncedPaths();
        for (auto & i : pathsToResync)
        {
            printf("%s\n", i.GetString().c_str());
        }
        printf("GetChangedInfoOnlyPaths\n");
        auto infoPaths = notice.GetChangedInfoOnlyPaths();
        for (auto & i : infoPaths)
        {
            printf("%s\n", i.GetString().c_str());
        }
    }

    void create_new_stage(std::string const& path)
    {
        TfNotice::Revoke(_objectsChangedNoticeKey);

        stage = UsdStage::CreateNew(path);

        // Start listening for change notices from this stage.
        auto self = TfCreateWeakPtr(this);
        _objectsChangedNoticeKey = TfNotice::Register(self, 
                &SceneProxy::_OnObjectsChanged, stage);

        // create a cube on the stage
        stage->DefinePrim(SdfPath("/Box"), TfToken("Cube"));
        UsdPrim cube = stage->GetPrimAtPath(SdfPath("/Box"));
        GfVec3f scaleVec = { 5.f, 5.f, 5.f };
        UsdGeomXformable cubeXf(cube);
        cubeXf.AddScaleOp().Set(scaleVec);
    }

    void load_stage(std::string const& filePath)
    {
        TfNotice::Revoke(_objectsChangedNoticeKey);

        printf("\nLoad_Stage : %s\n", filePath.c_str());
        auto supported = UsdStage::IsSupportedFile(filePath);
        if (supported)
        {
            printf("File format supported\n");
        }
        else
        {
            fprintf(stderr, 
                    "%s : File format not supported\n", filePath.c_str());
            return;
        }
 
        auto & resolver = ArGetResolver();
        auto resolverContext = resolver.CreateDefaultContextForAsset(filePath);
        auto resolvedPath = resolver.Resolve(filePath);
        // UsdStage::LoadNone would prevent loading references
        stage = UsdStage::Open(resolvedPath, 
                    resolverContext, UsdStage::LoadAll);

        if (stage)
        {
            auto pseudoRoot = stage->GetPseudoRoot();
            printf("Pseudo root path: %s\n",
                    pseudoRoot.GetPath().GetString().c_str());

            for (auto const& c : pseudoRoot.GetChildren())
            {
                printf("\tChild path: %s\n", c.GetPath().GetString().c_str());
            }
        }
        else
        {
            fprintf(stderr, "Stage was not loaded");
        }
    }

    void save_stage()
    {
        if (stage)
            stage->GetRootLayer()->Save();
    }

};


void usd_traverse(UsdPrim root_prim, usd_traverse_t& traverse)
{
    printf("usd_traverse begin\n");
    auto prim_range = UsdPrimRange::PreAndPostVisit(root_prim);
    for (auto prim_it = prim_range.begin(); 
            prim_it != prim_range.end(); ++prim_it) {
        if ((*prim_it).IsValid() && !prim_it.IsPostVisit()) {
            bool skip = false;
            if (*prim_it != root_prim) {
                if (traverse.skip_guides) {
                    TfToken purpose;
                    if (prim_it->GetAttribute(UsdGeomTokens->purpose).
                            Get(&purpose, traverse.frame)) {
                        skip = purpose == UsdGeomTokens->guide;
                    }
                }
                TfToken visibility;
                if (!skip && traverse.skip_invisible &&
                        prim_it->GetAttribute(UsdGeomTokens->visibility).
                            Get(&visibility, traverse.frame)) {
                    skip = visibility == UsdGeomTokens->invisible;
                }
            }
            if (skip) {
                prim_it.PruneChildren();
            }
            else {
                traverse.prims.push_back(*prim_it);
                if (prim_it->IsInstance()) {
                    UsdPrim prototype = prim_it->GetPrototype();
                    if (prototype.IsValid()) {
                        traverse.instances.push_back(*prim_it);
                        auto path = prototype.GetPath();
                        traverse.prototypes[path] = prototype;
                    }
                    else
                        ++traverse.invalid_instance_count;
                }
            }
        }
    }
    printf("usd_traverse end\n");
}


void usd_statistics(usd_traverse_t & traverse, usd_statistics_t & stats)
{
    printf("usd_statistics begin\n");
    
    // create this array outside the loop because .Get will re-use a container
    // to avoid allocation overhead of repeatedly creating a new one.
    VtIntArray face_vert_counts;
    for (auto& i : traverse.prims) {
        stats.data[i].face_count = 0;
        if (i.IsA<UsdGeomXform>()) { stats.xforms.insert(i); }
        else if (i.IsA<UsdGeomCamera>()) { stats.cameras.insert(i); }
        else if (i.IsA<UsdGeomMesh>()) { 
            stats.meshes.insert(i);
            UsdGeomMesh geom(i);
            geom.GetFaceVertexCountsAttr().Get(&face_vert_counts, 0);
            stats.data[i].face_count = (int) face_vert_counts.size();
        }
        else if (i.IsA<UsdGeomCurves>()) { stats.curves.insert(i); }
        else if (i.IsA<UsdGeomCube>()) { stats.cubes.insert(i); }
        else if (i.IsA<UsdGeomSphere>()) { stats.spheres.insert(i); }
        else if (i.IsA<UsdGeomCone>()) { stats.cones.insert(i); }
        else if (i.IsA<UsdGeomCylinder>()) { stats.cylinders.insert(i); }
        else if (i.IsA<UsdGeomCapsule>()) { stats.capsules.insert(i); }
        else if (i.IsA<UsdGeomPoints>()) { stats.points.insert(i); }
        else if (i.IsA<UsdGeomNurbsPatch>()) { stats.nurbsPatches.insert(i); }
        else if (i.IsA<UsdGeomPointInstancer>()) { stats.pointInstancers.insert(i); }
    }
    printf("usd_statistics end\n");
}

/*
 // render as a triangle strip
 void render_slice(
         float x, float y,
         float inner, float outer,
         float a0, float a1,
         float r, float g, float b, float a)
 {
     float step = 2.f * PI / 72.f; // 5 degree steps
     sgl_begin_triangle_strip();
     for (float th = a0; th <= a1 + step * 0.5f; th += step) {
         float innerx = x + inner * cosf(th);
         float innery = y + inner * sinf(th);
         float outerx = x + outer * cosf(th);
         float outery = y + outer * sinf(th);
         sgl_v2f_c4f(innerx, innery, r, g, b, a);
         sgl_v2f_c4f(outerx, outery, r, g, b, a);
     }
     sgl_end();
 }

 */

void RenderRingSlice(ImDrawList& DrawList,
                     const ImVec2& center, 
                     double inner_radius, double radius, 
                     double a0, double a1, ImU32 col) {
    static const float resolution = 50 / (2 * IM_PI);
    int n = ImMax(3, (int)((a1 - a0) * resolution));
    DrawList.PathClear();
    DrawList.PathArcTo(center, radius, a0, a1, n);
    DrawList.PathArcTo(center, inner_radius, a1, a0, n);

    // calculate the first point of the first arc:
    ImVec2 p0 = center + ImVec2(cosf(a0) * float(radius), sinf(a0) * float(radius));
    DrawList.PathLineTo(p0);
    //DrawList.PathFillConvex(col); // imgui doesn't fill concave
    //DrawList.PathStroke(IM_COL32(255, 255, 0, 255));
    DrawList.PathStroke(col);
}

float lerp(float x0, float x1, float u) {
    return x0 * (1.f - u) + x1 * u;
}

struct RenderSlice {
    UsdPrim prim;
    int level;
    float a0, a1;
    float r, g, b, a;
};

vector<RenderSlice> slices;
bool create_slices = true;

void RenderRadialChart(
        int depth,
        UsdPrim root, usd_statistics_t& stats, 
        float x, float y,
        float a0, float a1, float r0, float r1)
{
    auto dl = ImGui::GetWindowDrawList();
    if (!create_slices) {
        for (auto& i : slices) {
            float r0 = (float) i.level * 20;
            float r1 = r0 + 19;
            RenderRingSlice(*dl, ImVec2(x, y), r0, r1, i.a0, i.a1, 
                            IM_COL32((int)(i.r * 255.f), (int)(i.g * 255.f), (int)(i.b * 255.f), (int)(i.a * 255.f)));
        }
        return;
    }

    auto children = root.GetChildren();
    float total = (float) stats.accumulated_data[root].face_count;
    if (total < 1.f)
        return;

    float a2 = a0;
    for (auto i = children.begin(); i != children.end(); ++i) {
        int fc = stats.accumulated_data[*i].face_count;
        if (!fc)
            continue;

        float ctotal = (float) fc;
        float ratio = ctotal / total;
        float a3 = a2 + lerp(a0, a1, ratio) - a0;
        auto c = tinycolormap::GetCubehelixColor(ratio);
        if (a3 - a2 > 1e-2f) {
            RenderRingSlice(*dl, ImVec2(x, y), r0, r1, a2, a3, 
                            IM_COL32((int)(c.r() * 255.f), (int)(c.g() * 255.f), (int)(c.b() * 255.f), 255));
            if (create_slices) {
                slices.push_back((RenderSlice)
                        {*i, depth, a2, a3, 
                        (float) c.r(), (float) c.g(), (float) c.b(), 1.f});
            }
        }
        RenderRadialChart(depth + 1, *i, stats, x, y, a2, a3, r1 + 1, r1 + 20);
        a2 = a3;
    }
}

int thrognar(UsdPrim root, usd_statistics_t& stats) {
    auto children = root.GetChildren();
    int count = stats.data[root].face_count;
    for (auto i = children.begin(); i != children.end(); ++i) {
        count += thrognar(*i, stats);
    }
    stats.accumulated_data[root].face_count = count;
    return count;
}

void thrognar_print(UsdPrim root, usd_statistics_t& stats, int indent) {
    for (int i = 0; i < indent; ++i)
        printf(" ");
    printf("%d\n", stats.accumulated_data[root].face_count);
    auto children = root.GetChildren();
    for (auto i = children.begin(); i != children.end(); ++i) {
        thrognar_print(*i, stats, indent+3);
    }
}

void render_frame()
{

}


//Array<Rectangle> treemap;

void imgui_frame()
{
    lab_WindowState ws;
    lab_imgui_window_state("Hello LabImGui", &ws);
    if (!ws.valid)
        return;

    bool in_canvas = false;
    {
        ImGuiWindow* win = ImGui::GetCurrentWindow();
        ImRect edit_rect = win->ContentRegionRect;
        ImGuiIO& io = ImGui::GetIO();
        ImVec2 imgui_cursor_pos = ImGui::GetCursorPos();
        //ImGui::SetCursorScreenPos(edit_rect.Min);
        ImGui::SetCursorScreenPos((ImVec2) { 0,0 });

        //printf("%f %f %f %f\n", edit_rect.Min.x, edit_rect.Min.y, edit_rect.Max.x, edit_rect.Max.y);

        // detect that the mouse is in the content region
        bool click_finished = 
           ImGui::Button("GIZMOREGION", (ImVec2){1000, 1000});
        //bool click_finished = 
        //   ImGui::InvisibleButton("###GIZMOREGION", edit_rect.GetSize());
        in_canvas = click_finished || ImGui::IsItemHovered();

        //ImVec2 mouse_pos = io.MousePos - ImGui::GetCurrentWindow()->Pos;
        //mouse_state_update(mouse, mouse_pos.x, mouse_pos.y, 
        //        io.MouseDown[0] && io.MouseDownOwned[0]);

        // restore the ImGui state
        ImGui::SetCursorPos(imgui_cursor_pos);
    }
    //printf("%s\n", in_canvas? "in canvas" : "in gui");
   
    //------------ start the Dear ImGui portion

    lab_imgui_new_docking_frame(&ws);
    lab_imgui_begin_fullscreen_docking(&ws);

    //------------ custom begin

    ImGui::SetNextWindowSize((ImVec2){400.f, 500.f});
    ImGui::Begin("Scene Hierarchy");
    ImGui::Text("Want M input: %d", ImGui::GetIO().WantCaptureMouse);

    ImGui::Text("Canvas hovered: %s", ws.canvas_hovered? "1":"0");
    ImGui::Text("Canvas started: %s", ws.canvas_click_started? "1":"0");
    ImGui::Text("Canvas active: %s", ws.canvas_click_active? "1":"0");
    ImGui::Text("Canvas ended: %s", ws.canvas_click_ended? "1":"0");
    ImGui::Text("Canvas mouse: %f %f", ws.canvas_x, ws.canvas_y);

    /*
     * root 5
     *  child 1
     *    child 0
     *  child 2
     *    child 1
     *     child 0
     */

    struct H {
        string name;
        int all_count; };

    vector<H> usd_hierarchy;
    usd_hierarchy.push_back({"root", 5});
    usd_hierarchy.push_back({"child1", 1});
    usd_hierarchy.push_back({"child2", 0});
    usd_hierarchy.push_back({"child3", 2});
    usd_hierarchy.push_back({"child4", 1});
    usd_hierarchy.push_back({"child5", 0});

    vector<int> pop; 
    int id = 1999;
    int hier = 0;
    int tree_depth = 0;
    for (int hier = 0; hier < usd_hierarchy.size(); ) {
        ImGui::PushID(id++);
        auto& n = usd_hierarchy[hier];
        if (ImGui::TreeNode(n.name.c_str())) {
            if (n.all_count == 0) {
                ImGui::TreePop();
            }
            else {
                pop.push_back(1 + hier + n.all_count);
            }
            hier += 1;
        }
        else {
            hier += 1 + n.all_count;
            id += n.all_count;
        }
        ImGui::PopID();
        if (pop.size() > 0) {
            int check = pop.back();
            if (hier == check) {
                ImGui::TreePop();
                pop.pop_back();
            }
        }
    }
    while (pop.size() > 0) {
        ImGui::TreePop();
        pop.pop_back();
    }

   ImGui::End();

    {
        lab_WindowState ws;
        lab_imgui_window_state("Radial Chart", &ws);
        if (!ws.valid)
            return;
        
        float hw = 0.5f * ws.width;
        float hh = 0.5f * ws.height;
        
        RenderRadialChart(0, g_root, g_stats, hw, hh, 0, 2.f * PI, 0, 10);
        create_slices = false;
    }

    //static bool demo_window = false;
    //ImPlot::ShowDemoWindow(&demo_window);

    //------------ custom end

    lab_imgui_end_fullscreen_docking(&ws);
}

#define INSTALL_LOCN "/var/tmp/usddev0621"

int main(int argc, char** argv)
{
    //Plug_InitConfig();
    std::vector<std::string> test;
    std::vector<std::string> paths;
    paths.emplace_back(INSTALL_LOCN "/lib/usd");
    paths.emplace_back(INSTALL_LOCN "/plugin/usd");
    std::vector<std::string> msgs;
    msgs.emplace_back("looking for plugs here: " INSTALL_LOCN "/lib/usd");
    Plug_SetPaths(paths, msgs, true);

    SceneProxy scene;
    scene.create_new_stage("/var/tmp/test.usda");
    scene.save_stage();

    SceneProxy scene2;
    scene2.load_stage("/var/tmp/test.usda");

//  const char* usdFile = "/Users/nporcino/dev/assets/M2UniversityHall.usdc";
    const char* usdFile = "/Users/nporcino/dev/assets/PrvsIsland_set_flatten.usd";
    printf("Loading %s\n", usdFile);
    SceneProxy scene3;
    scene3.load_stage(usdFile);
    printf("Loaded %s\n", usdFile);

    //              _   _               
    //   __ _  __ _| |_| |__   ___ _ __ 
    //  / _` |/ _` | __| '_ \ / _ \ '__|
    // | (_| | (_| | |_| | | |  __/ |   
    //  \__, |\__,_|\__|_| |_|\___|_|   
    //  |___/  
    usd_traverse_t traverse;

    g_root = scene3.stage->GetPseudoRoot();
    
    // discover all the top level prims
    usd_traverse(scene3.stage->GetPseudoRoot(), traverse);

    printf("Traversed the stage\n");

    set<SdfPath> resolved_prototypes;

    // resolve all the prototypes
    while (resolved_prototypes.size() < traverse.prototypes.size()) {
        auto to_test = traverse.prototypes;
        for (auto & i : to_test) {
            if (resolved_prototypes.find(i.first) == resolved_prototypes.end()) {
                usd_traverse(i.second, traverse);
                resolved_prototypes.insert(i.first);
            }
        }
    }

    printf("Resolved the prototypes\n");

    //      _        _   _     _   _          
    //  ___| |_ __ _| |_(_)___| |_(_) ___ ___ 
    // / __| __/ _` | __| / __| __| |/ __/ __|
    // \__ \ || (_| | |_| \__ \ |_| | (__\__ \
    // |___/\__\__,_|\__|_|___/\__|_|\___|___/
    usd_statistics_t& stats = g_stats;
    usd_statistics(traverse, stats);
    printf("Stage has %d invalid instances.\n", traverse.invalid_instance_count);
    printf("Stage has %d instances.\n", (int) traverse.instances.size());
    printf("Stage has %d prims.\n", (int) traverse.prims.size());
    printf("Stage has %d xforms.\n", (int) stats.xforms.size());
    printf("Stage has %d cameras.\n", (int) stats.cameras.size());
    printf("Stage has %d meshes.\n", (int) stats.meshes.size());
    printf("Stage has %d curves.\n", (int) stats.curves.size());
    printf("Stage has %d cubes.\n", (int) stats.cubes.size());
    printf("Stage has %d spheres.\n", (int) stats.spheres.size());
    printf("Stage has %d cones.\n", (int) stats.cones.size());
    printf("Stage has %d cylinders.\n", (int) stats.cylinders.size());
    printf("Stage has %d points.\n", (int) stats.points.size());
    printf("Stage has %d nurbsPatches.\n", (int) stats.nurbsPatches.size());
    printf("Stage has %d pointInstancers.\n", (int) stats.pointInstancers.size());

    //map<UsdPrim, usd_prim_data_t> face_data;
    //usd_filter_t face_filter = { true };
    //usd_statistics_accumulate(face_filter, stats, face_data);

    //data = stats.data;
    //accumulate(scene3.stage->GetPseudoRoot());
    thrognar(scene3.stage->GetPseudoRoot(), stats);
    //thrognar_print(scene3.stage->GetPseudoRoot(), stats, 0);
    
    printf("aggregated facecounts\n");

    //  _   _ ___ 
    // | | | |_ _|
    // | | | || | 
    // | |_| || | 
    //  \___/|___|
    const char* asset_root = lab_application_resource_path(argv[0], 
                                "share/lab_font_demo/");
    lab_imgui_init(argv[0], asset_root);
    lab_imgui_create_window("Hello LabImGui", 2000, 1400, 
            render_frame, imgui_frame);
    lab_imgui_shutdown();
    return 0;
}

