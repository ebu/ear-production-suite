install(FILES
        EAR-Production-Suite_Session-Template.RPP
        README.md
        DESTINATION
        .)
install(TARGETS
        project_upgrade
        project_upgrade_gui
        DESTINATION
        tools)
set(CPACK_GENERATOR "ZIP")
set(CPACK_PACKAGE_NAME "EAR Production Suite")
set(CPACK_PACKAGE_VERSION "${CMAKE_PROJECT_VERSION}")
set(CPACK_PACKAGE_VENDOR "EBU")
set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/LICENSE.TXT")
set(CPACK_MONOLITHIC_INSTALL ON)
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
include(CPack)
