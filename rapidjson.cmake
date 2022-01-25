
#-------------------------------------------------------------------------------
# rapidJSON
#-------------------------------------------------------------------------------
if (TARGET RapidJSON::RapidJSON)
    message(STATUS "Found RapidJSON")
else()
    message(STATUS "Installing RapidJSON")

    include(FetchContent)
    FetchContent_Declare(rapidson
        GIT_REPOSITORY "https://github.com/TenCent/rapidjson.git"
        GIT_SHALLOW ON)

    FetchContent_GetProperties(rapidson)
    if (NOT rapidson_POPULATED)
        FetchContent_Populate(rapidson)

        add_library(rapidson INTERFACE ${IMPLOT_SRC} ${IMPLOT_HEADERS})
        target_include_directories(rapidson INTERFACE
            ${rapidson_SOURCE_DIR}/include)

    add_library(RapidJSON::RapidJSON ALIAS rapidson)
    endif()
endif()

