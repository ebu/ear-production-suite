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
add_subdirectory(libear)
set_target_properties(ear libear_resource_embedder PROPERTIES FOLDER submodules/libear)

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



set(BUILD_TESTING ${isTesting})
set(CMAKE_MESSAGE_LOG_LEVEL ${LOG_LEVEL})
set(CMAKE_WARN_DEPRECATED ${DEPRECATION_WARN} CACHE BOOL "Enable cmake deprecation warnings" FORCE)

message(STATUS "Processing submodules DONE!")