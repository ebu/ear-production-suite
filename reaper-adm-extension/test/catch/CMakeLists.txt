#add_library(catch2 INTERFACE)
#add_library(Catch::catch2 ALIAS catch2)
#target_include_directories(catch2
#    INTERFACE
#        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/catch2>)
#
#target_sources(catch2
#    INTERFACE
#        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/catch2_main.cpp>)
#
#target_compile_features(catch2
#    INTERFACE
#        cxx_std_11
#        )
#
#target_link_libraries(catch2
#    INTERFACE
#        gmock)
