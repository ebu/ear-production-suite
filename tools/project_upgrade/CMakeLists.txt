add_library(proj_upgrade)
target_sources(proj_upgrade
        PRIVATE
        upgrade.cpp)
target_compile_features(proj_upgrade PUBLIC cxx_std_17)

add_executable(project_upgrade)
target_sources(project_upgrade
        PRIVATE
        main.cpp)

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
    add_test(NAME test_project_upgrade
            COMMAND test_project_upgrade
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endif()