#pragma once

#include "JuceHeader.h"

namespace ear {
namespace plugin {
namespace ui {

class ReadOnlyAudioParameterInt : public AudioParameterInt {
 public:
  ReadOnlyAudioParameterInt(
      const String& parameterID, const String& name, int minValue, int maxValue,
      int defaultValue, const String& label = String(),
      std::function<String(int value, int maximumStringLength)> stringFromInt =
          nullptr,
      std::function<int(const String& text)> intFromString = nullptr)
      : AudioParameterInt(parameterID, name, minValue, maxValue, defaultValue,
                          label, stringFromInt, intFromString) {
    roResetting = false;
    roValue = convertTo0to1((float)defaultValue);
    roValueInt = defaultValue;
  }

  bool isAutomatable() const override { return false; };

  void internalSetIntAndNotifyHost(int newValue) {
    roValue = convertTo0to1((float)newValue);
    roValueInt = newValue;
    setValueNotifyingHost(roValue);
  }

 private:
  // Can not simply override the set method with blank as it is also used
  // internally. Can not use custom set code as the underlying 'value' var is
  // inaccessible (private in base class). Therefore, best we can do is to just
  // reset it if it is changed.
  void valueChanged(int newValue) override {
    if (!roResetting && newValue != roValueInt) {
      roResetting = true;
      setValueNotifyingHost(roValue);
      roResetting = false;
    }
  }

  float roValue;
  int roValueInt;
  bool roResetting;
};

}  // namespace ui
}  // namespace plugin
}  // namespace ear
