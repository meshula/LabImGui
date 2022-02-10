
#-------------------------------------------------------------------------------
# ImPlot
#-------------------------------------------------------------------------------

find_package(ImPlot QUIET)

if (TARGET Dear::implot)
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
            PUBLIC $<BUILD_INTERFACE:${implot_SOURCE_DIR}>
            PUBLIC $<INSTALL_INTERFACE:include/ImPlot>)

        set_property(TARGET implot PROPERTY CXX_STANDARD 17)

        target_link_libraries(implot PUBLIC Dear::dearImgui)
        add_library(Dear::implot ALIAS implot)

        install(
            TARGETS implot
            EXPORT ImPlotConfig
            LIBRARY DESTINATION lib
            ARCHIVE DESTINATION lib
            RUNTIME DESTINATION bin
            PUBLIC_HEADER DESTINATION include/implot
        )
    install(FILES ${IMPLOT_HEADERS} 
            DESTINATION "${CMAKE_INSTALL_PREFIX}/include/ImPlot")

        install(EXPORT ImPlotConfig
            DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/ImPlot"
            NAMESPACE Dear:: )

    endif()
endif()
