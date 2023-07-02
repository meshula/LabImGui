cmake_minimum_required(VERSION 3.19)
project(LabImgui)
set(LABIMGUI_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

cmake_policy(SET CMP0077 NEW)
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW) # make submodules respect CMP0077

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(BUILD_USD ON)

# if LABAPP_BACKEND is not set, deduce it from the platform
if (NOT DEFINED LABAPP_BACKEND)
    # construct the variable as a string, platform-graphicssystem-windowingsystem
    # check LABAPP_GRAPHICS_SYSTEM, and LABAPP_WINDOWING_SYSTEM, use them if
    # they are set, and otherwise use the default for the platform

    if (APPLE)
        # construct a temporary string string starting with mac-
        set(BACKEND "mac-")
        # if LABAPP_GRAPHICS_SYSTEM is set, append it
        if (DEFINED LABAPP_GRAPHICS_SYSTEM)
            set(BACKEND "${BACKEND}${LABAPP_GRAPHICS_SYSTEM}-")
        else()
            # otherwise, use the default for the platform
            set(BACKEND "${BACKEND}metal-")
        endif()
        # if LABAPP_WINDOWING_SYSTEM is set, append it
        if (DEFINED LABAPP_WINDOWING_SYSTEM)
            set(BACKEND "${BACKEND}${LABAPP_WINDOWING_SYSTEM}")
        else()
            # otherwise, use the default for the platform
            set(BACKEND "${BACKEND}cocoa")
        endif()
        set(LABAPP_BACKEND ${BACKEND} CACHE STRING "Backend")
    elseif (WIN32)
        # construct a temporary string string starting with win-
        set(BACKEND "win-")
        # if LABAPP_GRAPHICS_SYSTEM is set, append it
        if (DEFINED LABAPP_GRAPHICS_SYSTEM)
            set(BACKEND "${BACKEND}${LABAPP_GRAPHICS_SYSTEM}-")
        else()
            # otherwise, use the default for the platform
            set(BACKEND "${BACKEND}d3d11-")
        endif()
        # if LABAPP_WINDOWING_SYSTEM is set, append it
        if (DEFINED LABAPP_WINDOWING_SYSTEM)
            set(BACKEND "${BACKEND}${LABAPP_WINDOWING_SYSTEM}")
        else()
            # otherwise, use the default for the platform
            set(BACKEND "${BACKEND}glfw")
        endif()
    else()
        message(FATAL_ERROR "Unknown platform")
    endif()
endif()

if("${LABAPP_BACKEND}" STREQUAL "win-wgpu-glfw")
    set(IMGUI_BACKEND_D3D11 OFF)
    set(IMGUI_BACKEND_DAWN ON)
    set(IMGUI_BACKEND_GLFW ON)
    set(IMGUI_BACKEND_OPENGL3 OFF)
    set(IMGUI_BACKEND_METAL OFF)
    set(IMGUI_BACKEND_OSX OFF)
    set(IMGUI_BACKEND_SOKOL OFF)
    set(IMGUI_BACKEND_WGPU ON)
    set(IMGUI_BACKEND_WINAPI ON) 
elseif("${LABAPP_BACKEND}" STREQUAL "win-gl-glfw")
    set(IMGUI_BACKEND_D3D11 OFF)
    set(IMGUI_BACKEND_DAWN OFF)
    set(IMGUI_BACKEND_GLFW ON)
    set(IMGUI_BACKEND_OPENGL3 ON)
    set(IMGUI_BACKEND_METAL OFF)
    set(IMGUI_BACKEND_OSX OFF)
    set(IMGUI_BACKEND_SOKOL OFF)
    set(IMGUI_BACKEND_WGPU OFF)
    set(IMGUI_BACKEND_WINAPI OFF) 
elseif("${LABAPP_BACKEND}" STREQUAL "win-gl-sokol")
    set(IMGUI_BACKEND_D3D11 OFF)
    set(IMGUI_BACKEND_DAWN OFF)
    set(IMGUI_BACKEND_GLFW OFF)
    set(IMGUI_BACKEND_OPENGL3 ON)
    set(IMGUI_BACKEND_METAL OFF)
    set(IMGUI_BACKEND_OSX OFF)
    set(IMGUI_BACKEND_SOKOL ON)
    set(IMGUI_BACKEND_WGPU OFF)
    set(IMGUI_BACKEND_WINAPI OFF) 
elseif("${LABAPP_BACKEND}" STREQUAL "win-d3d11-sokol")
    set(IMGUI_BACKEND_D3D11 ON)
    set(IMGUI_BACKEND_DAWN OFF)
    set(IMGUI_BACKEND_GLFW OFF)
    set(IMGUI_BACKEND_OPENGL3 OFF)
    set(IMGUI_BACKEND_METAL OFF)
    set(IMGUI_BACKEND_OSX OFF)
    set(IMGUI_BACKEND_SOKOL ON)
    set(IMGUI_BACKEND_WGPU OFF)
    set(IMGUI_BACKEND_WINAPI OFF) 
elseif("${LABAPP_BACKEND}" STREQUAL "mac-metal-cocoa")
    set(IMGUI_BACKEND_D3D11 OFF)
    set(IMGUI_BACKEND_DAWN OFF)
    set(IMGUI_BACKEND_GLFW OFF)
    set(IMGUI_BACKEND_OPENGL3 OFF)
    set(IMGUI_BACKEND_METAL ON)
    set(IMGUI_BACKEND_OSX ON)
    set(IMGUI_BACKEND_SOKOL OFF)
    set(IMGUI_BACKEND_WGPU OFF)
    set(IMGUI_BACKEND_WINAPI OFF) 
elseif("${LABAPP_BACKEND}" STREQUAL "mac-wgpu-glfw")
    set(IMGUI_BACKEND_D3D11 OFF)
    set(IMGUI_BACKEND_DAWN ON)
    set(IMGUI_BACKEND_GLFW ON)
    set(IMGUI_BACKEND_OPENGL3 OFF)
    set(IMGUI_BACKEND_METAL OFF)
    set(IMGUI_BACKEND_OSX ON)
    set(IMGUI_BACKEND_SOKOL OFF)
    set(IMGUI_BACKEND_WGPU ON)
    set(IMGUI_BACKEND_WINAPI OFF) 
elseif("${LABAPP_BACKEND}" STREQUAL "mac-metal-sokol")
    set(IMGUI_BACKEND_D3D11 OFF)
    set(IMGUI_BACKEND_DAWN OFF)
    set(IMGUI_BACKEND_GLFW OFF)
    set(IMGUI_BACKEND_OPENGL3 OFF)
    set(IMGUI_BACKEND_METAL ON)
    set(IMGUI_BACKEND_OSX OFF)
    set(IMGUI_BACKEND_SOKOL ON)
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

if (BUILD_USD)
    include(usd.cmake)
endif()

include(imgui.cmake)
include(ImPlot.cmake)
#include(LabFont.cmake)
include(cute_headers.cmake)
include(rapidjson.cmake)

if (IMGUI_BACKEND_SOKOL)
    include(sokol_gp.cmake)
    include(sokol.cmake)
    set(BACKEND_LIB sokol::sokol)
    set(PLATFORM_SRC
        LabImGui-cute_headers.c
        LabImGui-cute_headers.cpp
        LabImGui-sokol.cpp)
endif()

if (IMGUI_BACKEND_OPENGL3 AND IMGUI_BACKEND_GLFW)
    set(PLATFORM_SRC
        LabImGui-gl-glfw.cpp
        GL/gl3w.c)
    set(BACKEND_LIB glfw)
endif()

if (IMGUI_BACKEND_METAL AND IMGUI_BACKEND_OSX)
    set(PLATFORM_SRC
        LabImGui-cute_headers.c
        LabImGui-cute_headers.cpp
        LabImGui-metal-cocoa.mm)
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

target_link_libraries(LabImgui 
    Dear::dearImgui 
    Dear::implot
    cute::cuteheaders
    ${BACKEND_LIB})

target_include_directories(LabImgui 
    PUBLIC 
        ${LABIMGUI_ROOT} 
        ${imgui_SOURCE_DIR} 
        ${glfw_SOURCE_DIR})

install(TARGETS LabImgui
    ARCHIVE   DESTINATION lib
    LIBRARY   DESTINATION lib
    FRAMEWORK DESTINATION lib
    BUNDLE    DESTINATION bin
    RUNTIME   DESTINATION bin)

install(FILES ${LABIMGUI_PUBLIC_H} DESTINATION include/LabImGui)

if (APPLE)
    set_property(TARGET LabImgui APPEND_STRING PROPERTY COMPILE_FLAGS "-fobjc-arc")
endif()

add_library(Lab::Imgui ALIAS LabImgui)

#------------------ LabImguiHello
#
add_executable(LabImguiHello 
    LabImguiHello.cpp
    LabDirectories.h
    LabDirectories.cpp)

set_property(TARGET LabImguiHello PROPERTY CXX_STANDARD 17)

target_link_libraries(LabImguiHello 
    Lab::Imgui 
    #Lab::labfont
    ${BACKEND_LIB} 
    ${LABIMGUI_RENDER_SYSTEM})

install(TARGETS LabImguiHello
    BUNDLE DESTINATION bin
    RUNTIME DESTINATION bin)

install(FILES
    "resources/robots1.png"
    "resources/robots1.ase"
    "resources/invaders.ase"
    DESTINATION share/lab_font_demo)



function(pxr_whole libs result)
    set(final "")
    foreach(lib ${libs})
        if(MSVC)
            list(APPEND final -WHOLEARCHIVE:$<TARGET_FILE:${lib}>)
            list(APPEND final ${lib})
        elseif(CMAKE_COMPILER_IS_GNUCXX)
            list(APPEND final -Wl,--whole-archive ${lib} -Wl,--no-whole-archive)
        elseif("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
            list(APPEND final -Wl,-force_load ${CMAKE_BINARY_DIR}/lib${lib}.a)
        else()
            # Unknown platform.
            list(APPEND final ${lib})
        endif()
    endforeach()
    set(${result} ${final} PARENT_SCOPE)
endfunction()


set(usd_libs
    arch gf js plug tf trace vt ar work kind ndr pcp
    sdf sdr usd usdGeom usdHydra usdLux usdMedia usdPhysics
    usdRi usdShade usdSkel usdUI usdUtils usdVol)
set(usd_libs_resolved "")
pxr_whole("${usd_libs}" usd_libs_resolved)

message(STATUS "${usd_libs_resolved}")

add_executable(LabUsd
    LabUsd.cpp
    LabDirectories.h
    LabDirectories.cpp)

set_property(TARGET LabUsd PROPERTY CXX_STANDARD 17)

target_include_directories(LabUsd SYSTEM PUBLIC 
    ${PXR_INCLUDE_DIRS}
    ${CMAKE_BINARY_DIR}/include)

target_link_libraries(LabUsd
    libtbb.dylib
    Lab::Imgui 
    #Lab::labfont 
    ${BACKEND_LIB} ${LABIMGUI_RENDER_SYSTEM}
    ${PXR_LIBRARIES})
    #${usd_libs_resolved})

# Remove warnings coming from usd and enable default multithreaded compilation on windows
target_compile_options(LabUsd PRIVATE
$<$<CXX_COMPILER_ID:MSVC>:/MP /wd4244 /wd4305 /wd4996>
$<$<CXX_COMPILER_ID:GNU>:-Wno-deprecated>)

install(TARGETS LabUsd
    BUNDLE DESTINATION bin
    RUNTIME DESTINATION bin)



