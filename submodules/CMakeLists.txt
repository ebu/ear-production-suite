message(STATUS "Processing submodules...")

set(isTesting ${BUILD_TESTING})
set(LOG_LEVEL ${CMAKE_MESSAGE_LOG_LEVEL})
set(CMAKE_MESSAGE_LOG_LEVEL ERROR)
set(DEPRECATION_WARN ${CMAKE_WARN_DEPRECATED})
set(CMAKE_WARN_DEPRECATED OFF CACHE BOOL "Enable cmake deprecation warnings")



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
set(UNIT_TESTS OFF CACHE BOOL "libadm unit tests" FORCE)
set(ADM_HIDE_INTERNAL_SYMBOLS OFF CACHE BOOL "hide libadm symbols by default" FORCE)
set(ADM_EXAMPLES OFF CACHE BOOL "Build ADM examples" FORCE)
add_subdirectory(libadm)
set_target_properties(adm resource_embedder PROPERTIES FOLDER submodules/libadm)

#libbw64
add_subdirectory(libbw64)
add_library(IRT::bw64 INTERFACE IMPORTED GLOBAL)
set_target_properties(IRT::bw64 PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/libbw64/include>)

#googletest
set(BUILD_SHARED_LIBS OFF)
set(INSTALL_GTEST OFF CACHE BOOL "Install gtest" FORCE)
set(gtest_force_shared_crt ON)
add_subdirectory(googletest)
add_library(catch2 STATIC ${CMAKE_SOURCE_DIR}/submodules/catch2/catch_main.cpp)
target_link_libraries(catch2
        PUBLIC
        gmock)
add_library(Catch::catch2 ALIAS catch2)
target_include_directories(catch2 PUBLIC ${CMAKE_SOURCE_DIR}/submodules)
target_compile_features(catch2 PUBLIC cxx_auto_type cxx_trailing_return_types)
set_target_properties(catch2 gmock gmock_main gtest gtest_main PROPERTIES FOLDER submodules/googletest)

### VISR

set(BUILD_PYTHON_BINDINGS OFF CACHE BOOL "Python" FORCE)
set(BUILD_AUDIOINTERFACES_JACK OFF CACHE BOOL "Jack" FORCE)
set(BUILD_AUDIOINTERFACES_PORTAUDIO OFF CACHE BOOL "PortAudio" FORCE)
set(BUILD_DOCUMENTATION OFF CACHE BOOL "Docs" FORCE)
set(BUILD_MATLAB_EXTERNALS OFF CACHE BOOL "Matlab" FORCE)
set(BUILD_MAX_MSP_EXTERNALS OFF CACHE BOOL "Max" FORCE)
set(Boost_USE_STATIC_LIBS ON CACHE BOOL "Boost" FORCE)
set(BUILD_INSTALL_SHARED_LIBRARIES OFF CACHE BOOL "shared libs" FORCE)
set(BUILD_INSTALL_STATIC_LIBRARIES OFF CACHE BOOL "shared libs" FORCE)
set(BUILD_INSTALL_STATIC_PIC_LIBRARIES ON CACHE BOOL "shared libs" FORCE)
set(BUILD_USE_SNDFILE_LIBRARY OFF CACHE BOOL "libsndfile" FORCE)
set(BUILD_STANDALONE_APPLICATIONS OFF CACHE BOOL "applications" FORCE)

set(VISR_TARGET_TYPE static_pic)

add_subdirectory(rd-audio-visr EXCLUDE_FROM_ALL) # EXCLUDE_FROM_ALL prevents VISRs default install behaviour

set(VISR_TARGET_NAMES
    apputilities_${VISR_TARGET_TYPE}
    audiointerfaces_${VISR_TARGET_TYPE}
    efl_${VISR_TARGET_TYPE}
    efl_${VISR_TARGET_TYPE}_AVX
    efl_${VISR_TARGET_TYPE}_FMA
    efl_${VISR_TARGET_TYPE}_SSE
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
	message(WARNING "VISR target not found: "${VISR_TARGET})
  endif()
endforeach()
set_target_properties(ffts_static kissfft_static PROPERTIES FOLDER submodules/VISR/3rd)

### BEAR

set(BUILD_PYTHON_BINDINGS OFF CACHE BOOL "Python" FORCE)
set(BEAR_UNIT_TESTS OFF CACHE BOOL "bear unit tests" FORCE)
set(BEAR_DOWNLOAD_DATA ON CACHE BOOL "bear data files" FORCE)
set(BEAR_VISR_LIB_TYPE "${VISR_TARGET_TYPE}" CACHE STRING "bear visr lib type" FORCE)
add_subdirectory(rd-audio-bear/visr_bear)
set_target_properties(bear PROPERTIES FOLDER submodules/bear)
set_target_properties(ear libear_resource_embedder PROPERTIES FOLDER submodules/libear)
add_dependencies(bear ${VISR_TARGET_NAMES})



set(BUILD_TESTING ${isTesting})
set(CMAKE_MESSAGE_LOG_LEVEL ${LOG_LEVEL})
set(CMAKE_WARN_DEPRECATED ${DEPRECATION_WARN} CACHE BOOL "Enable cmake deprecation warnings" FORCE)

message(STATUS "Processing submodules DONE!")