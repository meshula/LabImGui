
# adapted from https://github.com/ocornut/imgui/pull/4614 by yhsb2k

# check for the target because if another project in the hierarchy already
# built it, it shouldn't be build and added again.

if (TARGET Dear::Imgui)
    message(STATUS "Found Dear Imgui")
else()
    message(STATUS "Installing Dear Imgui")

    include(FetchContent)
    FetchContent_Declare(imgui
        GIT_REPOSITORY "https://github.com/ocornut/imgui.git"
        GIT_TAG "docking"
        GIT_SHALLOW ON)

    FetchContent_GetProperties(imgui)
    if (NOT imgui_POPULATED)
        FetchContent_Populate(imgui)

        option(IMGUI_BACKEND_ALLEGRO5 "Allegro Gaming Library")
        option(IMGUI_BACKEND_ANDROID "Android native API")
        option(IMGUI_BACKEND_DX9 "DirectX9")
        option(IMGUI_BACKEND_DX10 "DirectX10")
        option(IMGUI_BACKEND_DX11 "DirectX11")
        option(IMGUI_BACKEND_DX12 "DirectX12")
        option(IMGUI_BACKEND_GLFW "GLFW (Windows, macOS, Linux, etc.)")
        option(IMGUI_BACKEND_GLUT "GLUT/FreeGLUT (legacy, not recommended today)")
        option(IMGUI_BACKEND_MARMALADE "Marmalade SDK")
        option(IMGUI_BACKEND_METAL "Metal (with ObjC)")
        option(IMGUI_BACKEND_OPENGL2 "OpenGL 2 (legacy, not recommended today)")
        option(IMGUI_BACKEND_OPENGL3 "OpenGL 3/4")
        option(IMGUI_BACKEND_OSX "macOS native API")
        option(IMGUI_BACKEND_SDL "SDL2 (Windows, macOS, Linux, iOS, Android)")
        option(IMGUI_BACKEND_SDL_RENDERER "SDL2 renderer")
        option(IMGUI_BACKEND_VULKAN "Vulkan")
        option(IMGUI_BACKEND_WGPU "WebGPU")
        option(IMGUI_BACKEND_DAWN "Dawn")
        option(IMGUI_BACKEND_WINAPI "Win32 native API")

        add_library(imgui
            ${imgui_SOURCE_DIR}/imgui.cpp
            ${imgui_SOURCE_DIR}/imgui_demo.cpp
            ${imgui_SOURCE_DIR}/imgui_draw.cpp
            ${imgui_SOURCE_DIR}/imgui_tables.cpp
            ${imgui_SOURCE_DIR}/imgui_widgets.cpp
            $<$<BOOL:${IMGUI_BACKEND_ALLEGRO5}>:${imgui_SOURCE_DIR}/backends/imgui_impl_allegro5.cpp>
            $<$<BOOL:${IMGUI_BACKEND_ANDROID}>:${imgui_SOURCE_DIR}/backends/imgui_impl_android.cpp>
            $<$<BOOL:${IMGUI_BACKEND_DX9}>:${imgui_SOURCE_DIR}/backends/imgui_impl_dx9.cpp>
            $<$<BOOL:${IMGUI_BACKEND_DX10}>:${imgui_SOURCE_DIR}/backends/imgui_impl_dx10.cpp>
            $<$<BOOL:${IMGUI_BACKEND_DX11}>:${imgui_SOURCE_DIR}/backends/imgui_impl_dx11.cpp>
            $<$<BOOL:${IMGUI_BACKEND_DX12}>:${imgui_SOURCE_DIR}/backends/imgui_impl_dx12.cpp>
            $<$<BOOL:${IMGUI_BACKEND_GLFW}>:${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp>
            $<$<BOOL:${IMGUI_BACKEND_GLUT}>:${imgui_SOURCE_DIR}/backends/imgui_impl_glut.cpp>
            $<$<BOOL:${IMGUI_BACKEND_MARMALADE}>:${imgui_SOURCE_DIR}/backends/imgui_impl_marmalade.cpp>
            $<$<BOOL:${IMGUI_BACKEND_METAL}>:${imgui_SOURCE_DIR}/backends/imgui_impl_metal.mm>
            $<$<BOOL:${IMGUI_BACKEND_OPENGL2}>:${imgui_SOURCE_DIR}/backends/imgui_impl_opengl2.cpp>
            $<$<BOOL:${IMGUI_BACKEND_OPENGL3}>:${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp>
            $<$<BOOL:${IMGUI_BACKEND_OSX}>:${imgui_SOURCE_DIR}/backends/imgui_impl_osx.mm>
            $<$<BOOL:${IMGUI_BACKEND_SDL}>:${imgui_SOURCE_DIR}/backends/imgui_impl_sdl.cpp>
            $<$<BOOL:${IMGUI_BACKEND_SDL_RENDERER}>:${imgui_SOURCE_DIR}/backends/imgui_impl_sdlrenderer.cpp>
            $<$<BOOL:${IMGUI_BACKEND_VULKAN}>:${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp>
            $<$<BOOL:${IMGUI_BACKEND_WGPU}>:${imgui_SOURCE_DIR}/backends/imgui_impl_wgpu.cpp>
            $<$<BOOL:${IMGUI_BACKEND_WINAPI}>:${imgui_SOURCE_DIR}/backends/imgui_impl_win32.cpp>
        )

        set_property(TARGET imgui PROPERTY CXX_STANDARD 11)

        target_include_directories(imgui
            PUBLIC
                ${imgui_SOURCE_DIR}
                ${imgui_SOURCE_DIR}/backends
            PRIVATE 
                "${LABSLANG_DAWN_INSTALL_ROOT}/include"
                "${WEBGPU_HEADER_LOCATION}"
        )

        if (APPLE)
            if(CMAKE_OSX_SYSROOT MATCHES ".*iphoneos.*")
                target_link_libraries(imgui PUBLIC
                    "-framework Metal"
                    "-framework MetalKit")
            else()
                target_link_libraries(imgui PUBLIC
                    "-framework Metal"
                    "-framework MetalKit"
                    "-framework QuartzCore"
                    "-framework Cocoa")
            endif()
            set_property(TARGET imgui APPEND_STRING PROPERTY COMPILE_FLAGS "-fobjc-arc")
        endif()

        if (IMGUI_BACKEND_DAWN)
            add_dependencies(imgui webgpu_header)
        endif()

        target_link_libraries(imgui
            PRIVATE
                $<$<BOOL:${IMGUI_BACKEND_ANDROID}>:android>
                $<$<BOOL:${IMGUI_BACKED_DAWN}>:dawn_native>
                $<$<BOOL:${IMGUI_BACKEND_DX9}>:d3d9.lib>
                $<$<BOOL:${IMGUI_BACKEND_DX10}>:d3d10.lib>
                $<$<BOOL:${IMGUI_BACKEND_DX11}>:d3d11.lib>
                $<$<BOOL:${IMGUI_BACKEND_DX12}>:d3d12.lib>
                $<$<BOOL:${IMGUI_BACKEND_DX10}>:d3dcompiler.lib>
                $<$<BOOL:${IMGUI_BACKEND_DX11}>:d3dcompiler.lib>
                $<$<BOOL:${IMGUI_BACKEND_DX12}>:d3dcompiler.lib>
                $<$<BOOL:${IMGUI_BACKEND_DX12}>:dxgi.lib>
                $<$<BOOL:${IMGUI_BACKEND_GLFW}>:glfw>
                $<$<BOOL:${IMGUI_BACKEND_GLUT}>:glut>
                $<$<OR:$<BOOL:${IMGUI_BACKEND_SDL}>,$<BOOL:${IMGUI_BACKEND_SDL_RENDERER}>>:SDL2::SDL2main>
                $<$<OR:$<BOOL:${IMGUI_BACKEND_SDL}>,$<BOOL:${IMGUI_BACKEND_SDL_RENDERER}>>:$<IF:$<BOOL:${SDL_STATIC_ENABLED_BY_DEFAULT}>,SDL2::SDL2-static,SDL2::SDL2>>
                $<$<BOOL:${IMGUI_BACKEND_VULKAN}>:Vulkan::Vulkan>
                $<$<AND:$<BOOL:${IMGUI_BACKEND_OPENGL3}>,$<BOOL:${OpenGL_FOUND}>>:OpenGL::GL>
                $<$<AND:$<BOOL:${IMGUI_BACKEND_OPENGL2}>,$<BOOL:${OpenGL_FOUND}>>:OpenGL::GL>
        )

        add_library(Dear::Imgui ALIAS imgui)
    endif()
endif()



