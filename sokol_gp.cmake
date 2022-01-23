
#-------------------------------------------------------------------------------
# sokol
#-------------------------------------------------------------------------------
if (TARGET sokol::sokol_gp)
    message(STATUS "Found sokol_gp")
else()
    message(STATUS "Installing sokol_gp")

    include(FetchContent)
    FetchContent_Declare(sokol_gp
        GIT_REPOSITORY "https://github.com/edubart/sokol_gp.git"
        GIT_SHALLOW ON)

    FetchContent_GetProperties(sokol_gp)
    if (NOT sokol_gp_POPULATED)
        FetchContent_Populate(sokol_gp)

        set(SOKOL_GP_SRC 
            sokol_gp.c)

        set(SOKOL_GP_HEADERS 
            ${sokol_gp_SOURCE_DIR}/sokol_gp.h
            ${sokol_gp_SOURCE_DIR}/shaders/sample-effect.glsl.h
            ${sokol_gp_SOURCE_DIR}/shaders/sample-sdf.glsl.h
            ${sokol_gp_SOURCE_DIR}/shaders/sokol_gp.glsl.h
        )
        add_library(sokol_gp STATIC ${SOKOL_GP_SRC} ${SOKOL_GP_HEADERS})
        target_include_directories(sokol_gp SYSTEM 
            PUBLIC ${sokol_gp_SOURCE_DIR})

        if (IMGUI_BACKEND_OPENGL3)
            set(PLATFORM_DEFS SOKOL_GLCORE33)
        elseif (IMGUI_BACKEND_D3D11)
            set(PLATFORM_DEFS SOKOL_D3D11)
        endif()

        #target_link_libraries(sokol imgui)
        #add_dependencies(sokol imgui)
        target_compile_definitions(sokol_gp PRIVATE
            ${ST_GFX_DEFS}
            IMGUI_DEFINE_MATH_OPERATORS
            ${PLATFORM_DEFS}
        )

        target_link_libraries(sokol_gp PUBLIC ${PLATFORM_LIBS})

        install(
            TARGETS sokol_gp
            LIBRARY DESTINATION lib
            ARCHIVE DESTINATION lib
            RUNTIME DESTINATION bin
            PUBLIC_HEADER DESTINATION include/sokol
        )

        add_library(sokol::sokol_gp ALIAS sokol_gp)
    endif()
endif()

