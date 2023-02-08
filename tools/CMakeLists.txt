add_subdirectory(project_upgrade)
if(NOT "${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
	add_subdirectory(setup)
endif()