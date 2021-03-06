add_executable(testreaper_adm "")
target_sources(testreaper_adm
    PRIVATE
       blockbuilders.cpp
       pcmtests.cpp
       nodetests.cpp
       projecttreetests.cpp
       outertests.cpp
       pluginsuitetests.cpp
       mediatrackelementtests.cpp
       mediatakeelementtests.cpp
       objectautomationelementtests.cpp
       envelopetests.cpp
       guidtests.cpp
       earsuitetests.cpp
       maptests.cpp
       tempdir.cpp
       valueassignertests.cpp
       coordinateconversiontests.cpp
       automationpointtests.cpp)


if(MSVC)
target_compile_options(testreaper_adm 
  PRIVATE
    "/bigobj")
endif()

find_package(Boost REQUIRED COMPONENTS filesystem)

target_link_libraries(testreaper_adm
    PRIVATE
        reaper_adm_dependencies
        Catch::catch2
        Boost::filesystem)

target_include_directories(testreaper_adm
  PRIVATE
    $<TARGET_PROPERTY:Reaper_adm::reaper_adm,INCLUDE_DIRECTORIES>)
	
set_property(TARGET testreaper_adm PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "../../../test/reaper_adm/")

add_test(
    NAME reaper_admtest
    COMMAND testreaper_adm
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
	

add_executable(testacceptreaper_adm "")

target_sources(testacceptreaper_adm
    PRIVATE
    acceptancetests.cpp
    tempdir.cpp)

target_link_libraries(testacceptreaper_adm
    PRIVATE
        reaper_adm_dependencies
        Catch::catch2
        Boost::filesystem)

if(MSVC)
target_compile_options(testacceptreaper_adm
  PRIVATE
    "/bigobj")
endif()

target_include_directories(testacceptreaper_adm
  PRIVATE
    $<TARGET_PROPERTY:Reaper_adm::reaper_adm,INCLUDE_DIRECTORIES>)
	
set_property(TARGET testacceptreaper_adm PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "../../../test/reaper_adm/")

add_test(
    NAME reaper_adm_acceptance_test
    COMMAND testacceptreaper_adm
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

option(REAPER_ADM_BUILD_BENCHMARKS "Build benchmarks" OFF)
if(REAPER_ADM_BUILD_BENCHMARKS)
add_executable(benchmark_automation_simplification "")
target_sources(benchmark_automation_simplification
    PRIVATE
      benchmark_automation_simplification.cpp)
target_link_libraries(benchmark_automation_simplification
    PRIVATE
    reaper_adm_dependencies)
target_include_directories(benchmark_automation_simplification
  PRIVATE
    $<TARGET_PROPERTY:Reaper_adm::reaper_adm,INCLUDE_DIRECTORIES>)
target_compile_features(benchmark_automation_simplification PRIVATE cxx_std_20)
endif()
