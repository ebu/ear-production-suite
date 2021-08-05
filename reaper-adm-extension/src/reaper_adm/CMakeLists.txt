add_library(reaper_adm SHARED "dummy.cpp")
add_library(Reaper_adm::reaper_adm ALIAS reaper_adm)

find_package(Boost REQUIRED COMPONENTS filesystem)

file(GLOB_RECURSE cpp_files "*.cpp")
file(GLOB_RECURSE c_files "*.c")
file(GLOB_RECURSE hpp_files "*.hpp")
file(GLOB_RECURSE h_files "*.h")
file(GLOB_RECURSE rc_files "*.rc")
file(GLOB_RECURSE rc_mac_files "*.rc_mac_*")

list(FILTER cpp_files EXCLUDE REGEX "\\._+") 
list(FILTER c_files EXCLUDE REGEX "\\._+")
list(FILTER hpp_files EXCLUDE REGEX "\\._+") 
list(FILTER h_files EXCLUDE REGEX "\\._+")
list(FILTER rc_files EXCLUDE REGEX "\\._+") 
list(FILTER rc_mac_files EXCLUDE REGEX "\\._+")


add_library(reaper_adm_object OBJECT "")
target_sources(reaper_adm_object
    PRIVATE
		${h_files} 
		${hpp_files}
		${c_files} 
		${cpp_files}
		${rc_files}
		${rc_mac_files}
)
	
source_group("Headers" FILES ${h_files} ${hpp_files})

source_group("Resources" FILES ${rc_files} ${rc_mac_files}) 

target_compile_definitions(reaper_adm_object
    PUBLIC $<TARGET_PROPERTY:WDL::swell,INTERFACE_COMPILE_DEFINITIONS>)

set_target_properties(reaper_adm_object PROPERTIES POSITION_INDEPENDENT_CODE TRUE)

add_library(reaper_adm_dependencies INTERFACE)

target_include_directories(reaper_adm_dependencies
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<TARGET_PROPERTY:adm,INCLUDE_DIRECTORIES>
)

find_package(WDL REQUIRED)

target_include_directories(reaper_adm_object
    PRIVATE
    $<TARGET_PROPERTY:WDL::swell,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:IRT::bw64,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:adm,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:nng::nng,INTERFACE_INCLUDE_DIRECTORIES>
    )

target_link_libraries(reaper_adm_dependencies
    INTERFACE
      $<TARGET_OBJECTS:reaper_adm_object>
      IRT::bw64
      adm
      WDL::swell
      Boost::filesystem
      nng::nng
    )

add_dependencies(reaper_adm_dependencies reaper_adm_object)

set_target_properties(reaper_adm
    PROPERTIES PREFIX "")

target_compile_features(reaper_adm_dependencies
    INTERFACE
        cxx_std_17)

target_compile_features(reaper_adm_object
    PRIVATE
        cxx_std_17)

target_link_libraries(reaper_adm
    PRIVATE
      reaper_adm_dependencies
      )

if(CMAKE_GENERATOR STREQUAL Xcode)
add_custom_command(
    TARGET reaper_adm 
    PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory        
        ${CMAKE_CURRENT_BINARY_DIR}/ear-production-combined.build
        ${CMAKE_CURRENT_BINARY_DIR}/reaper_adm.build
)
endif()


set(win_install_dir "${EPS_EXTENSION_INSTALL_PREFIX}UserPlugins")
set(macos_install_dir "${EPS_EXTENSION_INSTALL_PREFIX}UserPlugins")
set(linux_install_dir "share/REAPER/UserPlugins")

install(TARGETS
    reaper_adm
    RUNTIME DESTINATION
        $<$<PLATFORM_ID:Windows>:${win_install_dir}>
    LIBRARY DESTINATION
        $<$<PLATFORM_ID:Darwin>:${macos_install_dir}>$<$<PLATFORM_ID:Linux>:${linux_install_dir}>
    )

if(WIN32)
	install(FILES $<TARGET_PDB_FILE:reaper_adm>
			DESTINATION
				${win_install_dir}
			OPTIONAL)
endif()

install(DIRECTORY ADMPresets COMPONENT Extension DESTINATION $<$<PLATFORM_ID:Windows>:${win_install_dir}>$<$<PLATFORM_ID:Darwin>:${macos_install_dir}>$<$<PLATFORM_ID:Linux>:${linux_install_dir}>)
