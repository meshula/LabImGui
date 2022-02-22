#cmake_minimum_required(VERSION 3.19)
#project(Usd-mono C CXX)
#set(USD_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
#cmake_policy(SET CMP0077 NEW)
#set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
#set_property(GLOBAL PROPERTY USE_FOLDERS ON)

#-------------------------------------------------------------------------------
# USD
#-------------------------------------------------------------------------------

find_package(pxrusd)

if (TARGET pxrusd::usd)
    # assume the existance of pxrusd::usd means all the packages exist
    message(STATUS "Found usd")
else()
    message(STATUS "Installing usd")

    include(FetchContent)
    FetchContent_Declare(pxrusd
        GIT_REPOSITORY "https://github.com/PixarAnimationStudios/USD.git"
        GIT_TAG "dev"
        GIT_SHALLOW ON)

    FetchContent_GetProperties(pxrusd)
    if (NOT usd_POPULATED)
        FetchContent_Populate(pxrusd)

        set(USD_ROOT ${pxrusd_SOURCE_DIR})
        
        # config

        # Set PXR_MAJOR/MINOR/PATCH_VERSION and PXR_VERSION
        include(${USD_ROOT}/cmake/defaults/version.cmake)

        set(PXR_USE_NAMESPACES 1)
        set(PXR_EXTERNAL_NAMESPACE pxr)
        set(PXR_INTERNAL_NAMESPACE lite)
        set(PXR_PYTHON_SUPPORT_ENABLED 0)
        set(PXR_PREFER_SAFETY_OVER_SPEED 0)

        set(AR_VERSION 2)

        configure_file(${USD_ROOT}/pxr/pxr.h.in
            ${CMAKE_BINARY_DIR}/include/pxr/pxr.h     
        )  
        install(
            FILES ${CMAKE_BINARY_DIR}/include/pxr/pxr.h
            DESTINATION include/pxr
        )

        configure_file(${USD_ROOT}/pxr/usd/ar/ar.h.in
            ${CMAKE_BINARY_DIR}/include/pxr/usd/ar/ar.h     
        )  
        install(
            FILES ${CMAKE_BINARY_DIR}/include/pxr/usd/ar/ar.h
            DESTINATION include/pxr/usd/ar/ar.h
        )

        function(pxr_whole libs result)
            set(final "")
            foreach(lib ${libs})
                message(STATUS "Lib: ${lib}")
                if(MSVC)
                    list(APPEND final -WHOLEARCHIVE:$<TARGET_FILE:${lib}>)
                    list(APPEND final ${lib})
                elseif(CMAKE_COMPILER_IS_GNUCXX)
                    list(APPEND final -Wl,--whole-archive ${lib} -Wl,--no-whole-archive)
                elseif("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
                    list(APPEND final -Wl,-force_load ${lib})
                else()
                    # Unknown platform.
                    list(APPEND final ${lib})
                endif()
            endforeach()
            set(${result} ${final} PARENT_SCOPE)
            message(STATUS "Woof ${final}")
        endfunction()

        function(pxr_files result root)
            #file(GLOB result_h    "${USD_ROOT}/pxr/base/plug/*.h")
            file(GLOB result_cpp  "${root}/*.cpp")
            file(GLOB result_wrap "${root}/wrap*.cpp")
            file(GLOB result_temp "${root}/*.template.cpp")
            file(GLOB result_ex   "${root}/examples.cpp")
            file(GLOB result_v1   "${root}/*_v1.cpp")
            file(GLOB result_py   "${root}/py*.*")
            file(GLOB result_py1  "${root}/arrayPy*.*")
            file(GLOB result_py2  "${root}/makePy*.*")
            file(GLOB result_py3  "${root}/module*.cpp")
            file(GLOB result_py4  "${root}/script*.cpp")
            file(GLOB result_py5  "${root}/valueFromPy*.cpp")
            file(GLOB result_test "${root}/test*.cpp")
            list(REMOVE_ITEM result_cpp ${result_test} ${result_temp}
                                        ${result_ex}   ${result_wrap}
                                        ${result_v1})
            list(REMOVE_ITEM result_cpp ${result_py}   ${result_py1}
                                        ${result_py2}  ${result_py3} 
                                        ${result_py4}  ${result_py5})
            set(${result} ${result_cpp} PARENT_SCOPE)
        endfunction()

# arch

        file(GLOB arch_h   "${USD_ROOT}/pxr/base/arch/*.h")
        pxr_files(arch_cpp "${USD_ROOT}/pxr/base/arch/")
        set(arch_group base)
        add_library(arch STATIC ${arch_h} ${arch_cpp})
        set_property(TARGET arch PROPERTY CXX_STANDARD 14)
        target_include_directories(arch SYSTEM 
            PUBLIC
            $<BUILD_INTERFACE:${USD_ROOT}>
            $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>)

# gf

        file(GLOB gf_h  "${USD_ROOT}/pxr/base/gf/*.h")
        pxr_files(gf_cpp "${USD_ROOT}/pxr/base/gf/")
        set(gf_group base)
        add_library(gf STATIC ${gf_h} ${gf_cpp})
        set_property(TARGET gf PROPERTY CXX_STANDARD 14)
        target_include_directories(gf SYSTEM 
            PUBLIC
            $<BUILD_INTERFACE:${USD_ROOT}>
            $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>)

# js

        file(GLOB js_h   "${USD_ROOT}/pxr/base/js/*.h")
        pxr_files(js_cpp "${USD_ROOT}/pxr/base/js/")
        set(js_group base)
        add_library(js STATIC ${js_h} ${js_cpp})
        set_property(TARGET js PROPERTY CXX_STANDARD 14)
        target_include_directories(js SYSTEM 
            PUBLIC
            $<BUILD_INTERFACE:${USD_ROOT}>
            $<BUILD_INTERFACE:${USD_ROOT}/pxr/base/js>
            $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>)

# plug

        file(GLOB plug_h    "${USD_ROOT}/pxr/base/plug/*.h")
        pxr_files(plug_cpp "${USD_ROOT}/pxr/base/plug/")
        set(plug_group base)
        add_library(plug STATIC ${plug_h} ${plug_cpp})
        set_property(TARGET plug PROPERTY CXX_STANDARD 14)
        target_include_directories(plug SYSTEM 
            PUBLIC
            $<BUILD_INTERFACE:${USD_ROOT}>
            $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>)

# tf

        file(GLOB tf_h    "${USD_ROOT}/pxr/base/tf/*.h")
        pxr_files(tf_cpp "${USD_ROOT}/pxr/base/tf/")
        set(tf_group base)
        add_library(tf STATIC ${tf_h} ${tf_cpp}
            "${USD_ROOT}/pxr/base/tf/pxrDoubleConversion/double-conversion.cc"
            "${USD_ROOT}/pxr/base/tf/pxrDoubleConversion/bignum.cc"
            "${USD_ROOT}/pxr/base/tf/pxrDoubleConversion/bignum-dtoa.cc"
            "${USD_ROOT}/pxr/base/tf/pxrDoubleConversion/cached-powers.cc"
            "${USD_ROOT}/pxr/base/tf/pxrDoubleConversion/diy-fp.cc"
            "${USD_ROOT}/pxr/base/tf/pxrDoubleConversion/fast-dtoa.cc"
            "${USD_ROOT}/pxr/base/tf/pxrDoubleConversion/fixed-dtoa.cc"
            "${USD_ROOT}/pxr/base/tf/pxrDoubleConversion/strtod.cc"
            "${USD_ROOT}/pxr/base/tf/pxrLZ4/lz4.cpp")
        set_property(TARGET tf PROPERTY CXX_STANDARD 14)
        target_include_directories(tf SYSTEM 
            PUBLIC
            $<BUILD_INTERFACE:${USD_ROOT}>
            $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>)

# trace

        file(GLOB trace_h    "${USD_ROOT}/pxr/base/trace/*.h")
        pxr_files(trace_cpp "${USD_ROOT}/pxr/base/trace/")
        set(trace_group base) 
        add_library(trace STATIC ${trace_h} ${trace_cpp})
        set_property(TARGET trace PROPERTY CXX_STANDARD 14)
        target_include_directories(trace SYSTEM 
            PUBLIC
            $<BUILD_INTERFACE:${USD_ROOT}>
            $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>)

# vt

        file(GLOB vt_h    "${USD_ROOT}/pxr/base/vt/*.h")
        pxr_files(vt_cpp  "${USD_ROOT}/pxr/base/vt/")
        set(vt_group base)
        add_library(vt STATIC ${vt_h} ${vt_cpp})
        set_property(TARGET vt PROPERTY CXX_STANDARD 14)
        target_include_directories(vt SYSTEM 
            PUBLIC
            $<BUILD_INTERFACE:${USD_ROOT}>
            $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>)


# ar

        file(GLOB ar_h    "${USD_ROOT}/pxr/usd/ar/*.h")
        pxr_files(ar_cpp  "${USD_ROOT}/pxr/usd/ar/")
        set(ar_group usd)
        add_library(ar STATIC ${ar_h} ${ar_cpp})
        set_property(TARGET ar PROPERTY CXX_STANDARD 14)
        target_include_directories(ar SYSTEM 
            PUBLIC
            $<BUILD_INTERFACE:${USD_ROOT}>
            $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>)


# kind 

        file(GLOB kind_h    "${USD_ROOT}/pxr/usd/kind/*.h")
        pxr_files(kind_cpp  "${USD_ROOT}/pxr/usd/kind/")
        set(kind_group usd)
        add_library(kind STATIC ${kind_h} ${kind_cpp})
        set_property(TARGET kind PROPERTY CXX_STANDARD 14)
        target_include_directories(kind SYSTEM 
            PUBLIC
            $<BUILD_INTERFACE:${USD_ROOT}>
            $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>)


# ndr 

        file(GLOB ndr_h    "${USD_ROOT}/pxr/usd/ndr/*.h")
        pxr_files(ndr_cpp  "${USD_ROOT}/pxr/usd/ndr/")
        set(ndr_group usd)
        add_library(ndr STATIC ${ndr_h} ${ndr_cpp})
        set_property(TARGET ndr PROPERTY CXX_STANDARD 14)
        target_include_directories(ndr SYSTEM 
            PUBLIC
            $<BUILD_INTERFACE:${USD_ROOT}>
            $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>)

# pcp 

        file(GLOB pcp_h    "${USD_ROOT}/pxr/usd/pcp/*.h")
        pxr_files(pcp_cpp  "${USD_ROOT}/pxr/usd/pcp/")
        set(pcp_group usd)
        add_library(pcp STATIC ${pcp_h} ${pcp_cpp})
        set_property(TARGET pcp PROPERTY CXX_STANDARD 14)
        target_include_directories(pcp SYSTEM 
            PUBLIC
            $<BUILD_INTERFACE:${USD_ROOT}>
            $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>)

# sdf 

        file(GLOB sdf_h    "${USD_ROOT}/pxr/usd/sdf/*.h")
        pxr_files(sdf_cpp  "${USD_ROOT}/pxr/usd/sdf/")
        set(sdf_group usd)
        add_library(sdf STATIC ${sdf_h} ${sdf_cpp})
        set_property(TARGET sdf PROPERTY CXX_STANDARD 14)
        target_include_directories(sdf SYSTEM 
            PUBLIC
            $<BUILD_INTERFACE:${USD_ROOT}>
            $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>)

# sdr 

        file(GLOB sdr_h    "${USD_ROOT}/pxr/usd/sdr/*.h")
        pxr_files(sdr_cpp  "${USD_ROOT}/pxr/usd/sdr/")
        set(sdr_group sdr)
        add_library(sdr STATIC ${sdr_h} ${sdr_cpp})
        set_property(TARGET sdr PROPERTY CXX_STANDARD 14)
        target_include_directories(sdr SYSTEM 
            PUBLIC
            $<BUILD_INTERFACE:${USD_ROOT}>
            $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>)

# usd 

        file(GLOB usd_h    "${USD_ROOT}/pxr/usd/usd/*.h")
        pxr_files(usd_cpp  "${USD_ROOT}/pxr/usd/usd/")
        set(usd_group usd)
        add_library(usd STATIC ${usd_h} ${usd_cpp})
        set_property(TARGET usd PROPERTY CXX_STANDARD 14)
        target_include_directories(usd SYSTEM 
            PUBLIC
            $<BUILD_INTERFACE:${USD_ROOT}>
            $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>)

# work

        file(GLOB work_h    "${USD_ROOT}/pxr/base/work/*.h")
        pxr_files(work_cpp  "${USD_ROOT}/pxr/base/work/")
        set(work_group base)
        add_library(work STATIC ${work_h} ${work_cpp})
        set_property(TARGET work PROPERTY CXX_STANDARD 14)
        target_include_directories(work SYSTEM 
            PUBLIC
            $<BUILD_INTERFACE:${USD_ROOT}>
            $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>)


# usdGeom

        file(GLOB usdGeom_h    "${USD_ROOT}/pxr/usd/usdGeom/*.h")
        pxr_files(usdGeom_cpp  "${USD_ROOT}/pxr/usd/usdGeom/")
        set(usdGeom_group usd)
        add_library(usdGeom STATIC ${usdGeom_h} ${usdGeom_cpp})
        set_property(TARGET usdGeom PROPERTY CXX_STANDARD 14)
        target_include_directories(usdGeom SYSTEM 
            PUBLIC
            $<BUILD_INTERFACE:${USD_ROOT}>
            $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>)



# usdHydra

        file(GLOB usdHydra_h    "${USD_ROOT}/pxr/usd/usdHydra/*.h")
        pxr_files(usdHydra_cpp  "${USD_ROOT}/pxr/usd/usdHydra/")
        set(usdHydra_group usd)
        add_library(usdHydra STATIC ${usdHydra_h} ${usdHydra_cpp})
        set_property(TARGET usdHydra PROPERTY CXX_STANDARD 14)
        target_include_directories(usdHydra SYSTEM 
            PUBLIC
            $<BUILD_INTERFACE:${USD_ROOT}>
            $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>)



# usdLux

        file(GLOB usdLux_h    "${USD_ROOT}/pxr/usd/usdLux/*.h")
        pxr_files(usdLux_cpp  "${USD_ROOT}/pxr/usd/usdLux/")
        set(usdLux_group usd)
        add_library(usdLux STATIC ${usdLux_h} ${usdLux_cpp})
        set_property(TARGET usdLux PROPERTY CXX_STANDARD 14)
        target_include_directories(usdLux SYSTEM 
            PUBLIC
            $<BUILD_INTERFACE:${USD_ROOT}>
            $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>)


# usdMedia

        file(GLOB usdMedia_h    "${USD_ROOT}/pxr/usd/usdMedia/*.h")
        pxr_files(usdMedia_cpp  "${USD_ROOT}/pxr/usd/usdMedia/")
        set(usdMedia_group usd)
        add_library(usdMedia STATIC ${usdMedia_h} ${usdMedia_cpp})
        set_property(TARGET usdMedia PROPERTY CXX_STANDARD 14)
        target_include_directories(usdMedia SYSTEM 
            PUBLIC
            $<BUILD_INTERFACE:${USD_ROOT}>
            $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>)



# usdPhysics

        file(GLOB usdPhysics_h    "${USD_ROOT}/pxr/usd/usdPhysics/*.h")
        pxr_files(usdPhysics_cpp  "${USD_ROOT}/pxr/usd/usdPhysics/")
        set(usdPhysics_group usd)
        add_library(usdPhysics STATIC ${usdPhysics_h} ${usdPhysics_cpp})
        set_property(TARGET usdPhysics PROPERTY CXX_STANDARD 14)
        target_include_directories(usdPhysics SYSTEM 
            PUBLIC
            $<BUILD_INTERFACE:${USD_ROOT}>
            $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>)


# usdRi

        file(GLOB usdRi_h    "${USD_ROOT}/pxr/usd/usdRi/*.h")
        pxr_files(usdRi_cpp  "${USD_ROOT}/pxr/usd/usdRi/")
        set(usdRi_group usd)
        add_library(usdRi STATIC ${usdRi_h} ${usdRi_cpp})
        set_property(TARGET usdRi PROPERTY CXX_STANDARD 14)
        target_include_directories(usdRi SYSTEM 
            PUBLIC
            $<BUILD_INTERFACE:${USD_ROOT}>
            $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>)


# usdShade

        file(GLOB usdShade_h    "${USD_ROOT}/pxr/usd/usdShade/*.h")
        pxr_files(usdShade_cpp  "${USD_ROOT}/pxr/usd/usdShade/")
        set(usdShade_group usd)
        add_library(usdShade STATIC ${usdShade_h} ${usdShade_cpp})
        set_property(TARGET usdShade PROPERTY CXX_STANDARD 14)
        target_include_directories(usdShade SYSTEM 
            PUBLIC
            $<BUILD_INTERFACE:${USD_ROOT}>
            $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>)


# usdSkel

        file(GLOB usdSkel_h    "${USD_ROOT}/pxr/usd/usdSkel/*.h")
        pxr_files(usdSkel_cpp  "${USD_ROOT}/pxr/usd/usdSkel/")
        set(usdSkel_group usd)
        add_library(usdSkel STATIC ${usdSkel_h} ${usdSkel_cpp})
        set_property(TARGET usdSkel PROPERTY CXX_STANDARD 14)
        target_include_directories(usdSkel SYSTEM 
            PUBLIC
            $<BUILD_INTERFACE:${USD_ROOT}>
            $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>)



# usdUI

        file(GLOB usdUI_h    "${USD_ROOT}/pxr/usd/usdUI/*.h")
        pxr_files(usdUI_cpp  "${USD_ROOT}/pxr/usd/usdUI/")
        set(usdUI_group usd)
        add_library(usdUI STATIC ${usdUI_h} ${usdUI_cpp})
        set_property(TARGET usdUI PROPERTY CXX_STANDARD 14)
        target_include_directories(usdUI SYSTEM 
            PUBLIC
            $<BUILD_INTERFACE:${USD_ROOT}>
            $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>)


# usdUtils

        file(GLOB usdUtils_h    "${USD_ROOT}/pxr/usd/usdUtils/*.h")
        pxr_files(usdUtils_cpp  "${USD_ROOT}/pxr/usd/usdUtils/")
        set(usdUtils_group usd)
        add_library(usdUtils STATIC ${usdUtils_h} ${usdUtils_cpp})
        set_property(TARGET usdUtils PROPERTY CXX_STANDARD 14)
        target_include_directories(usdUtils SYSTEM 
            PUBLIC
            $<BUILD_INTERFACE:${USD_ROOT}>
            $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>)

# usdVol

        file(GLOB usdVol_h    "${USD_ROOT}/pxr/usd/usdVol/*.h")
        pxr_files(usdVol_cpp  "${USD_ROOT}/pxr/usd/usdVol/")
        set(usdVol_group usd)
        add_library(usdVol STATIC ${usdVol_h} ${usdVol_cpp})
        set_property(TARGET usdVol PROPERTY CXX_STANDARD 14)
        target_include_directories(usdVol SYSTEM 
            PUBLIC
            $<BUILD_INTERFACE:${USD_ROOT}>
            $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>)

        #add_executable(tinyusd tinyusd.cpp)
        #target_include_directories(tinyusd SYSTEM 
        #    PUBLIC
        #    $<BUILD_INTERFACE:${USD_ROOT}>
        #    $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>)

        set(usd_libs
            arch gf js plug tf trace vt ar work kind ndr pcp
            sdf sdr usd usdGeom usdHydra usdLux usdMedia usdPhysics
            usdRi usdShade usdSkel usdUI usdUtils usdVol)
        set(usd_libs_resolved "")

        message(STATUS "Zounds: ${usd_libs}")

        pxr_whole("${usd_libs}" usd_libs_resolved)

        message(STATUS "Hello! ${usd_libs_resolved}")

        #target_link_libraries(tinyusd
        #    ${CMAKE_BINARY_DIR}/libtbb.dylib
        #    ${CMAKE_BINARY_DIR}/libz.1.2.11.dylib
        #    ${usd_libs_resolved})

        #set_property(TARGET tinyusd PROPERTY CXX_STANDARD 14)

        foreach(lib ${usd_libs})
            add_library(pxrusd::${lib} ALIAS ${lib})
            install(FILES ${${lib}_h} 
                DESTINATION "${CMAKE_INSTALL_PREFIX}/include/pxr/${${lib}_group}/${lib}")

            #            install(
            #    TARGETS ${lib}
            #    EXPORT ${lib}Config
            #    LIBRARY DESTINATION lib
            #    ARCHIVE DESTINATION lib
            #    RUNTIME DESTINATION bin
            #    PUBLIC_HEADER DESTINATION include/pxr)

            #install(EXPORT ${lib}Config
            #    DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/pxrusd"
            #    NAMESPACE pxrusd:: )

        endforeach()


        install(
            TARGETS ${usd_libs}
            EXPORT pxrusdConfig
            LIBRARY DESTINATION lib
            ARCHIVE DESTINATION lib
            RUNTIME DESTINATION bin
            PUBLIC_HEADER DESTINATION include/pxr)

        install(EXPORT pxrusdConfig
            DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/pxrusd"
            NAMESPACE pxrusd:: )


    endif()
endif()

