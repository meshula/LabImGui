
#-------------------------------------------------------------------------------
# sokol
#-------------------------------------------------------------------------------
if (TARGET Dear::ImPlot)
    message(STATUS "Found ImPlot")
else()
    message(STATUS "Installing ImPlot")

    include(FetchContent)
    FetchContent_Declare(implot
        GIT_REPOSITORY "https://github.com/epezent/implot.git"
        GIT_SHALLOW ON)

    FetchContent_GetProperties(implot)
    if (NOT implot_POPULATED)
        FetchContent_Populate(implot)

        set(IMPLOT_SRC
            ${implot_SOURCE_DIR}/implot.cpp
            ${implot_SOURCE_DIR}/implot_items.cpp
            ${implot_SOURCE_DIR}/implot_demo.cpp)

        set(IMPLOT_HEADERS 
            ${implot_SOURCE_DIR}/implot.h
            ${implot_SOURCE_DIR}/implot_internal.h
        )
        add_library(implot STATIC ${IMPLOT_SRC} ${IMPLOT_HEADERS})
        target_include_directories(implot SYSTEM 
            PUBLIC ${implot_SOURCE_DIR})

        target_link_libraries(implot PUBLIC Dear::Imgui)

        install(
            TARGETS implot
            LIBRARY DESTINATION lib
            ARCHIVE DESTINATION lib
            RUNTIME DESTINATION bin
            PUBLIC_HEADER DESTINATION include/implot
        )

    add_library(Dear::ImPlot ALIAS implot)
    endif()
endif()
