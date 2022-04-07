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
    std::shared_ptr<ItemsContainer> const& itemsContainer,
    std::shared_ptr<AutoModeOverlay> const& autoModeOverlay,
    std::shared_ptr<MultipleScenePluginsOverlay> const& multipleScenePluginsOverlay
) {
  itemsContainer->addListener(this);
  autoModeOverlay->addListener(this);
  itemsContainer_ = itemsContainer;
  autoModeOverlay_ = autoModeOverlay;
  multipleScenePluginsOverlay_ = multipleScenePluginsOverlay;
  data_.refresh();
}

//TODO call on message thread
void JuceSceneFrontendConnector::duplicateSceneDetected(bool isDuplicate) {
  if(auto overlay = multipleScenePluginsOverlay_.lock()) {
    overlay->setVisible(isDuplicate);
  }

}

// --- Restore Editor

void JuceSceneFrontendConnector::dataReset(
    proto::ProgrammeStore const& programmeStore,
    ItemMap const& items) {

    auto selectedProgramme = programmeStore.selected_programme_index();
    selectedProgramme = std::max<int>(selectedProgramme, 0);

    if (auto container = itemsContainer_.lock()) {
        container->createOrUpdateViews(items);
    }

    for (int i = 0; i < programmeStore.programme_size(); ++i) {
        auto const &programme = programmeStore.programme(i);
        if (i == selectedProgramme) {
            updateAddItemsContainer({{i, true},
                                     programme,
                                     items});
        }

        if (auto overlay = autoModeOverlay_.lock()) {
            overlay->setVisible(programmeStore.auto_mode());
        }
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

void JuceSceneFrontendConnector::removeFromItemView(
    communication::ConnectionId id) {
  if (auto container = itemsContainer_.lock()) {
    container->removeView(id);
  }
}

// --- Programme Management
void JuceSceneFrontendConnector::programmeSelected(ProgrammeObjects const& objects) {
  updateAddItemsContainer(objects);
}

void JuceSceneFrontendConnector::updateAddItemsContainer(ProgrammeObjects const& objects) {
  if (auto itemsContainer = itemsContainer_.lock()) {
    itemsContainer->themeItemsFor(objects);
  }
}

// ItemsContainer::Listener
void JuceSceneFrontendConnector::addItemsClicked(
    ItemsContainer* container, std::vector<communication::ConnectionId> ids) {
  if(!ids.empty()) {
      data_.addItemsToSelectedProgramme(ids);
  }
}

void JuceSceneFrontendConnector::itemsAddedToProgramme(ProgrammeStatus status, std::vector<ProgrammeObject> const& items) {
  if(status.isSelected) {
    for(auto const& item : items) {
      auto itemsContainer = itemsContainer_.lock();
      if (itemsContainer) {
        itemsContainer->setPresentThemeFor(item.inputMetadata.connection_id());
      }
    }
  }
}

void JuceSceneFrontendConnector::programmeItemUpdated(ProgrammeStatus status, ProgrammeObject const& item) {
    updateAndCheckPendingElements(item.inputMetadata.connection_id(), item.inputMetadata);
}

// AutoModeOverlay::Listener
void JuceSceneFrontendConnector::autoModeChanged(AutoModeOverlay* overlay,
                                                 bool state) {
  data_.setAutoMode(state);
}

void JuceSceneFrontendConnector::inputRemoved(communication::ConnectionId const& id) {
  removeFromItemView(id);
}
void JuceSceneFrontendConnector::autoModeChanged(bool enabled) {
  if(auto overlay = autoModeOverlay_.lock()) {
    overlay->setVisible(enabled);
  }
}
void JuceSceneFrontendConnector::inputAdded(const InputItem& item) {
}

void JuceSceneFrontendConnector::inputUpdated(const InputItem& item) {
    auto itemsContainer = itemsContainer_.lock();
    if (itemsContainer) {
        itemsContainer->createOrUpdateView(item.data);
    }
}
void JuceSceneFrontendConnector::itemRemovedFromProgramme(
    ProgrammeStatus status, const communication::ConnectionId& id) {
  if(status.isSelected) {
    auto itemsContainer = itemsContainer_.lock();
    if (itemsContainer) {
      itemsContainer->setMissingThemeFor(id);
    }
  }
}

}  // namespace ui
}  // namespace plugin
}  // namespace ear
