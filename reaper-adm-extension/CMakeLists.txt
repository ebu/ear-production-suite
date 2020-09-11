cmake_minimum_required(VERSION 3.10.0)
project(reaper_adm VERSION 0.6.0 LANGUAGES CXX)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT AND (NOT INITIAL_CMAKE_INSTALL_PREFIX_OVERRIDDEN))
    if(APPLE)
        set(INITIAL_CMAKE_INSTALL_PREFIX_OVERRIDDEN ON CACHE INTERNAL "Has default install prefix been overridden once?" FORCE)
        set(CMAKE_INSTALL_PREFIX "$ENV{HOME}/Library/Application Support/REAPER" CACHE PATH "Install path prefix, prepended onto install directories" FORCE)
    endif()
    if(WIN32)
        set(INITIAL_CMAKE_INSTALL_PREFIX_OVERRIDDEN ON CACHE INTERNAL "Has default install prefix been overridden once?" FORCE)
        set(CMAKE_INSTALL_PREFIX "$ENV{programfiles}/REAPER (x64)" CACHE PATH "Install path prefix, prepended onto install directories" FORCE)
    endif()
endif()

include(CTest)
set(Boost_USE_STATIC_LIBS ON)
add_subdirectory(src)
if(BUILD_TESTING)
    add_subdirectory(test)
endif()

message(${CMAKE_SYSTEM_NAME})

