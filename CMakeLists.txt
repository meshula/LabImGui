cmake_minimum_required(VERSION 3.19)
project(LabImgui)
set(LABIMGUI_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
cmake_policy(SET CMP0077 NEW)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

find_package(OpenGL REQUIRED)

#if arget named glfw is not defined, fetch it and build it
include(glfw.cmake)

# configure Dear Imgui, once again, fetching it if necessary
if(APPLE)
    set(IMGUI_BACKEND_OSX ON)
    set(IMGUI_BACKEND_METAL OFF)
    set(IMGUI_BACKEND_OPENGL3 ON)
 elseif(WINDOWS)
    set(IMGUI_BACKEND_OPENGL3 ON)
    set(IMGUI_BACKEND_WINAPI ON)
endif()
set(IMGUI_BACKEND_GLFW ON)
set(IMGUI_BACKEND_WGPU OFF)
set(IMGUI_BACKEND_DAWN OFF)
set(WEBGPU_HEADER_LOCATION "${CMAKE_INSTALL_PREFIX}/include")
include(imgui.cmake)

if (IMGUI_BACKEND_OPENGL3)
    set(PLATFORM_SRC
        LabImGui-gl-glfw.cpp
        GL/gl3w.c
        LabImgui/LabImgui-gl-glfw.h)
endif()

add_library(LabImgui STATIC
    Widgets.cpp
    fontmanager.cpp
    LabImgui/FontManager.h
    LabImgui/Widgets.h
    LabImgui/materialIconDefinitions.h
    LabImgui/meshula-icons.h
    LabImgui/roboto_regular.ttf.h
    LabImgui/robotomono_regular.ttf.h
    ${PLATFORM_SRC}
)
set_property(TARGET LabImgui PROPERTY CXX_STANDARD 17)
target_link_libraries(LabImgui Dear::Imgui glfw)
target_include_directories(LabImgui PUBLIC ${LABIMGUI_ROOT} ${imgui_SOURCE_DIR} ${glfw_SOURCE_DIR})
add_library(Lab::Imgui ALIAS LabImgui)

add_executable(LabImguiHello LabImguiHello.cpp)
target_link_libraries(LabImguiHello Dear::Imgui Lab::Imgui glfw ${LABIMGUI_RENDER_SYSTEM})
set_property(TARGET LabImguiHello PROPERTY CXX_STANDARD 17)
install(TARGETS LabImguiHello
    BUNDLE DESTINATION bin
    RUNTIME DESTINATION bin)

