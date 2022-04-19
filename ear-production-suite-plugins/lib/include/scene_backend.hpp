#pragma once

#include "communication/message_buffer.hpp"
#include "communication/scene_command_receiver.hpp"
#include "communication/scene_metadata_receiver.hpp"
#include "communication/scene_connection_manager.hpp"
#include "store_metadata.hpp"
#include "scene_store.hpp"
#include "log.hpp"
#include "ear-plugin-base/export.h"
#include <mutex>
#include <set>

namespace ear {
namespace plugin {

/**
 * @brief Coordinates plugin communication and consolidates scene metadata
 *
 * Both input and output plugins register themselves with the scene master.
 * Input plugins send metadata to the scene master, which compiles this
 * fragments into a full audio scene. This audio scene is then passed on to the
 * monitoring plugins to render the audio signals.
 *
 * Besides being a registry and broker for the other plugins, the scene master
 * is also responsible to manage all informationt that needs to be globally
 * consistent. Examples include audio track allocations/mappings, connections
 * ids, etc.
 *
 * Furthermore, from a UI/user perspective, it might be the place to edit
 * metadata that influences the whole audio scene. Examples might include
 * interactive elements, audio programme selection and creation, etc.
 *
 * This class is the interface between the actual plugin implentation (include
 * UI) and the "backend". The public interface is currently very minimal and
 * will be amended as needed to
 *   - communicate with the UI bidirectional (Maybe using a (wrapped?) JUCE
 * ValueTree or similar) bi
 *   - trigger forwarding the current audio scene state to the monitorings.
 */
class SceneBackend {
 public:
  EAR_PLUGIN_BASE_EXPORT SceneBackend(Metadata& data);
  EAR_PLUGIN_BASE_EXPORT ~SceneBackend();
  SceneBackend(const SceneBackend &) = delete;
  SceneBackend(SceneBackend &&) = delete;
  SceneBackend &operator=(SceneBackend &&) = delete;
  SceneBackend &operator=(const SceneBackend &) = delete;
  void triggerMetadataSend();
  void triggerMetadataSend(proto::SceneStore const& store);
  void setup();

 private:
  void onConnectionEvent(communication::SceneConnectionManager::Event,
                         communication::ConnectionId id);

  std::mutex mutex_;
  std::shared_ptr<spdlog::logger> logger_;
  Metadata& data_;
  std::shared_ptr<SceneStore> sceneStore_;
  std::set<communication::ConnectionId> previousScene_;
  std::set<std::string> overlappingIds_;
  communication::SceneConnectionManager connectionManager_;
  nng::PubSocket metadataSender_;
  communication::SceneCommandReceiver commandReceiver_;
  communication::SceneMetadataReceiver metadataReceiver_;
};

}  // namespace plugin
}  // namespace ear
