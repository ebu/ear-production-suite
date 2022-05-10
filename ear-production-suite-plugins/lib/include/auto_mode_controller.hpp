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
        Metadata& data_;
        void inputAdded(InputItem const& item, bool autoModeState) override;
    };
}


#endif //EAR_PRODUCTION_SUITE_AUTO_MODE_CONTROLLER_HPP
