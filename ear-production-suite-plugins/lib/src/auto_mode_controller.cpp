//
// Created by Richard Bailey on 11/04/2022.
//

#include <vector>
#include "auto_mode_controller.hpp"
#include "store_metadata.hpp"
using namespace ear::plugin;

/*
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
*/

AutoModeController::AutoModeController(Metadata& data) : data_{data} {}

bool ear::plugin::AutoModeController::addItemIfNecessary(const InputItem & item)
{
  if(!data_.getAutoMode()) return false;
  auto selectedProgrammeIndex = data_.getSelectedProgrammeIndex();
  if(data_.programmeHasElement(selectedProgrammeIndex, item.id)) return false;
  data_.addItemsToSelectedProgramme({ item.id });
  return true;
}

/*
void AutoModeController::pushItemOrdering() {

    std::vector<communication::ConnectionId> newOrder;
    newOrder.reserve(itemOrder.size());
    std::transform(itemOrder.begin(), itemOrder.end(), std::back_inserter(newOrder), [](auto const& element){
        return element.id;
    });
    data_.setElementOrder(0, newOrder);
}
*/
void ear::plugin::AutoModeController::inputAdded(InputItem const & item)
{
  if(addItemIfNecessary(item)) {
    auto selectedProgrammeIndex = data_.getSelectedProgrammeIndex();
    data_.sortProgrammeElements(selectedProgrammeIndex);
  }
}

void AutoModeController::inputUpdated(const InputItem &item, proto::InputItemMetadata const& oldItem)
{
  bool doSort = false;
  if(addItemIfNecessary(item)) doSort = true;
  if(item.data.routing() != oldItem.routing()) doSort = true;
  if(doSort) {
    auto selectedProgrammeIndex = data_.getSelectedProgrammeIndex();
    data_.sortProgrammeElements(selectedProgrammeIndex);
  }
}
