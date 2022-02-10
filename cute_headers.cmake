
#-------------------------------------------------------------------------------
#i cute headers
#-------------------------------------------------------------------------------

find_package(cuteheaders QUIET)

if (TARGET cute::cuteheaders)
    message(STATUS "Found cute headers")
else()
    message(STATUS "Installing cute headers")

    include(FetchContent)
    FetchContent_Declare(cuteheaders
        GIT_REPOSITORY "https://github.com/RandyGaul/cute_headers.git"
        GIT_SHALLOW ON)

    FetchContent_GetProperties(cuteheaders)
    if (NOT cuteheaders_POPULATED)
        FetchContent_Populate(cuteheaders)

        file(GLOB cute_headers "${cuteheaders_SOURCE_DIR}/*.h")

        add_library(cuteheaders INTERFACE)
        target_include_directories(cuteheaders INTERFACE
            $<BUILD_INTERFACE:${cuteheaders_SOURCE_DIR}>
            $<INSTALL_INTERFACE:include/cute>)

        add_library(cute::cuteheaders ALIAS cuteheaders)
        
        install(TARGETS cuteheaders
            EXPORT cuteheadersConfig
            PUBLIC_HEADER DESTINATION include/cute)

        install(FILES
            ${cute_headers} DESTINATION include/cute)

        install(EXPORT cuteheadersConfig
            DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/cuteheaders"
            NAMESPACE cute:: )

    endif()
endif()

