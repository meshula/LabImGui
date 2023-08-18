
# adapted from https://github.com/ocornut/imgui/pull/4614 by yhsb2k

# check for the target because if another project in the hierarchy already
# built it, it shouldn't be build and added again.

find_package(dearImgui QUIET)

if (TARGET Dear::dearImgui)
    message(STATUS "Found Dear Imgui")
else()
    message(STATUS "Installing Dear Imgui")

    include(FetchContent)
    FetchContent_Declare(dearImgui
        GIT_REPOSITORY "https://github.com/ocornut/imgui.git"
        GIT_TAG "docking"
        GIT_SHALLOW ON)

    FetchContent_GetProperties(dearImgui)
    if (NOT dearimgui_POPULATED)
        FetchContent_Populate(dearImgui)

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

        set(dearimgui_PUBLIC_HEADERS
            ${dearimgui_SOURCE_DIR}/imconfig.h
            ${dearimgui_SOURCE_DIR}/imgui.h
            ${dearimgui_SOURCE_DIR}/imgui_internal.h
            ${dearimgui_SOURCE_DIR}/imstb_rectpack.h
            ${dearimgui_SOURCE_DIR}/imstb_textedit.h
            ${dearimgui_SOURCE_DIR}/imstb_truetype.h)

        add_library(dearImgui
            ${dearimgui_PUBLIC_HEADERS}
            ${dearimgui_SOURCE_DIR}/imgui.cpp
            ${dearimgui_SOURCE_DIR}/imgui_demo.cpp
            ${dearimgui_SOURCE_DIR}/imgui_draw.cpp
            ${dearimgui_SOURCE_DIR}/imgui_tables.cpp
            ${dearimgui_SOURCE_DIR}/imgui_widgets.cpp
            $<$<BOOL:${IMGUI_BACKEND_ALLEGRO5}>:${dearimgui_SOURCE_DIR}/backends/imgui_impl_allegro5.cpp>
            $<$<BOOL:${IMGUI_BACKEND_ANDROID}>:${dearimgui_SOURCE_DIR}/backends/imgui_impl_android.cpp>
            $<$<BOOL:${IMGUI_BACKEND_DX9}>:${dearimgui_SOURCE_DIR}/backends/imgui_impl_dx9.cpp>
            $<$<BOOL:${IMGUI_BACKEND_DX10}>:${dearimgui_SOURCE_DIR}/backends/imgui_impl_dx10.cpp>
            $<$<BOOL:${IMGUI_BACKEND_DX11}>:${dearimgui_SOURCE_DIR}/backends/imgui_impl_dx11.cpp>
            $<$<BOOL:${IMGUI_BACKEND_DX12}>:${dearimgui_SOURCE_DIR}/backends/imgui_impl_dx12.cpp>
            $<$<BOOL:${IMGUI_BACKEND_GLFW}>:${dearimgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp>
            $<$<BOOL:${IMGUI_BACKEND_GLUT}>:${dearimgui_SOURCE_DIR}/backends/imgui_impl_glut.cpp>
            $<$<BOOL:${IMGUI_BACKEND_MARMALADE}>:${dearimgui_SOURCE_DIR}/backends/imgui_impl_marmalade.cpp>
            $<$<BOOL:${IMGUI_BACKEND_METAL}>:${dearimgui_SOURCE_DIR}/backends/imgui_impl_metal.mm>
            $<$<BOOL:${IMGUI_BACKEND_OPENGL2}>:${dearimgui_SOURCE_DIR}/backends/imgui_impl_opengl2.cpp>
            $<$<BOOL:${IMGUI_BACKEND_OPENGL3}>:${dearimgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp>
            $<$<BOOL:${IMGUI_BACKEND_OSX}>:${dearimgui_SOURCE_DIR}/backends/imgui_impl_osx.mm>
            $<$<BOOL:${IMGUI_BACKEND_SDL}>:${dearimgui_SOURCE_DIR}/backends/imgui_impl_sdl.cpp>
            $<$<BOOL:${IMGUI_BACKEND_SDL_RENDERER}>:${dearimgui_SOURCE_DIR}/backends/imgui_impl_sdlrenderer.cpp>
            $<$<BOOL:${IMGUI_BACKEND_VULKAN}>:${dearimgui_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp>
            $<$<BOOL:${IMGUI_BACKEND_WGPU}>:${dearimgui_SOURCE_DIR}/backends/imgui_impl_wgpu.cpp>
            $<$<BOOL:${IMGUI_BACKEND_WINAPI}>:${dearimgui_SOURCE_DIR}/backends/imgui_impl_win32.cpp>
        )

        set_property(TARGET dearImgui PROPERTY CXX_STANDARD 17)
        message(STATUS "IMGUI_BACKEND_WINAPI: ${IMGUI_BACKEND_WINAPI}")

        target_include_directories(dearImgui
            PUBLIC
                # BUILD_INTERFACE distinguishes from an INSTALL_INTERFACE
                $<BUILD_INTERFACE:${dearimgui_SOURCE_DIR}>
                $<BUILD_INTERFACE:${dearimgui_SOURCE_DIR}/backends>
            	$<BUILD_INTERFACE:$<$<BOOL:${IMGUI_BACKEND_WGPU}>:${CMAKE_INSTALL_PREFIX}/include>>
            PRIVATE 
 	            #$<$<BOOL:${IMGUI_BACKEND_DAWN}>:"${LABSLANG_DAWN_INSTALL_ROOT}/include">
		        #$<$<BOOL:${IMGUI_BACKEND_WGPU}>:"${WEBGPU_HEADER_LOCATION}">
            )

        #target_compile_definitions(dearImgui PUBLIC
        #    ImDrawIdx=ImU32
        #)

        if (APPLE)
            if(CMAKE_OSX_SYSROOT MATCHES ".*iphoneos.*")
                target_link_libraries(dearImgui PUBLIC
                    "-framework Metal"
                    "-framework MetalKit")
            else()
                target_link_libraries(dearImgui PUBLIC
                    "-framework Metal"
                    "-framework MetalKit"
                    "-framework QuartzCore"
                    "-framework Cocoa")
            endif()
            set_property(TARGET dearImgui APPEND_STRING PROPERTY COMPILE_FLAGS "-fobjc-arc")
        endif()

        target_link_libraries(dearImgui
            PUBLIC
                $<$<BOOL:${IMGUI_BACKEND_GLFW}>:glfw>
                $<$<BOOL:${IMGUI_BACKEND_ANDROID}>:android>
                $<$<BOOL:${IMGUI_BACKEND_DAWN}>:dawncpp>
                $<$<BOOL:${IMGUI_BACKEND_DAWN}>:dawn_proc>
                $<$<BOOL:${IMGUI_BACKEND_DAWN}>:dawn_common>
                $<$<BOOL:${IMGUI_BACKEND_DAWN}>:dawn_native>
                $<$<BOOL:${IMGUI_BACKEND_DAWN}>:dawn_wire>
                $<$<BOOL:${IMGUI_BACKEND_DAWN}>:dawn_utils>
                $<$<BOOL:${IMGUI_BACKEND_DX9}>:d3d9.lib>
                $<$<BOOL:${IMGUI_BACKEND_DX10}>:d3d10.lib>
                $<$<BOOL:${IMGUI_BACKEND_DX11}>:d3d11.lib>
                $<$<BOOL:${IMGUI_BACKEND_DX12}>:d3d12.lib>
                $<$<BOOL:${IMGUI_BACKEND_DX10}>:d3dcompiler.lib>
                $<$<BOOL:${IMGUI_BACKEND_DX11}>:d3dcompiler.lib>
                $<$<BOOL:${IMGUI_BACKEND_DX12}>:d3dcompiler.lib>
                $<$<BOOL:${IMGUI_BACKEND_DX12}>:dxgi.lib>
                $<$<BOOL:${IMGUI_BACKEND_GLUT}>:glut>
                $<$<OR:$<BOOL:${IMGUI_BACKEND_SDL}>,$<BOOL:${IMGUI_BACKEND_SDL_RENDERER}>>:SDL2::SDL2main>
                $<$<OR:$<BOOL:${IMGUI_BACKEND_SDL}>,$<BOOL:${IMGUI_BACKEND_SDL_RENDERER}>>:$<IF:$<BOOL:${SDL_STATIC_ENABLED_BY_DEFAULT}>,SDL2::SDL2-static,SDL2::SDL2>>
                $<$<BOOL:${IMGUI_BACKEND_VULKAN}>:Vulkan::Vulkan>
                $<$<AND:$<BOOL:${IMGUI_BACKEND_OPENGL3}>,$<BOOL:${OpenGL_FOUND}>>:OpenGL::GL>
                $<$<AND:$<BOOL:${IMGUI_BACKEND_OPENGL2}>,$<BOOL:${OpenGL_FOUND}>>:OpenGL::GL>
        )

        add_library(Dear::dearImgui ALIAS dearImgui)

        install(FILES ${dearimgui_PUBLIC_HEADERS}
             DESTINATION "${CMAKE_INSTALL_PREFIX}/include")
        install(FILES "${dearimgui_SOURCE_DIR}/LICENSE.txt"
             DESTINATION "${CMAKE_INSTALL_PREFIX}/share/dearImgui")

        install(TARGETS dearImgui
            EXPORT dearImguiConfig
            INCLUDES DESTINATION "${CMAKE_INSTALL_PREFIX}/include"
            ARCHIVE DESTINATION  "${CMAKE_INSTALL_PREFIX}/lib"
            LIBRARY DESTINATION  "${CMAKE_INSTALL_PREFIX}/lib"
            RUNTIME DESTINATION  "${CMAKE_INSTALL_PREFIX}/bin")

        install(EXPORT dearImguiConfig
            DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/dearImgui"
            NAMESPACE Dear:: )

    endif()
endif()

