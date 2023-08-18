cmake_minimum_required(VERSION 3.19)
project(LabImgui)
set(LABIMGUI_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
cmake_policy(SET CMP0077 NEW)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# configure Dear Imgui, once again, fetching it if necessary
if (APPLE)
    option(IMGUI_BACKEND_OSX "Native Mac backed" ON)
    option(IMGUI_BACKEND_METAL "Metal renderer" OFF)
    option(IMGUI_BACKEND_OPENGL3 "OpenGL renderer" ON)
    option(IMGUI_BACKEND_GLFW "GLFW backend" OFF)
elseif (WIN32)
    option(IMGUI_BACKEND_OPENGL3 "OpenGL renderer" ON)
    option(IMGUI_BACKEND_SOKOL "Sokol backend" OFF)
    option(IMGUI_BACKEND_GLFW "GLFW backend" ON)
    option(IMGUI_BACKEND_WINAPI "Windows native interfaces" ON) 
endif()

option(IMGUI_BACKEND_WGPU "Use wgpu API backend" OFF)
option(IMGUI_BACKEND_DAWN "Use Dawn renderer backend" OFF)
set(WEBGPU_HEADER_LOCATION "${CMAKE_INSTALL_PREFIX}/include")

if (IMGUI_BACKEND_OPENGL3)
    find_package(OpenGL REQUIRED)
endif()
if (IMGUI_BACKEND_GLFW)
    include(glfw.cmake)
endif()

include(imgui.cmake)

if (IMGUI_BACKEND_SOKOL)
    include(sokol.cmake)
    set(BACKEND_LIB sokol::sokol)
    set(PLATFORM_SRC
        LabImGui-sokol.cpp)
endif()

if (IMGUI_BACKEND_OPENGL3 AND IMGUI_BACKEND_GLFW)
    set(PLATFORM_SRC
        LabImGui-gl-glfw.cpp
        GL/gl3w.c)
    set(BACKEND_LIB glfw)
elseif (IMGUI_BACKEND_METAL AND IMGUI_BACKEND_OSX)
    set(PLATFORM_SRC
        LabImGui-metal-cocoa.mm)
elseif (IMGUI_BACKEND_OPENGL3 AND IMGUI_BACKEND_OSX)
    set(PLATFORM_SRC
        LabImGui-gl-cocoa.mm)
endif()


add_library(LabImgui STATIC
    Widgets.cpp
    fontmanager.cpp
    LabImGui/LabImGui.h
    LabImgui/FontManager.h
    LabImgui/Widgets.h
    LabImgui/materialIconDefinitions.h
    LabImgui/meshula-icons.h
    LabImgui/roboto_regular.ttf.h
    LabImgui/robotomono_regular.ttf.h
    ${PLATFORM_SRC}
)
set_property(TARGET LabImgui PROPERTY CXX_STANDARD 17)

target_link_libraries(LabImgui Dear::Imgui ${BACKEND_LIB})
target_include_directories(LabImgui PUBLIC ${LABIMGUI_ROOT} ${imgui_SOURCE_DIR} ${glfw_SOURCE_DIR})

if (APPLE)
    set_property(TARGET LabImgui APPEND_STRING PROPERTY COMPILE_FLAGS "-fobjc-arc")
endif()

add_library(Lab::Imgui ALIAS LabImgui)

if (APPLE)
    set(APPLE_FRAMEWORKS
        "-framework AppKit"
        "-framework CoreGraphics"
        "-framework Foundation"
        "-framework GameController"
        "-framework Metal"
        "-framework MetalKit"
        "-framework QuartzCore"
        "-framework OpenGL"
    )
    set(MAIN_EXT "mm")
else()
    set(MAIN_EXT "cpp")
endif()

add_executable(LabImguiHello LabImguiHello.${MAIN_EXT})
target_link_libraries(LabImguiHello Dear::Imgui Lab::Imgui 
    ${BACKEND_LIB} ${LABIMGUI_RENDER_SYSTEM}
    ${APPLE_FRAMEWORKS})


set_property(TARGET LabImguiHello PROPERTY CXX_STANDARD 17)
install(TARGETS LabImguiHello
    BUNDLE DESTINATION bin
    RUNTIME DESTINATION bin)
