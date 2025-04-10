function(set_includes_as_system tgt)
    get_target_property(public_includes ${tgt} INTERFACE_INCLUDE_DIRECTORIES)
    set_target_properties(${tgt} PROPERTIES INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${public_includes}")
endfunction()

message(STATUS "Processing submodules...")

## Disable testing and reduce noise when building submodules
option(QUIET_SUBMODULES "Limit submodule output to errors" ON)
set(isTesting ${BUILD_TESTING})
set(LOG_LEVEL ${CMAKE_MESSAGE_LOG_LEVEL})
set(DEPRECATION_WARN ${CMAKE_WARN_DEPRECATED})
if(QUIET_SUBMODULES)
  add_compile_options("$<IF:$<CXX_COMPILER_ID:MSVC>,/w,-w>")
  set(CMAKE_MESSAGE_LOG_LEVEL ERROR)
  set(CMAKE_WARN_DEPRECATED OFF CACHE BOOL "Enable cmake deprecation warnings")
endif()

# JUCE
if((NOT DEFINED REAPPLY_JUCE_PATCHES) OR (${REAPPLY_JUCE_PATCHES}))
	execute_process(COMMAND git apply ../0001-JUCE_Allow-Windows-to-detect-UDP-port-open-fail.patch
		WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/JUCE")
	set(REAPPLY_JUCE_PATCHES OFF CACHE BOOL "" FORCE)
endif()

# VST3
if (MSVC) # Fix for MSVC 2022 based on https://forums.steinberg.net/t/pluginterfaces-lib-compilation-error-win-10-vs-2022/768976/4
	set(SMTG_USE_STDATOMIC_H OFF CACHE BOOL "" FORCE)
endif()
set(SMTG_USE_STATIC_CRT ON CACHE BOOL "" FORCE)
set(SMTG_ADD_VSTGUI OFF CACHE BOOL "" FORCE)
if((NOT DEFINED REAPPLY_VST3_PATCHES) OR (${REAPPLY_VST3_PATCHES}))
	execute_process(COMMAND git apply ../../0001-VST3-base_Add-MinSizeRel-config.patch
		WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/VST3/base")
	execute_process(COMMAND git apply ../../0001-VST3-cmake_Add-RelWithDebInfo-and-MinSizeRel-configs.patch
		WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/VST3/cmake")
	execute_process(COMMAND git apply ../../0002-VST3-cmake_Dont-mess-with-project-folders.patch
		WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/VST3/cmake")
	set(REAPPLY_VST3_PATCHES OFF CACHE BOOL "" FORCE)
endif()
add_subdirectory(VST3)
set_target_properties(base cmake_modules pluginterfaces sdk sdk_common sdk_hosting validator PROPERTIES FOLDER submodules/vst3)
if(QUIET_SUBMODULES)
set_includes_as_system(base)
set_includes_as_system(sdk)
set_includes_as_system(sdk_common)
endif()

# libear
set(EAR_HIDE_INTERNAL_SYMBOLS OFF CACHE BOOL " " FORCE)
set(EAR_UNIT_TESTS OFF CACHE BOOL " " FORCE)
set(EAR_EXAMPLES OFF CACHE BOOL " " FORCE)
set(EAR_PACKAGE_AND_INSTALL OFF CACHE BOOL " " FORCE)
set(BUILD_SHARED_LIBS OFF)
#add_subdirectory(libear) ### COMMENTED OUT while using BEARs custom EAR
#set_target_properties(ear libear_resource_embedder PROPERTIES FOLDER submodules/libear)

#libadm
set(BUILD_TESTING OFF)
set(BUILD_SHARED_LIBS OFF)
set(UNIT_TESTS OFF CACHE BOOL "libadm unit tests" FORCE)
set(ADM_HIDE_INTERNAL_SYMBOLS OFF CACHE BOOL "hide libadm symbols by default" FORCE)
set(ADM_EXAMPLES OFF CACHE BOOL "Build ADM examples" FORCE)
add_subdirectory(libadm)
set_target_properties(adm PROPERTIES FOLDER submodules/libadm)

#AdmCoordConv
set(BUILD_TESTS OFF)
set(BUILD_SHARED_LIBS OFF)
add_subdirectory(adm_coordinate_conversion EXCLUDE_FROM_ALL) # EXCLUDE_FROM_ALL prevents AdmCoordConv default install behaviour)
set_target_properties(AdmCoordConv PROPERTIES FOLDER submodules/AdmCoordConv)

#libbw64
add_subdirectory(libbw64)
add_library(IRT::bw64 INTERFACE IMPORTED GLOBAL)
set_target_properties(IRT::bw64 PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/libbw64/include>)

#googletest
if(isTesting)
set(BUILD_SHARED_LIBS OFF)
set(INSTALL_GTEST OFF CACHE BOOL "Install gtest" FORCE)
set(gtest_force_shared_crt ON)
add_subdirectory(googletest)
add_subdirectory(Catch2)
add_library(eps_catch2 INTERFACE)
add_library(EPS::Catch2WithMain ALIAS eps_catch2)
target_sources(eps_catch2 INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/catch_main.cpp)
target_link_libraries(eps_catch2 INTERFACE Catch2::Catch2)
target_link_libraries(eps_catch2 INTERFACE gmock)
set_target_properties(
        Catch2
        Catch2WithMain
        gmock
        gmock_main
        gtest
        gtest_main
        PROPERTIES
            FOLDER submodules/test-framework)
endif()

### VISR

message(STATUS CMAKE_SYSTEM_PROCESSOR " - " ${CMAKE_SYSTEM_PROCESSOR})
message(STATUS CMAKE_SYSTEM_NAME " - " ${CMAKE_SYSTEM_NAME})
message(STATUS CMAKE_CROSSCOMPILING " - " ${CMAKE_CROSSCOMPILING})

set(BUILD_PYTHON_BINDINGS OFF CACHE BOOL "Python" FORCE)
set(BUILD_AUDIOINTERFACES_JACK OFF CACHE BOOL "Jack" FORCE)
set(BUILD_AUDIOINTERFACES_PORTAUDIO OFF CACHE BOOL "PortAudio" FORCE)
set(BUILD_DOCUMENTATION OFF CACHE BOOL "Docs" FORCE)
set(BUILD_MATLAB_EXTERNALS OFF CACHE BOOL "Matlab" FORCE)
set(BUILD_MAX_MSP_EXTERNALS OFF CACHE BOOL "Max" FORCE)
set(Boost_USE_STATIC_LIBS ON CACHE BOOL "Boost" FORCE)
set(BUILD_INSTALL_SHARED_LIBRARIES OFF CACHE BOOL "shared libs" FORCE)
set(BUILD_INSTALL_STATIC_LIBRARIES OFF CACHE BOOL "static libs" FORCE)
set(BUILD_INSTALL_STATIC_PIC_LIBRARIES ON CACHE BOOL "static pic libs" FORCE)
set(BUILD_USE_SNDFILE_LIBRARY OFF CACHE BOOL "libsndfile" FORCE)
set(BUILD_STANDALONE_APPLICATIONS OFF CACHE BOOL "applications" FORCE)

set(VISR_TARGET_TYPE static_pic)

include(FetchContent)
FetchContent_Declare(
    visr
    GIT_REPOSITORY https://github.com/ebu/bear.git
    GIT_TAG visr
    SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/visr"
)
FetchContent_GetProperties(visr)
if(NOT visr_POPULATED)
    FetchContent_Populate(visr)

endif()

message(STATUS visr_POPULATED " - " ${visr_POPULATED})
message(STATUS visr_SOURCE_DIR " - " ${visr_SOURCE_DIR})
message(STATUS visr_BINARY_DIR " - " ${visr_BINARY_DIR})

add_subdirectory(${visr_SOURCE_DIR} ${visr_BINARY_DIR} EXCLUDE_FROM_ALL) # EXCLUDE_FROM_ALL prevents VISRs default install behaviour

# Only add targets optimised with specific instruction sets where they exist
# e.g. don't try and link to SSE optimised build on arm64
set(base_efl_target efl_${VISR_TARGET_TYPE})
foreach(instruction_set IN ITEMS "" _AVX _FMA _SSE)
  set(current_target ${base_efl_target}${instruction_set})
  if(TARGET ${current_target})
    list(APPEND efl_targets ${current_target})
  endif()
endforeach()
message(STATUS "Using the following efl targets:\n${efl_targets}\n")

set(VISR_TARGET_NAMES
    apputilities_${VISR_TARGET_TYPE}
    audiointerfaces_${VISR_TARGET_TYPE}
    ${efl_targets}
    objectmodel_${VISR_TARGET_TYPE}
    panning_${VISR_TARGET_TYPE}
    panningdsp_${VISR_TARGET_TYPE}
    pml_${VISR_TARGET_TYPE}
    rbbl_${VISR_TARGET_TYPE}
    rcl_${VISR_TARGET_TYPE}
    reverbobject_${VISR_TARGET_TYPE}
    rrl_${VISR_TARGET_TYPE}
    signalflows_${VISR_TARGET_TYPE}
    signalflowspython_${VISR_TARGET_TYPE}
    visr_${VISR_TARGET_TYPE})

foreach(VISR_TARGET ${VISR_TARGET_NAMES} )
  if(TARGET ${VISR_TARGET})
    set_target_properties(${VISR_TARGET} PROPERTIES FOLDER submodules/VISR)
    # Put library in VISR namespace so BEAR can find it
    add_library(VISR::${VISR_TARGET} ALIAS ${VISR_TARGET})
    # Clear header properties so they don't install - we don't need them, and causes issues due to relative paths on OSX
    set_target_properties(${VISR_TARGET} PROPERTIES PUBLIC_HEADER "" )
    set_target_properties(${VISR_TARGET} PROPERTIES PRIVATE_HEADER "" )
  else()
	message(WARNING "VISR target not found: " ${VISR_TARGET})
  endif()
endforeach()
set_target_properties(ffts_static kissfft_static PROPERTIES FOLDER submodules/VISR/3rd)

### BEAR

set(BUILD_PYTHON_BINDINGS OFF CACHE BOOL "Python" FORCE)
set(BEAR_UNIT_TESTS OFF CACHE BOOL "bear unit tests" FORCE)
set(BEAR_DOWNLOAD_DATA_DEFAULT OFF CACHE BOOL "bear data files" FORCE)
set(BEAR_DOWNLOAD_DATA_DEFAULT_SMALL ON CACHE BOOL "bear data files" FORCE)
set(BEAR_VISR_LIB_TYPE "${VISR_TARGET_TYPE}" CACHE STRING "bear visr lib type" FORCE)
add_subdirectory(bear/visr_bear)
set_target_properties(bear PROPERTIES FOLDER submodules/bear)
set_target_properties(ear PROPERTIES FOLDER submodules/libear)
# Folder up instruction-set specific targets (platform specific so need to check exists)
if(TARGET ear_avx512bw)
  set_target_properties(ear_avx512bw PROPERTIES FOLDER submodules/libear)
endif()
if(TARGET ear_avx2_fma)
  set_target_properties(ear_avx2_fma PROPERTIES FOLDER submodules/libear)
endif()
if(TARGET ear_avx)
  set_target_properties(ear_avx PROPERTIES FOLDER submodules/libear)
endif()
if(TARGET ear_sse4_2)
  set_target_properties(ear_sse4_2 PROPERTIES FOLDER submodules/libear)
endif()
if(TARGET ear_default_arch)
  set_target_properties(ear_default_arch PROPERTIES FOLDER submodules/libear)
endif()
add_dependencies(bear ${VISR_TARGET_NAMES})

set(BUILD_TESTING ${isTesting})
set(CMAKE_MESSAGE_LOG_LEVEL ${LOG_LEVEL})
set(CMAKE_WARN_DEPRECATED ${DEPRECATION_WARN} CACHE BOOL "Enable cmake deprecation warnings" FORCE)

message(STATUS "Processing submodules DONE!")
