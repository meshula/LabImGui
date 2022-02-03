
#-------------------------------------------------------------------------------
# rapidJSON
#-------------------------------------------------------------------------------
if (TARGET RapidJSON::RapidJSON)
    message(STATUS "Found RapidJSON")
else()
    message(STATUS "Installing RapidJSON")

    include(FetchContent)
    FetchContent_Declare(rapidjson
        GIT_REPOSITORY "https://github.com/TenCent/rapidjson.git"
        GIT_SHALLOW ON)

    FetchContent_GetProperties(rapidjson)
    if (NOT rapidjson_POPULATED)
        FetchContent_Populate(rapidjson)

        add_library(rapidjson INTERFACE)
        target_include_directories(rapidjson INTERFACE
            ${rapidjson_SOURCE_DIR}/include)

    add_library(RapidJSON::RapidJSON ALIAS rapidjson)
    endif()
endif()

