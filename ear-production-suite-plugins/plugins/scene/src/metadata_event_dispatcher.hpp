//
// Created by Richard Bailey on 06/04/2022.
//

#ifndef EAR_PRODUCTION_SUITE_METADATA_EVENT_DISPATCHER_HPP
#define EAR_PRODUCTION_SUITE_METADATA_EVENT_DISPATCHER_HPP
#include "store_metadata.hpp"

namespace ear::plugin {
class MetadataThread;
class MetadataEventDispatcher : public EventDispatcher {
public:
    explicit MetadataEventDispatcher(MetadataThread& thread) : thread_(thread) {}
protected:
    void doDispatch(std::function<void()> event) override;
private:
    MetadataThread& thread_;



};
} // ear::plugin

#endif //EAR_PRODUCTION_SUITE_METADATA_EVENT_DISPATCHER_HPP
