//
// Created by Richard Bailey on 11/04/2022.
//

#include <vector>
#include "auto_mode_controller.hpp"
#include "store_metadata.hpp"
using namespace ear::plugin;

AutoModeController::AutoModeController(Metadata& data) : data_{data} {}

void ear::plugin::AutoModeController::addItemIfNecessary(const InputItem & item)
{
  if(data_.getAutoMode()) {
    auto selectedProgrammeId = data_.getSelectedProgrammeId();
    if(!data_.programmeHasElement(selectedProgrammeId, item.id)) {
      data_.addItemsToSelectedProgramme({ item.id });
    }
  }
}

void ear::plugin::AutoModeController::inputAdded(InputItem const & item)
{
  addItemIfNecessary(item);
}

void AutoModeController::inputUpdated(const InputItem &item, proto::InputItemMetadata const& oldItem)
{
  addItemIfNecessary(item);
}
