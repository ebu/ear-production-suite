add_custom_target(ear-version-generator ALL
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_SOURCE_DIR}/gen_version.cmake
	)
	
include(${CMAKE_CURRENT_SOURCE_DIR}/gen_version.cmake)
	
add_library(ear-version STATIC
	eps_version.cpp
	eps_version.h)
add_dependencies(ear-version ear-version-generator)

set_target_properties(ear-version PROPERTIES FOLDER versioning)
set_target_properties(ear-version-generator PROPERTIES FOLDER versioning)