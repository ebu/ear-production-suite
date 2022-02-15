if(MSVC)
    add_compile_options(/bigobj)
endif()

add_library(proj_upgrade)
target_sources(proj_upgrade
        PRIVATE
        upgrade.cpp)
target_compile_features(proj_upgrade PUBLIC cxx_std_17)
set_target_properties(proj_upgrade PROPERTIES FOLDER tools)

add_executable(project_upgrade)
target_sources(project_upgrade
        PRIVATE
        main.cpp)
set_target_properties(project_upgrade PROPERTIES FOLDER tools)		
target_link_libraries(project_upgrade
        PRIVATE
        proj_upgrade)

find_package(JUCE REQUIRED QUIET)

set(MACOSX_BUNDLE_BUNDLE_NAME "EPS Project Upgrade Utility")
set(MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}")
set(MACOSX_BUNDLE_GUI_IDENTIFIER "ch.ebu.reaper_project_upgrade_gui")
set(MACOSX_BUNDLE_COPYRIGHT "(C) 2019-2022 BBC, (C) 2020-2021 EBU, (C) 2019-2021 IRT")
add_executable(project_upgrade_gui WIN32 MACOSX_BUNDLE)
target_sources(project_upgrade_gui
        PRIVATE
        gui_app/Main.cpp
        gui_app/MainComponent.cpp
        ${EPS_SHARED_DIR}/binary_data.cpp)
if(APPLE)
	set_target_properties(project_upgrade_gui PROPERTIES
        OUTPUT_NAME "Project Upgrade Utility GUI"
        XCODE_ATTRIBUTE_EXECUTABLE_NAME "Project Upgrade Utility"
        MACOSX_BUNDLE TRUE)
endif()
if(WIN32)
	set_target_properties(project_upgrade_gui PROPERTIES
        OUTPUT_NAME "project_upgrade_utility_gui"
        WIN32_EXECUTABLE TRUE)
endif()
set_target_properties(project_upgrade_gui PROPERTIES FOLDER tools)		
target_link_libraries(project_upgrade_gui PRIVATE Juce::core ear-version)
target_include_directories(project_upgrade_gui PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/gui_app ${EPS_SHARED_DIR})
target_link_libraries(project_upgrade_gui
        PRIVATE
        proj_upgrade)

if(BUILD_TESTING)
    add_executable(test_project_upgrade test_project_upgrade.cpp)
    target_sources(test_project_upgrade
            PRIVATE
            test_project_upgrade.cpp)
    target_link_libraries(test_project_upgrade
            PRIVATE
            proj_upgrade
            EPS::Catch2WithMain)
    set_target_properties(test_project_upgrade PROPERTIES FOLDER tools/tests)
    add_test(NAME test_project_upgrade
            COMMAND test_project_upgrade
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endif()