add_library(reaper_adm SHARED "dummy.cpp")
add_library(Reaper_adm::reaper_adm ALIAS reaper_adm)

find_package(Boost REQUIRED COMPONENTS filesystem)

file(GLOB_RECURSE cpp_files *.cpp)
file(GLOB_RECURSE c_files *.c)
file(GLOB_RECURSE hpp_files *.hpp)
file(GLOB_RECURSE h_files *.h)
file(GLOB_RECURSE rc_files *.rc)
file(GLOB_RECURSE rc_mac_files *.rc_mac_*)

add_library(reaper_adm_object OBJECT "")
target_sources(reaper_adm_object
    PRIVATE
		${h_files} 
		${hpp_files}
		${c_files} 
		${cpp_files}
		${rc_files}
		${rc_mac_files}
		coordinate_conversion/coord_conv.cpp
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

set(macos_install_dir "UserPlugins")
set(win_install_dir "Plugins")
if(EXISTS "C:/Program Files/REAPER (x64)/reaper.exe")
	# Not running as CI job - install to REAPER Plugins dir
   set(win_install_dir "C:/Program Files/REAPER (x64)/Plugins")
endif()
set(linux_install_dir "share/REAPER/UserPlugins")

install(TARGETS
    reaper_adm
    RUNTIME DESTINATION
        $<$<PLATFORM_ID:Windows>:${win_install_dir}>
    LIBRARY DESTINATION
        $<$<PLATFORM_ID:Darwin>:${macos_install_dir}>$<$<PLATFORM_ID:Linux>:${linux_install_dir}>
    )

install(DIRECTORY ADMPresets DESTINATION $<$<PLATFORM_ID:Windows>:${win_install_dir}>$<$<PLATFORM_ID:Darwin>:${macos_install_dir}>$<$<PLATFORM_ID:Linux>:${linux_install_dir}>)
