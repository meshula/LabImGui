
#-------------------------------------------------------------------------------
# rapidJSON
#-------------------------------------------------------------------------------
if (TARGET cute::headers)
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

        add_library(cuteheaders INTERFACE)
        target_include_directories(cuteheaders INTERFACE
            ${cuteheaders_SOURCE_DIR})

    add_library(cute::headers ALIAS cuteheaders)
    endif()
endif()

