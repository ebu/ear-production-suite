//
// Created by Richard Bailey on 08/04/2022.
//

#ifndef EAR_PRODUCTION_SUITE_PENDING_STORE_HPP
#define EAR_PRODUCTION_SUITE_PENDING_STORE_HPP
#include <map>
#include "metadata_listener.hpp"

namespace ear::plugin {
    class PendingStore : public MetadataListener {
    public:
        PendingStore(Metadata& metadata,
                     std::string admStr,
                     std::vector<uint32_t> mappings);
        void populateFromAdm(std::string const& admStr, std::vector<uint32_t> const& mappings);
        void programmeItemUpdated(ProgrammeStatus status, ProgrammeObject const& item) override;
    private:
        bool finished{false};
        Metadata& data_;
        std::multimap<int, ear::plugin::proto::ProgrammeElement*> pendingElements_;
        ear::plugin::proto::ProgrammeStore pendingStore_;
    };
} // ear::plugin

#endif //EAR_PRODUCTION_SUITE_PENDING_STORE_HPP