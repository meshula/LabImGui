
#-------------------------------------------------------------------------------
# sokol
#-------------------------------------------------------------------------------
if (TARGET Lab::Font)
    message(STATUS "Found LabFont")
else()
    include(FetchContent)
    FetchContent_Declare(labfont
        GIT_REPOSITORY "https://github.com/meshula/LabFont.git"
        GIT_TAG "main"
        GIT_SHALLOW ON)

    FetchContent_GetProperties(labfont)
    if (NOT labfont_POPULATED)
        message(STATUS "Installing LabFont")
        FetchContent_Populate(labfont)

        configure_file("${labfont_SOURCE_DIR}/LabFontDemo.config.h"
                       "${labfont_SOURCE_DIR}/LabFontDemo.h" @ONLY)


        set(LABFONT_SRC
            ${labfont_SOURCE_DIR}/src/LabFont.cpp
            ${labfont_SOURCE_DIR}/src/LabFontDemo.cpp
            ${labfont_SOURCE_DIR}/src/lab_floooh_8bitfont.cpp
            ${labfont_SOURCE_DIR}/src/quadplay_font.cpp
            )

        set(LABFONT_HEADERS 
            ${labfont_SOURCE_DIR}/LabFont.h
            ${labfont_SOURCE_DIR}/LabFontDemo.h
        )
        add_library(labfont STATIC ${LABFONT_SRC} ${LABFONT_HEADERS})
        target_include_directories(labfont SYSTEM 
            PUBLIC ${labfont_SOURCE_DIR})

        target_link_libraries(labfont PUBLIC RapidJSON::RapidJSON sokol::sokol)

        install(
            TARGETS labfont
            LIBRARY DESTINATION lib
            ARCHIVE DESTINATION lib
            RUNTIME DESTINATION bin
            PUBLIC_HEADER DESTINATION include/labfont
            )
        
        install(FILES
            "${labfont_SOURCE_DIR}/resources/DroidSansJapanese.ttf"
            "${labfont_SOURCE_DIR}/resources/DroidSerif-Bold.ttf"
            "${labfont_SOURCE_DIR}/resources/DroidSerif-Italic.ttf"
            "${labfont_SOURCE_DIR}/resources/DroidSerif-Regular.ttf"
            "${labfont_SOURCE_DIR}/resources/robot-18.png"
            "${labfont_SOURCE_DIR}/resources/robot-18.font.json"
            "${labfont_SOURCE_DIR}/resources/hauer-12.png"
            "${labfont_SOURCE_DIR}/resources/hauer-12.font.json"
            "${labfont_SOURCE_DIR}/resources/Cousine-Regular.ttf"
            DESTINATION share/lab_font_demo)
        add_library(Lab::Font ALIAS labfont)
    endif()
endif()

