# libear
set(EAR_HIDE_INTERNAL_SYMBOLS OFF CACHE BOOL " " FORCE)
set(EAR_UNIT_TESTS OFF CACHE BOOL " " FORCE)
set(EAR_EXAMPLES OFF CACHE BOOL " " FORCE)
set(EAR_PACKAGE_AND_INSTALL OFF CACHE BOOL " " FORCE)
set(BUILD_SHARED_LIBS OFF)
add_subdirectory(libear)
set(isTesting ${BUILD_TESTING})
set(BUILD_TESTING OFF)
set(UNIT_TESTS OFF CACHE BOOL "libadm unit tests" FORCE)
set(ADM_HIDE_INTERNAL_SYMBOLS OFF CACHE BOOL "hide libadm symbols by default" FORCE)
set(ADM_EXAMPLES OFF CACHE BOOL "Build ADM examples" FORCE)
add_subdirectory(libadm)
set(BUILD_TESTING ${isTesting})
add_subdirectory(libbw64)

#add_library(uuid INTERFACE)
#target_include_directories(uuid INTERFACE ${PROJECT_SOURCE_DIR}/submodules/stduuid/include)
add_library(IRT::bw64 INTERFACE IMPORTED GLOBAL)
set_target_properties(IRT::bw64 PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/libbw64/include>)

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
