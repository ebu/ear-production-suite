//
// Created by Richard Bailey on 11/04/2022.
//

#ifndef EAR_PRODUCTION_SUITE_AUTO_MODE_CONTROLLER_HPP
#define EAR_PRODUCTION_SUITE_AUTO_MODE_CONTROLLER_HPP
#include "metadata_listener.hpp"

namespace ear::plugin {
    class Metadata;
    class AutoModeController : public MetadataListener {
    public:
        explicit AutoModeController(Metadata& data);

    private:
        struct IDRoute {
            int route;
            communication::ConnectionId id;
            friend bool operator!=(IDRoute const& lhs,
                                   IDRoute const& rhs) {
                return std::tie(lhs.route, lhs.id) != std::tie(rhs.route, rhs.id);
            }
            friend bool operator==(IDRoute const& lhs,
                                   IDRoute const& rhs) {
                return !(lhs != rhs);
            }
        };

        Metadata& data_;
        std::vector<IDRoute> itemOrder;
        bool on_{true};
        void pushItemOrdering();
        void addOrUpdateItem(const InputItem &item);

        void dataReset(const proto::ProgrammeStore &programmes, const ItemMap &items) override;
        void autoModeChanged(bool enabled) override;
        void itemRemovedFromProgramme(ProgrammeStatus status, const communication::ConnectionId &id) override;
        void inputAdded(InputItem const& item) override;
        void inputUpdated(const InputItem &item, proto::InputItemMetadata const& oldItem) override;
    };
}


#endif //EAR_PRODUCTION_SUITE_AUTO_MODE_CONTROLLER_HPP
