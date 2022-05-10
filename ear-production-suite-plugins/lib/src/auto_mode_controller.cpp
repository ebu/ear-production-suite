//
// Created by Richard Bailey on 11/04/2022.
//

#include <vector>
#include "auto_mode_controller.hpp"
#include "store_metadata.hpp"
using namespace ear::plugin;

AutoModeController::AutoModeController(Metadata& data) : data_{data} {}

void ear::plugin::AutoModeController::inputAdded(InputItem const & item, bool autoModeEnabled)
{
  if(autoModeEnabled) {
    data_.addItemsToSelectedProgramme({ item.id });
  }
}

