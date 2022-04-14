//
// Created by Richard Bailey on 13/04/2022.
//

#include "restored_pending_store.hpp"
#include "store_metadata.hpp"

namespace ear::plugin {
    RestoredPendingStore::RestoredPendingStore(Metadata &metadata) : data_{metadata} {}

    void RestoredPendingStore::start(proto::ProgrammeStore restored,
                                     const std::pair<proto::ProgrammeStore, ItemMap> &currentStores) {
        store_ = std::move(restored);
        auto const& currentItems = currentStores.second;
        for(auto const& programme : store_.programme()) {
            for(auto const& item : programme.element()) {
                if(item.has_object()) {
                    auto id = item.object().connection_id();
                    assert(communication::ConnectionId{id}.isValid());
                    auto found = currentItems.find(id) != currentItems.end();
                    if(!found) {
                        missingInputs.insert(id);
                    }
                }
            }
        }
        on_ = true;
        tryRestore();
    }

    void RestoredPendingStore::inputUpdated(const InputItem &item, const proto::InputItemMetadata &oldItem) {
        if(on_) {
            if (auto missingInput = missingInputs.find(item.id.string());
                    missingInput != missingInputs.end()) {
                missingInputs.erase(missingInput);
                tryRestore();
            }
        }
    }

    void RestoredPendingStore::tryRestore() {
        if (missingInputs.empty()) {
            on_ = false;
            data_.setStore(store_);
        }

    }
}