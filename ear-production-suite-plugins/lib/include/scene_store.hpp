//
// Created by Richard Bailey on 24/03/2022.
//

#ifndef EAR_PRODUCTION_SUITE_SCENE_STORE_HPP
#define EAR_PRODUCTION_SUITE_SCENE_STORE_HPP

#include <functional>
#include <mutex>
#include <set>
#include "metadata_listener.hpp"
#include "scene_store.pb.h"

namespace ear::plugin {
class SceneStore : public MetadataListener {
public:
    explicit SceneStore(std::function<void(proto::SceneStore const&)> update);
    void triggerSend();

private:
    proto::SceneStore store_;
    bool changed{true};
    bool sendData{true};
    std::set<std::string> overlappingIds_;
    std::function<void(proto::SceneStore const&)> updateCallback_;

    // MetadataListener interface
    void exporting(bool isExporting) override;
    void dataReset(const proto::ProgrammeStore &programmes, const ItemMap &items) override;
    void programmeSelected(const ProgrammeObjects &objects) override;
    void itemsAddedToProgramme(ProgrammeStatus status, const std::vector<ProgrammeObject> &objects) override;
    void itemRemovedFromProgramme(ProgrammeStatus status, const communication::ConnectionId &id) override;
    void programmeItemUpdated(ProgrammeStatus status, const ProgrammeObject &object) override;
    void inputRemoved(const communication::ConnectionId &id) override;
    void inputUpdated(const InputItem &item, proto::InputItemMetadata const&) override;

    // Implementation details
    void addAvailableInputItemsToSceneStore(ItemMap const& items);
    void addMonitoringItem(proto::InputItemMetadata const& inputItem);
    bool updateMonitoringItem(proto::InputItemMetadata const& inputItem);
    void setMonitoringItemFrom(proto::MonitoringItemMetadata& monitoringItem,
                               proto::InputItemMetadata const& inputItem);
    void addGroup(proto::ProgrammeElement const& element);
    void addToggle(proto::ProgrammeElement const& element);
    void sendUpdate();
    void flagOverlaps();
};
}

#endif //EAR_PRODUCTION_SUITE_SCENE_STORE_HPP
