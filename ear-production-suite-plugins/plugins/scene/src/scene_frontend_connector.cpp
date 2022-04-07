#include "scene_frontend_connector.hpp"

#include "speaker_setups.hpp"
#include "helper/move.hpp"
#include "components/overlay.hpp"
#include "helper/iso_lang_codes.hpp"
#include "object_view.hpp"

#include <memory>

namespace ear {
namespace plugin {
namespace ui {

JuceSceneFrontendConnector::JuceSceneFrontendConnector (
    SceneAudioProcessor* processor)
    : SceneFrontendBackendConnector(), p_(processor),
      data_{processor->getData()} {
}

// --- Component Setter

void JuceSceneFrontendConnector::repopulateUIComponents(
    std::shared_ptr<MultipleScenePluginsOverlay> const& multipleScenePluginsOverlay
) {
  multipleScenePluginsOverlay_ = multipleScenePluginsOverlay;
  data_.refresh();
}

//TODO call on message thread
void JuceSceneFrontendConnector::duplicateSceneDetected(bool isDuplicate) {
  if(auto overlay = multipleScenePluginsOverlay_.lock()) {
    overlay->setVisible(isDuplicate);
  }

}

// --- ItemList Management
void JuceSceneFrontendConnector::updateAndCheckPendingElements(
    const communication::ConnectionId& id,
    const proto::InputItemMetadata& item) const {
  auto& pendingElements = p_->getPendingElements();
  auto range = pendingElements.equal_range(item.routing());
  if (range.first != range.second) {
    for (auto el = range.first; el != range.second; ++el) {
      el->second->mutable_object()->set_connection_id(id.string());
    }
    pendingElements.erase(range.first, range.second);

    if (pendingElements.empty()) {
      p_->setStoreFromPending();
    }
  }
}

void JuceSceneFrontendConnector::programmeItemUpdated(ProgrammeStatus status, ProgrammeObject const& item) {
    updateAndCheckPendingElements(item.inputMetadata.connection_id(), item.inputMetadata);
}


}  // namespace ui
}  // namespace plugin
}  // namespace ear
