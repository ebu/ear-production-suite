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

if(BUILD_TESTING)
    add_executable(test_project_upgrade test_project_upgrade.cpp)
    target_sources(test_project_upgrade
            PRIVATE
            test_project_upgrade.cpp)
    target_link_libraries(test_project_upgrade
            PRIVATE
            proj_upgrade
            Catch::catch2)
    set_target_properties(test_project_upgrade PROPERTIES FOLDER tools/tests)
    add_test(NAME test_project_upgrade
            COMMAND test_project_upgrade
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endif()