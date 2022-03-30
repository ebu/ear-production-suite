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
  src/communication/metadata_sender.cpp
  src/communication/direct_speakers_metadata_sender.cpp
  src/communication/hoa_metadata_sender.cpp
  src/communication/input_control_connection.cpp
  src/communication/input_control_socket.cpp
  src/communication/monitoring_control_connection.cpp
  src/communication/monitoring_metadata_receiver.cpp
  src/communication/object_metadata_sender.cpp
  src/communication/scene_command_receiver.cpp
  src/communication/scene_connection_manager.cpp
  src/communication/scene_connection_registry.cpp
  src/communication/scene_metadata_receiver.cpp
  src/direct_speakers_backend.cpp
  src/hoa_backend.cpp
  src/helper/protobuf_utilities.cpp
  src/proto_printers.cpp
  src/log.cpp
  src/binaural_monitoring_audio_processor.cpp
  src/binaural_monitoring_backend.cpp
  src/listener_orientation.cpp
  src/monitoring_audio_processor.cpp
  src/monitoring_backend.cpp
  src/multichannel_convolver.cpp
  src/nng-cpp/error_handling.cpp
  src/object_backend.cpp
  src/item_store.cpp
  src/programme_types.cpp
  src/routing_overlap.cpp
  src/scene_backend.cpp
  src/scene_gains_calculator.cpp
  src/programme_store.cpp
  src/store_metadata.cpp
  src/metadata_listener.cpp
  src/programme_store_adm_populator.cpp
  src/programme_store_adm_serializer.cpp
  src/ui/direct_speakers_frontend_backend_connector.cpp
  src/ui/hoa_frontend_backend_connector.cpp
  src/ui/object_frontend_backend_connector.cpp
  src/ui/scene_frontend_backend_connector.cpp
  src/ui/binaural_monitoring_frontend_backend_connector.cpp
  ${EPS_SHARED_DIR}/helper/common_definition_helper.cpp
  ${EAR_PROTO_SRC} src/scene_store.cpp include/scene_store.hpp)
		
set(EAR_BASE_HEADERS		
	include/binaural_monitoring_audio_processor.hpp
	include/binaural_monitoring_backend.hpp
	include/communication/commands.hpp
	include/communication/common_types.hpp
	include/communication/data_wrapper.hpp
	include/communication/metadata_sender.hpp
	include/communication/direct_speakers_metadata_sender.hpp
	include/communication/hoa_metadata_sender.hpp
	include/communication/input_control_connection.hpp
	include/communication/input_control_socket.hpp
	include/communication/message_buffer.hpp
	include/communication/monitoring_control_connection.hpp
	include/communication/monitoring_metadata_receiver.hpp
	include/communication/object_metadata_sender.hpp
	include/communication/scene_command_receiver.hpp
	include/communication/scene_connection_manager.hpp
	include/communication/scene_connection_registry.hpp
	include/communication/scene_metadata_receiver.hpp
	include/detail/constants.hpp
	include/detail/log_config.hpp
	include/detail/named_type.hpp
	include/detail/spdl_nng_sink.hpp
	include/direct_speakers_backend.hpp
	include/helper/eps_to_ear_metadata_converter.hpp
	include/helper/move.hpp
	include/helper/weak_ptr.hpp
	include/helper/protobuf_utilities.hpp
	include/item_store.hpp
	include/log.hpp
	include/listener_orientation.hpp
	include/monitoring_audio_processor.hpp
	include/monitoring_backend.hpp
	include/multichannel_convolver.hpp
	include/nng-cpp/asyncio.hpp
	include/nng-cpp/buffer.hpp
	include/nng-cpp/dialer.hpp
	include/nng-cpp/enum_bitmask.hpp
	include/nng-cpp/error_handling.hpp
	include/nng-cpp/message.hpp
	include/nng-cpp/nng.hpp
	include/nng-cpp/pipe.hpp
	include/nng-cpp/placeholders.hpp
	include/nng-cpp/protocols/protocol_traits.hpp
	include/nng-cpp/protocols/pub.hpp
	include/nng-cpp/protocols/pull.hpp
	include/nng-cpp/protocols/push.hpp
	include/nng-cpp/protocols/rep.hpp
	include/nng-cpp/protocols/req.hpp
	include/nng-cpp/protocols/socket_options.hpp
	include/nng-cpp/protocols/sub.hpp
	include/nng-cpp/socket_base.hpp
	include/object_backend.hpp
	include/programme_element_visitor.hpp
	include/programme_store.hpp
	include/store_metadata.hpp
	include/metadata_listener.hpp
	include/programme_store_adm_populator.hpp
	include/programme_store_adm_serializer.hpp
	include/programme_types.hpp
	include/proto_printers.hpp
	include/routing_overlap.hpp
	include/scene_backend.hpp
	include/scene_gains_calculator.hpp
	include/ui/binaural_monitoring_frontend_backend_connector.hpp
	include/ui/direct_speakers_frontend_backend_connector.hpp
	include/ui/item_colour.hpp
	include/ui/object_frontend_backend_connector.hpp
	include/ui/scene_frontend_backend_connector.hpp
	include/variable_block_adapter.hpp
	${EPS_SHARED_DIR}/speaker_setups.hpp
	)

set_source_files_properties(${EAR_PROTO_INPUT} PROPERTIES HEADER_ONLY TRUE)
source_group("Header Files" FILES ${EAR_BASE_HEADERS})
target_sources(ear-plugin-base PRIVATE ${EAR_PROTO_INPUT} ${EAR_BASE_HEADERS})

target_link_libraries(ear-plugin-base PUBLIC
  ear
  bear
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
	${EPS_SHARED_DIR}
)
target_compile_features(ear-plugin-base PUBLIC cxx_std_17)
target_compile_definitions(ear-plugin-base PUBLIC BOOST_UUID_FORCE_AUTO_LINK)
set_target_properties(ear-plugin-base PROPERTIES POSITION_INDEPENDENT_CODE ON CXX_EXTENSIONS OFF)
set_target_properties(ear-plugin-base PROPERTIES FOLDER ${IDE_FOLDER_PLUGINS})

include(GenerateExportHeader)
generate_export_header(ear-plugin-base
EXPORT_FILE_NAME ${PROJECT_BINARY_DIR}/ear-plugin-base/export.h
)
