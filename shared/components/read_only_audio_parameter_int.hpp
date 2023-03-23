#pragma once

#include "JuceHeader.h"

class ReadOnlyAudioParameterInt : public AudioParameterInt, private Timer {
 public:
  ReadOnlyAudioParameterInt(
      const String& parameterID, const String& name, int minValue, int maxValue,
      int defaultValue, const String& label = String(),
      std::function<String(int value, int maximumStringLength)> stringFromInt =
          nullptr,
      std::function<int(const String& text)> intFromString = nullptr)
      : AudioParameterInt(parameterID, name, minValue, maxValue, defaultValue,
                          label, stringFromInt, intFromString) {
    roValue = convertTo0to1((float)defaultValue);
    roValueInt = defaultValue;
  }

  ~ReadOnlyAudioParameterInt() {
      if(isTimerRunning()) {
          stopTimer();
      }
  }

  bool isAutomatable() const override {
      return false;
  }

  void internalSetIntAndNotifyHost(int newValue) {
    roValue = convertTo0to1((float)newValue);
    roValueInt = newValue;
    beginChangeGesture();
    setValueNotifyingHost(roValue);
    endChangeGesture();
    startTimer(1); // yes, this is super hacky, but since the upgrade to JUCE 6, the value isn't getting set if it is being set as a consequence of another parameter change! 
  }

 private:
  // Can not simply override the set method with blank as it is also used
  // internally. Can not use custom set code as the underlying 'value' var is
  // inaccessible (private in base class). Therefore, best we can do is to just
  // reset it if it is changed.
  void valueChanged(int newValue) override {
    if (newValue != roValueInt) {
        startTimer(1);
    }
  }

  void timerCallback() override {
      stopTimer();
      beginChangeGesture();
      setValueNotifyingHost(roValue);
      endChangeGesture();
  }

  float roValue;
  int roValueInt;
};
