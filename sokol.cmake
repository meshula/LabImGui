
#-------------------------------------------------------------------------------
# sokol
#-------------------------------------------------------------------------------

find_package(sokol QUIET)

if (TARGET sokol::sokol)
    message(STATUS "Found sokol")
else()
    message(STATUS "Installing sokol")

    include(FetchContent)
    FetchContent_Declare(sokol
        GIT_REPOSITORY "https://github.com/floooh/sokol.git"
        GIT_SHALLOW ON)

    FetchContent_GetProperties(sokol)
    if (NOT sokol_POPULATED)
        FetchContent_Populate(sokol)

        set(SOKOL_SRC 
            sokol.c
            sokol.cpp)

        set(SOKOL_HEADERS 
            ${sokol_SOURCE_DIR}/sokol_app.h
            ${sokol_SOURCE_DIR}/sokol_args.h
            ${sokol_SOURCE_DIR}/sokol_audio.h
            ${sokol_SOURCE_DIR}/sokol_fetch.h
            ${sokol_SOURCE_DIR}/sokol_gfx.h
            ${sokol_SOURCE_DIR}/sokol_glue.h
            ${sokol_SOURCE_DIR}/sokol_time.h
            ${sokol_SOURCE_DIR}/util/sokol_fontstash.h
            ${sokol_SOURCE_DIR}/util/sokol_gfx_imgui.h
            ${sokol_SOURCE_DIR}/util/sokol_gl.h
            ${sokol_SOURCE_DIR}/util/sokol_imgui.h
        )
        add_library(sokol STATIC ${SOKOL_SRC} ${SOKOL_HEADERS})
        target_include_directories(sokol SYSTEM 
            PUBLIC 
            $<BUILD_INTERFACE:${sokol_SOURCE_DIR}>
            $<BUILD_INTERFACE:${sokol_SOURCE_DIR}/util>
            $<INSTALL_INTERFACE:include/sokol>)

        set_property(TARGET sokol PROPERTY CXX_STANDARD 17)
        if (IMGUI_BACKEND_OPENGL3)
            set(PLATFORM_DEFS SOKOL_GLCORE33)
        elseif (IMGUI_BACKEND_D3D11)
            set(PLATFORM_DEFS SOKOL_D3D11)
        elseif (IMGUI_BACKEND_METAL)
            set(PLATFORM_DEFS SOKOL_METAL)
            set_source_files_properties(sokol.c 
                PROPERTIES COMPILE_FLAGS "-x objective-c -fobjc-arc")
            set_source_files_properties(sokol.cpp 
                PROPERTIES COMPILE_FLAGS "-x objective-c++ -fobjc-arc")

        endif()

        #target_link_libraries(sokol imgui)
        #add_dependencies(sokol imgui)
        target_compile_definitions(sokol PRIVATE
            ${ST_GFX_DEFS}
            IMGUI_DEFINE_MATH_OPERATORS
            HAVE_IMGUI
            ${PLATFORM_DEFS}
            SOKOL_NO_ENTRY
            SOKOL_WIN32_FORCE_MAIN # enable for a console application on Windows
        )

        if (WIN32)
        #    set(PLATFORM_LIBS ws2_32 Iphlpapi.lib opengl32.lib)
        endif()

        target_link_libraries(sokol PUBLIC 
            Dear::dearImgui sokol::sokol_gp ${PLATFORM_LIBS})

        add_library(sokol::sokol ALIAS sokol)
        
        install(
            TARGETS sokol
            EXPORT sokolConfig
            LIBRARY DESTINATION lib
            ARCHIVE DESTINATION lib
            RUNTIME DESTINATION bin
            PUBLIC_HEADER DESTINATION include/sokol
        )

        install(FILES ${SOKOL_HEADERS} 
            DESTINATION "${CMAKE_INSTALL_PREFIX}/include/sokol")

        install(EXPORT sokolConfig
            DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/sokol"
            NAMESPACE sokol:: )

    endif()
endif()
