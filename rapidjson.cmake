
#-----------------------------------------------------------------------
# rapidJSON
#-----------------------------------------------------------------------
include(ExternalProject)
find_package(rapidjson QUIET)

if (TARGET rapidjson)
    message(STATUS "Found RapidJSON")
else()
    message(STATUS "Installing RapidJSON")
    ExternalProject_Add(
        rapidjson
        PREFIX "vendor/rapidjson"
        GIT_REPOSITORY "https://github.com/Tencent/rapidjson.git"
        #GIT_TAG f54b0e47a08782a6131cc3d60f94d038fa6e0a51
        GIT_SHALLOW ON
        TIMEOUT 10
        CMAKE_ARGS
            -DRAPIDJSON_BUILD_TESTS=OFF
            -DRAPIDJSON_BUILD_DOC=OFF
            -DRAPIDJSON_BUILD_EXAMPLES=OFF
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
        INSTALL_COMMAND ""
        UPDATE_COMMAND ""
    )
    #add_library(RapidJSON::rapidjson ALIAS rapidjson)
    #ExternalProject_Get_Property(rapidjson source_dir)
    #set(RAPIDJSON_INCLUDE_DIR ${source_dir}/include)
endif()

