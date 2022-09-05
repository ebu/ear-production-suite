//
// Created by Richard Bailey on 08/04/2022.
//

#ifndef EAR_PRODUCTION_SUITE_PENDING_STORE_HPP
#define EAR_PRODUCTION_SUITE_PENDING_STORE_HPP
#include <map>
#include "metadata_listener.hpp"
#include <adm/adm.hpp>

#include <thread>
#include <mutex>
#include <atomic>

namespace ear::plugin {
    class PendingStore : public MetadataListener {
    public:
        PendingStore(Metadata& metadata,
                     std::string admStr);
        ~PendingStore();
        void populateFromAdm(std::string const& admStr);

    private:
        void inputAdded(InputItem const& item, bool autoModeState) override;
        void inputUpdated(InputItem const& item, proto::InputItemMetadata const& oldItem) override;

        void timeout(int msLimit);
        std::thread timeoutThread;
        std::atomic_bool killThread{ false };

        std::mutex finisher;
        std::atomic_bool finished{false};

        Metadata& data_;
        std::multimap<std::pair<adm::AudioObjectId, adm::AudioTrackUidId>, ear::plugin::proto::ProgrammeElement*> pendingElements_;
        ear::plugin::proto::ProgrammeStore pendingStore_;
        void checkAgainstPendingElements(InputItem const& item);
        void finishPendingElementsSearch();
    };
} // ear::plugin

#endif //EAR_PRODUCTION_SUITE_PENDING_STORE_HPP
