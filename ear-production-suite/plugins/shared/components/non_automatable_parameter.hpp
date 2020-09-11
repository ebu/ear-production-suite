#pragma once

#include "JuceHeader.h"

namespace ear {
namespace plugin {
namespace ui {

/**
 * This template class can adapt any AudioProcessorParameter and mark
 * it as non-automatable.
 *
 * @see AudioParameterFloat, AudioParameterBool, AudioParameterChoice,
 * AudioParameterInt
 *
 * @note
 * According to a post on the Juce forum the isAutomatable() function may get
 * ignored by some hosts. This post is from 2016, so maybe this information is
 * outdated.
 * https://forum.juce.com/t/how-can-i-set-a-certain-parameter-not-automatable/18077/14:w
 */
template <typename BaseParameter>
class NonAutomatedParameter : public BaseParameter {
 public:
  // inherit base constructor
  using BaseParameter::BaseParameter;

  bool isAutomatable() const override { return false; }
};

}  // namespace ui
}  // namespace plugin
}  // namespace ear
