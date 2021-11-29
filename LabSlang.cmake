# this target guard exists so that if LabSlang was previously built by another
# project in the hierarchy it won't be redundantly built
if (TARGET Lab::Slang)
    message(STATUS "Found LabSlang")
else()
    message(STATUS "Installing LabSlang")

    include(FetchContent)
    FetchContent_Declare(labslang
        GIT_REPOSITORY "https://github.com/meshula/LabSlang.git"
        GIT_TAG "main"
        GIT_SHALLOW ON)

    FetchContent_GetProperties(labslang)
    if(NOT labslang_POPULATED)
        FetchContent_Populate(labslang)
        set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "shared")
	message(STATUS "labslang src dir ${labslang_SOURCE_DIR}")
	message(STATUS "labslang bin dir ${labslang_BINARY_DIR}")
	add_subdirectory(${labslang_SOURCE_DIR} ${labslang_BINARY_DIR})
    endif()
endif()

