
#-----------------------------------------------------------------------
# rapidJSON
#-----------------------------------------------------------------------

find_package(rapidjson QUIET)

if (TARGET RapidJSON::rapidjson)
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
            $<BUILD_INTERFACE:"${rapidjson_SOURCE_DIR}/include">)

    file(GLOB rapidjson_headers "${rapidjson_SOURCE_DIR}/include/rapidjson/*.h")
    file(GLOB rapidjson_error_headers "${rapidjson_SOURCE_DIR}/include/rapidjson/error/*.h")
    file(GLOB rapidjson_internal_headers "${rapidjson_SOURCE_DIR}/include/rapidjson/internal/*.h")
    file(GLOB rapidjson_msint_headers "${rapidjson_SOURCE_DIR}/include/rapidjson/msinttypes/*.h")
    add_library(RapidJSON::rapidjson ALIAS rapidjson)

    install(
        TARGETS rapidjson
        EXPORT rapidjsonConfig
        PUBLIC_HEADER DESTINATION include/rapidjson)

    install(FILES 
        ${rapidjson_headers} 
        DESTINATION include/rapidjson)

    install(FILES 
        ${rapidjson_error_headers} 
        DESTINATION include/rapidjson/error)

     install(FILES 
        ${rapidjson_internal_headers} 
        DESTINATION include/rapidjson/internal)

   install(FILES 
        ${rapidjson_msint_headers} 
        DESTINATION include/rapidjson/msinttypes)

    install(EXPORT rapidjsonConfig
        DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/rapidjson"
        NAMESPACE RapidJSON:: )

    endif()
endif()

