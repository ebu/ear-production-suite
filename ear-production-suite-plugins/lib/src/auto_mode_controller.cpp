//
// Created by Richard Bailey on 11/04/2022.
//

#include <vector>
#include "auto_mode_controller.hpp"
#include "store_metadata.hpp"
using namespace ear::plugin;

namespace {
    template<typename V>
    auto findElement(V& container, std::string const& id) {
        return std::find_if(container.begin(), container.end(), [&id](auto const& idRoute) {
            return idRoute.id == communication::ConnectionId{id};
        });
    }

    template<typename V>
    auto sortRoutes(V& container) {
        std::stable_sort(container.begin(), container.end(),
                         [](auto const& lhs, auto const& rhs) {
            return lhs.route < rhs.route;
        });
    }
}

AutoModeController::AutoModeController(Metadata& data) : data_{data} {}

void AutoModeController::dataReset(const proto::ProgrammeStore &programmes,
                                   const ItemMap &items) {
    on_ = programmes.auto_mode();
    if(on_) {
        itemOrder.clear();
        auto index = programmes.selected_programme_index();
        assert(index == 0);
        auto const& elements = programmes.programme(index).element();
        for(auto const& element : elements) {
            if(element.has_object()) {
                auto id = communication::ConnectionId{element.object().connection_id()};
                itemOrder.push_back({items.at(id).routing(), {element.object().connection_id()}});
            }
        }
        sortRoutes(itemOrder);
        setNewRoutes();
    }
}

void AutoModeController::autoModeChanged(bool enabled) {
    on_ = enabled;
}

void AutoModeController::setNewRoutes() {
    std::vector<communication::ConnectionId> newOrder;
    newOrder.reserve(itemOrder.size());
    std::transform(itemOrder.begin(), itemOrder.end(), std::back_inserter(newOrder), [](auto const& element){
        return element.id;
    });
    data_.setElementOrder(0, newOrder);
}

void AutoModeController::itemsAddedToProgramme(ProgrammeStatus status, const std::vector<ProgrammeObject> &objects) {
    if(on_ && status.isSelected) {
        assert(status.index == 0);
        for(auto const& object : objects) {
            itemOrder.push_back({object.inputMetadata.routing(), object.inputMetadata.connection_id()});
        }
        sortRoutes(itemOrder);
        setNewRoutes();
    }
}

void AutoModeController::itemRemovedFromProgramme(ProgrammeStatus status, const communication::ConnectionId &id) {
    if(on_ && status.isSelected) {
        assert(status.index == 0);
        auto el = findElement(itemOrder, id.string());
        itemOrder.erase(el);
        setNewRoutes();
    }
}

void AutoModeController::inputUpdated(const InputItem &item, proto::InputItemMetadata const& oldItem) {
    if(on_) {
        auto route = item.data.routing();
        if(route != oldItem.routing()) {
            auto el = findElement(itemOrder, oldItem.connection_id());
            if(el != itemOrder.end()) {
                el->route = route;
                sortRoutes(itemOrder);
                setNewRoutes();
            } else {
                data_.addItemsToSelectedProgramme({item.id});
            }
        }
    }
}