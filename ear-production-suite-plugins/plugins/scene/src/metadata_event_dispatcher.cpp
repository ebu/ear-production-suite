//
// Created by Richard Bailey on 06/04/2022.
//

#include "metadata_event_dispatcher.hpp"
#include "metadata_thread.hpp"
namespace ear::plugin {

void MetadataEventDispatcher::doDispatch(std::function<void()> event) {
    thread_.post(std::move(event));
}

}