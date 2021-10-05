message(STATUS "Processing submodules...")

set(isTesting ${BUILD_TESTING})
set(LOG_LEVEL ${CMAKE_MESSAGE_LOG_LEVEL})
set(CMAKE_MESSAGE_LOG_LEVEL ERROR)
set(DEPRECATION_WARN ${CMAKE_WARN_DEPRECATED})
set(CMAKE_WARN_DEPRECATED OFF CACHE BOOL "Enable cmake deprecation warnings")

# JUCE
execute_process(COMMAND git apply ../0001-fix-objc_msgsend-for-M1-mac.patch
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/JUCE"
    ERROR_QUIET)

# libear
set(EAR_HIDE_INTERNAL_SYMBOLS OFF CACHE BOOL " " FORCE)
set(EAR_UNIT_TESTS OFF CACHE BOOL " " FORCE)
set(EAR_EXAMPLES OFF CACHE BOOL " " FORCE)
set(EAR_PACKAGE_AND_INSTALL OFF CACHE BOOL " " FORCE)
set(BUILD_SHARED_LIBS OFF)
add_subdirectory(libear)
set_target_properties(ear PROPERTIES FOLDER submodules/libear)

#libadm
set(BUILD_TESTING OFF)
set(UNIT_TESTS OFF CACHE BOOL "libadm unit tests" FORCE)
set(ADM_HIDE_INTERNAL_SYMBOLS OFF CACHE BOOL "hide libadm symbols by default" FORCE)
set(ADM_EXAMPLES OFF CACHE BOOL "Build ADM examples" FORCE)
add_subdirectory(libadm)
set_target_properties(adm PROPERTIES FOLDER submodules/libadm)

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



set(BUILD_TESTING ${isTesting})
set(CMAKE_MESSAGE_LOG_LEVEL ${LOG_LEVEL})
set(CMAKE_WARN_DEPRECATED ${DEPRECATION_WARN} CACHE BOOL "Enable cmake deprecation warnings" FORCE)

message(STATUS "Processing submodules DONE!")
