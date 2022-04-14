//
// Created by Richard Bailey on 13/04/2022.
//

#ifndef EAR_PRODUCTION_SUITE_RESTORED_PENDING_STORE_HPP
#define EAR_PRODUCTION_SUITE_RESTORED_PENDING_STORE_HPP
#include <unordered_set>
#include "metadata.hpp"
#include "metadata_listener.hpp"

namespace ear::plugin {
    class Metadata;

    class RestoredPendingStore : public MetadataListener {
    public:
        explicit RestoredPendingStore(Metadata& metadata);
        void start(proto::ProgrammeStore restored,
                   std::pair<proto::ProgrammeStore, ItemMap> const& currentStores);

    private:
        void inputUpdated(const InputItem &item, const proto::InputItemMetadata &oldItem) override;
        void tryRestore();

    private:
        Metadata& data_;
        proto::ProgrammeStore store_;
        bool on_{false};
        std::unordered_set<std::string> missingInputs;
    };

}

#endif //EAR_PRODUCTION_SUITE_RESTORED_PENDING_STORE_HPP
