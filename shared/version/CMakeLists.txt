set(EPS_VERSION_FILE ${CMAKE_CURRENT_SOURCE_DIR}/eps_version.cpp)

add_custom_target(ear-version-generator ALL
	BYPRODUCTS ${EPS_VERSION_FILE}
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	COMMAND ${CMAKE_COMMAND}
		-DCMAKE_PROJECT_VERSION=${CMAKE_PROJECT_VERSION}
		-P ${CMAKE_CURRENT_SOURCE_DIR}/gen_version.cmake
	)
	
include(${CMAKE_CURRENT_SOURCE_DIR}/gen_version.cmake)
	
add_library(ear-version STATIC
	${EPS_VERSION_FILE}
	eps_version.h)
add_dependencies(ear-version ear-version-generator)

set_target_properties(ear-version PROPERTIES FOLDER versioning)
set_target_properties(ear-version-generator PROPERTIES FOLDER versioning)