cmake_minimum_required(VERSION 3.19)
project(LabImgui)
set(LABIMGUI_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
cmake_policy(SET CMP0077 NEW)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# mac-metal-foundation
# mac-gl-glfw
# mac-metal-sokol
# mac-gl-sokol
#
# win-gl-glfw
# win-gl-sokol
# win-dawn-glfw

if (APPLE)
    set(LABAPP_BACKEND "mac-metal-foundation" CACHE STRING "Backend")
    #set(LABAPP_BACKEND "mac-gl-glfw" CACHE STRING "Backend")
elseif (WIN32)
    set(LABAPP_BACKEND "win-dawn-glfw" CACHE STRING "Backend")
    #set(LABAPP_BACKEND "win-gl-glfw" CACHE STRING "Backend")
endif()

if("${LABAPP_BACKEND}" STREQUAL "win-dawn-glfw")
    set(IMGUI_BACKEND_DAWN ON)
    set(IMGUI_BACKEND_GLFW ON)
    set(IMGUI_BACKEND_OPENGL3 OFF)
    set(IMGUI_BACKEND_METAL OFF)
    set(IMGUI_BACKEND_OSX OFF)
    set(IMGUI_BACKEND_SOKOL OFF)
    set(IMGUI_BACKEND_WGPU ON)
    set(IMGUI_BACKEND_WINAPI ON) 
elseif("${LABAPP_BACKEND}" STREQUAL "win-gl-glfw")
    set(IMGUI_BACKEND_DAWN OFF)
    set(IMGUI_BACKEND_GLFW ON)
    set(IMGUI_BACKEND_OPENGL3 ON)
    set(IMGUI_BACKEND_METAL OFF)
    set(IMGUI_BACKEND_OSX OFF)
    set(IMGUI_BACKEND_SOKOL OFF)
    set(IMGUI_BACKEND_WGPU OFF)
    set(IMGUI_BACKEND_WINAPI OFF) 
elseif("${LABAPP_BACKEND}" STREQUAL "mac-metal-foundation")
    set(IMGUI_BACKEND_DAWN OFF)
    set(IMGUI_BACKEND_GLFW OFF)
    set(IMGUI_BACKEND_OPENGL3 OFF)
    set(IMGUI_BACKEND_METAL ON)
    set(IMGUI_BACKEND_OSX ON)
    set(IMGUI_BACKEND_SOKOL OFF)
    set(IMGUI_BACKEND_WGPU OFF)
    set(IMGUI_BACKEND_WINAPI OFF) 
endif()

set(WEBGPU_HEADER_LOCATION "${CMAKE_INSTALL_PREFIX}/include")

if (IMGUI_BACKEND_OPENGL3)
    find_package(OpenGL REQUIRED)
endif()
if (IMGUI_BACKEND_GLFW)
    include(glfw.cmake)
    add_library(glfw3 ALIAS glfw)
endif()

if (IMGUI_BACKEND_DAWN)
    include(LabSlang.cmake)
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
endif()
if (IMGUI_BACKEND_METAL)
    set(PLATFORM_SRC
        LabImGui-metal-cocoa.mm
        LabImgui/LabImgui-metal-cocoa.h)
endif()
if (IMGUI_BACKEND_WGPU)
    set(PLATFORM_SRC
        LabApp-wgpu-glfw.cpp)
    set(LABIMGUI_RENDER_SYSTEM
        dawncpp dawn_proc dawn_common dawn_native
        dawn_wire dawn_utils)
else()
endif()

set(LABIMGUI_PUBLIC_H
    LabImGui/LabImGui.h
    LabImgui/FontManager.h
    LabImgui/Widgets.h
    LabImgui/materialIconDefinitions.h
    LabImgui/meshula-icons.h
    LabImgui/roboto_regular.ttf.h
    LabImgui/robotomono_regular.ttf.h
)
add_library(LabImgui STATIC
    Widgets.cpp
    fontmanager.cpp
    ${LABIMGUI_PUBLIC_H}
    ${PLATFORM_SRC}
)
set_property(TARGET LabImgui PROPERTY CXX_STANDARD 17)

target_link_libraries(LabImgui Dear::Imgui ${BACKEND_LIB})
target_include_directories(LabImgui PUBLIC ${LABIMGUI_ROOT} ${imgui_SOURCE_DIR} ${glfw_SOURCE_DIR})

install(TARGETS LabImgui
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    FRAMEWORK DESTINATION lib
    BUNDLE DESTINATION bin
    RUNTIME DESTINATION bin)

install(FILES ${LABIMGUI_PUBLIC_H} DESTINATION include/LabImGui)

if (APPLE)
    set_property(TARGET LabImgui APPEND_STRING PROPERTY COMPILE_FLAGS "-fobjc-arc")
endif()

add_library(Lab::Imgui ALIAS LabImgui)

add_executable(LabImguiHello LabImguiHello.cpp)
target_link_libraries(LabImguiHello Dear::Imgui Lab::Imgui ${BACKEND_LIB} ${LABIMGUI_RENDER_SYSTEM})
set_property(TARGET LabImguiHello PROPERTY CXX_STANDARD 17)
install(TARGETS LabImguiHello
    BUNDLE DESTINATION bin
    RUNTIME DESTINATION bin)
