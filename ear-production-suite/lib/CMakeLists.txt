set(EAR_PROTO_INPUT
  messages/common_commands.proto
  messages/common_types.proto
  messages/connection_commands.proto
  messages/input_item_metadata.proto
  messages/monitoring_item_metadata.proto
  messages/scene_store.proto
  messages/type_metadata.proto
  messages/programme_store.proto)

protobuf_generate_cpp(EAR_PROTO_SRC EAR_PROTO_HDR ${EAR_PROTO_INPUT})

add_library(ear-plugin-base STATIC
  src/communication/commands.cpp
  src/communication/direct_speakers_metadata_sender.cpp
  src/communication/input_control_connection.cpp
  src/communication/monitoring_control_connection.cpp
  src/communication/monitoring_metadata_receiver.cpp
  src/communication/object_metadata_sender.cpp
  src/communication/scene_command_receiver.cpp
  src/communication/scene_connection_manager.cpp
  src/communication/scene_connection_registry.cpp
  src/communication/scene_metadata_receiver.cpp
  src/direct_speakers_backend.cpp
  src/helper/protobuf_utilities.cpp
  src/log.cpp
  src/monitoring_audio_processor.cpp
  src/monitoring_backend.cpp
  src/multichannel_convolver.cpp
  src/nng-cpp/error_handling.cpp
  src/object_backend.cpp
  src/programme_types.cpp
  src/scene_backend.cpp
  src/scene_gains_calculator.cpp
  src/programme_store_adm_populator.cpp
  src/programme_store_adm_serializer.cpp
  src/ui/direct_speakers_frontend_backend_connector.cpp
  src/ui/object_frontend_backend_connector.cpp
  src/ui/scene_frontend_backend_connector.cpp
  ${EAR_PROTO_SRC}
        include/proto_printers.hpp src/proto_printers.cpp)

set_source_files_properties(${EAR_PROTO_INPUT} PROPERTIES HEADER_ONLY TRUE)
target_sources(ear-plugin-base PRIVATE ${EAR_PROTO_INPUT})


target_link_libraries(ear-plugin-base PUBLIC
  ear
  Boost::boost
  protobuf::libprotobuf
  nng::nng
  spdlog::spdlog
  bw64
  adm
  Eigen3::Eigen)
if(SPDLOG_FMT_EXTERNAL)
  target_link_libraries(ear-plugin-base PUBLIC fmt::fmt)
endif()

target_include_directories(ear-plugin-base PUBLIC 
  # Headers used from source/build location:
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}>  # config.h / export.h
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>  # protobuf
)
target_compile_features(ear-plugin-base PUBLIC cxx_std_14)
target_compile_definitions(ear-plugin-base PUBLIC BOOST_UUID_FORCE_AUTO_LINK)
set_target_properties(ear-plugin-base PROPERTIES POSITION_INDEPENDENT_CODE ON CXX_EXTENSIONS OFF)

include(GenerateExportHeader)
generate_export_header(ear-plugin-base
EXPORT_FILE_NAME ${PROJECT_BINARY_DIR}/ear-plugin-base/export.h
)
