set(REAPER_ADM_EXTENSION_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)

add_library(reaper_adm SHARED "dummy.cpp")
add_library(Reaper_adm::reaper_adm ALIAS reaper_adm)

find_package(Boost REQUIRED QUIET COMPONENTS filesystem)

set(EXTENSION_SOURCES
	actionmanager.cpp
	admchannel.cpp
	admextraction.cpp
	admimporter.cpp
	admmetadata.cpp
	admtraits.cpp
	admtypehelpers.cpp
	admvstcontrol.cpp
	automationenvelope.cpp
	automationpoint.cpp
	cartesianspeakerlayouts.cpp
	channelindexer.cpp
	channelrouter.cpp
	color.cpp
	commontrackpool.cpp
	common_definition_helper.cpp
	communicators.cpp
	directspeakerautomationelement.cpp
	elementcomparator.cpp
	envelopecreator.cpp
	exportaction.cpp
	exportaction_admsource-admvst.cpp
	exportaction_admsource-earvst.cpp
	exportaction_admsourcescontainer.cpp
	exportaction_dialogcontrol.cpp
	exportaction_parameterprocessing.cpp
	exportaction_pcmsink.cpp
	filehelpers.cpp
	hoaautomationelement.cpp
	importaction.cpp
	importelement.cpp
	importexecutor.cpp
	mediatakeelement.cpp
	mediatrackelement.cpp
	menu.cpp
	nodefactory.cpp
	objectautomationelement.cpp
	parameter.cpp
	parametervaluemapping.cpp
	pcmgroup.cpp
	pcmgroupregistry.cpp
	pcmreader.cpp
	pcmsourcecreator.cpp
	pcmwriter.cpp
	pcmwriterfactory.cpp
	plugin.cpp
	pluginmain.cpp
	pluginregistry.cpp
	pluginsuite.cpp
	pluginsuite_ear.cpp
	pluginsuite_fb360.cpp
	pluginsuite_visr.cpp
	projectelements.cpp
	projectnode.cpp
	projecttree.cpp
	reaperapi.cpp
	reaperapiimpl.cpp
	reaperguid.cpp
	reaperhost.cpp
	track.cpp
	coordinate_conversion/coord_conv.cpp
	progress/importdialog.cpp
	progress/importlistener.cpp
	progress/importprogress.cpp
	)
	
set(EXTENSION_HEADERS
    ${EPS_SHARED_DIR}/helper/nng_wrappers.h
	
	actionmanager.h
	admchannel.h
	admextraction.h
	admimporter.h
	admmetadata.h
	admtraits.h
	admtypehelpers.h
	admvstcontrol.h
	automate.h
	automationenvelope.h
	automationpoint.h
	cartesianspeakerlayouts.h
	channelindexer.h
	channelrouter.h
	color.h
	commontrackpool.h
	common_definition_helper.h
	communicators.h
	directspeakerautomationelement.h
	elementcomparator.h
	envelopecreator.h
	exportaction.h
	exportaction_admsource-admvst.h
	exportaction_admsource-earvst.h
	exportaction_admsourcescontainer.h
	exportaction_dialogcontrol.h
	exportaction_issues.h
	exportaction_parameterprocessing.h
	exportaction_pcmsink.h
	filehelpers.h
	hoaautomationelement.h
	importaction.h
	importelement.h
	importexecutor.h
	mediatakeelement.h
	mediatrackelement.h
	menu.h
	nodefactory.h
	objectautomationelement.h
	parameter.h
	parametervaluemapping.h
	parametised.h
	pcmgroup.h
	pcmgroupregistry.h
	pcmreader.h
	pcmsourcecreator.h
	pcmwriter.h
	pcmwriterfactory.h
	plugin.h
	pluginregistry.h
	pluginsuite.h
	pluginsuite_ear.h
	pluginsuite_fb360.h
	pluginsuite_visr.h
	projectelements.h
	projectnode.h
	projecttree.h
	reaperapi.h
	reaperapiimpl.h
	reaperapivalues.h
	reaperguid.h
	reaperhost.h
	reaper_plugin.h
	reaper_plugin_functions.h
	resource.h
	track.h
	win_mem_debug.h
	win_nonblock_msg.h
	coordinate_conversion/coord_conv.cpp
	progress/importdialog.h
	progress/importlistener.h
	progress/importprogress.h
	progress/importreporter.h
	progress/importstatus.h
	)
	
set(EXTENSION_RESOURCES
	reaper_adm_object.rc
	reaper_adm_object.rc_mac_dlg
	reaper_adm_object.rc_mac_menu
	)

add_library(reaper_adm_object OBJECT "")
target_sources(reaper_adm_object
    PRIVATE
		${EXTENSION_SOURCES} 
		${EXTENSION_HEADERS}
		${EXTENSION_RESOURCES} 
)
	
source_group("Header Files" FILES ${EXTENSION_HEADERS})

source_group("Resources" FILES ${EXTENSION_RESOURCES}) 

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

find_package(WDL REQUIRED QUIET)

target_include_directories(reaper_adm_object
    PRIVATE
    $<TARGET_PROPERTY:WDL::swell,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:IRT::bw64,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:adm,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:nng::nng,INTERFACE_INCLUDE_DIRECTORIES>
	${EPS_SHARED_DIR}
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
