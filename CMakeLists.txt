cmake_minimum_required(VERSION 3.19)
project(LabImgui)
set(LABIMGUI_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

cmake_policy(SET CMP0077 NEW)
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW) # make submodules respect CMP0077

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(BUILD_USD OFF)

if (APPLE)
    #set(LABAPP_BACKEND "mac-gl-foundation" CACHE STRING "Backend")
    set(LABAPP_BACKEND "mac-metal-foundation" CACHE STRING "Backend")
    #set(LABAPP_BACKEND "mac-wgpu-glfw" CACHE STRING "Backend")
    #set(LABAPP_BACKEND "mac-metal-sokol" CACHE STRING "Backend")
elseif (WIN32)
    #set(LABAPP_BACKEND "win-wgpu-glfw" CACHE STRING "Backend")
    #set(LABAPP_BACKEND "win-gl-glfw" CACHE STRING "Backend")
    set(LABAPP_BACKEND "win-d3d11-sokol" CACHE STRING "Backend")
    #set(LABAPP_BACKEND "win-gl-sokol" CACHE STRING "Backend")
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
elseif("${LABAPP_BACKEND}" STREQUAL "mac-metal-foundation")
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
elseif("${LABAPP_BACKEND}" STREQUAL "mac-gl-foundation")
    set(IMGUI_BACKEND_D3D11 OFF)
    set(IMGUI_BACKEND_DAWN OFF)
    set(IMGUI_BACKEND_GLFW OFF)
    set(IMGUI_BACKEND_OPENGL3 ON)
    set(IMGUI_BACKEND_METAL OFF)
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
    message(STATUS "GLFW backend enabled")
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
include(LabFont.cmake)
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

if (IMGUI_BACKEND_OPENGL3 AND IMGUI_BACKEND_OSX)
    set(PLATFORM_SRC
        LabImGui-gl-cocoa.mm)
endif()

if (IMGUI_BACKEND_WGPU)
    set(PLATFORM_SRC
        LabApp-wgpu-glfw.cpp)
    set(LABIMGUI_RENDER_SYSTEM
        dawncpp dawn_proc dawn_common dawn_native
        dawn_wire dawn_utils)
endif()

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

add_executable(LabImguiHello 
    LabDirectories.h
    LabDirectories.cpp
    LabImguiHello.${MAIN_EXT})

target_link_libraries(LabImguiHello Dear::dearImgui Lab::Imgui 
    ${BACKEND_LIB} ${LABIMGUI_RENDER_SYSTEM}
    ${APPLE_FRAMEWORKS})

set_property(TARGET LabImguiHello PROPERTY CXX_STANDARD 17)

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

if (BUILD_USD)

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

    target_include_directories(LabUsd SYSTEM PUBLIC ${CMAKE_BINARY_DIR}/include)

    target_link_libraries(LabUsd
        ${CMAKE_BINARY_DIR}/libtbb.dylib
        Lab::Imgui Lab::labfont ${BACKEND_LIB} ${LABIMGUI_RENDER_SYSTEM}
        ${usd_libs_resolved})

    install(TARGETS LabUsd
        BUNDLE DESTINATION bin
        RUNTIME DESTINATION bin)

endif()

