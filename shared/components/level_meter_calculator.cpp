
#include "level_meter_calculator.hpp"

namespace ear {
namespace plugin {


// precalculated attack and release constants
static float ATTACK_8000 = 0.8982300758361816;
static float RELEASE_8000 = 0.9994245171546936;
static float ATTACK_11025 = 0.9293791055679321;
static float RELEASE_11025 = 0.9995824098587036;
static float ATTACK_22050 = 0.9638625383377075;
static float RELEASE_22050 = 0.999791145324707;
static float ATTACK_44100 = 0.9821288585662842;
static float RELEASE_44100 = 0.9998955726623535;
static float ATTACK_48000 = 0.9835786819458008;
static float RELEASE_48000 = 0.9999040365219116;
static float ATTACK_88200 = 0.9910279512405396;
static float RELEASE_88200 = 0.9999477863311768;
static float ATTACK_96000 = 0.9917539358139038;
static float RELEASE_96000 = 0.9999520182609558;
static float ATTACK_192000 = 0.9958682060241699;
static float RELEASE_192000 = 0.9999760389328003;
static float ATTACK_384000 = 0.9979321956634521;
static float RELEASE_384000 = 0.9999880194664001;

// time to decay from 1.0 to 0.0001
static juce::int64 MAX_DECAY_TIME_MS = 2000;

LevelMeterCalculator::LevelMeterCalculator(std::size_t channels,
                                           std::size_t samplerate)
    : lastMeasurement_(juce::Time::currentTimeMillis()) {
  setup(channels, samplerate);
}

void LevelMeterCalculator::setup(std::size_t channels, std::size_t samplerate) {
  std::lock_guard<std::mutex> lock(mutex_);
  channels_ = channels;
  samplerate_ = samplerate;
  lastLevel_.clear();
  lastLevel_.assign(channels_, 0.);
  lastLevelHasSignal_.clear();
  lastLevelHasSignal_.assign(channels_, 0.);
  lastLevelHasClipped_.clear();
  lastLevelHasClipped_.assign(channels_, 0);
  setConstants();
}

void LevelMeterCalculator::process(const AudioBuffer<float>& buffer) {
  std::lock_guard<std::mutex> lock(mutex_);
  lastMeasurement_ = juce::Time::currentTimeMillis();
  for (std::size_t c = 0; c < channels_; ++c) {
    for (std::size_t n = 0; n < buffer.getNumSamples(); ++n) {
        processSample(buffer.getSample(c, n), c);
    }
  }
}

bool LevelMeterCalculator::hasSignal(int channel) {
    if (channel >= 0 && channel < lastLevelHasSignal_.size()) {
        return lastLevelHasSignal_[channel];
    }
    else {
        return false;
    }
}

bool LevelMeterCalculator::thisTrackHasClipped() {
    for (size_t i(0); i < channels_; i++) {
        if (lastLevelHasClipped_[i]) {
            return true;
        }
    }
    return false;
}

bool LevelMeterCalculator::thisChannelHasClipped(int channel) {
    if (lastLevelHasClipped_[channel]) {
        return true;
    }
    else {
        return false;
    }
}

float LevelMeterCalculator::getLevel(std::size_t channel) {
  const std::lock_guard<std::mutex> lock(mutex_);
  if (channel >= 0 && channel < lastLevel_.size()) {
    return lastLevel_[channel];
  } else {
    return 0.f;
  }
}

void LevelMeterCalculator::processSample(float currentValue,
                                         std::size_t channel) {
  auto currentValueAbs = std::abs(currentValue);
  if (currentValueAbs > lastLevel_[channel]) {
    lastLevel_[channel] =
        currentValueAbs - attack_constant_ * (currentValueAbs - lastLevel_[channel]);
  } else {
    lastLevel_[channel] = lastLevel_[channel] * release_constant_;
  }

  if (currentValue < 0.00005 && currentValue > -0.00005) {
      lastLevelHasSignal_[channel] = false;
  }
  else { lastLevelHasSignal_[channel] = true; }

  if (currentValue > 1 || currentValue < -1 || lastLevelHasClipped_[channel]==true) {
      lastLevelHasClipped_[channel] = true;
  }
  else { lastLevelHasClipped_[channel] = false; }
}

void LevelMeterCalculator::decayIfNeeded(int maxDuration) {
  std::lock_guard<std::mutex> lock(mutex_);
  juce::int64 time = juce::Time::currentTimeMillis();
  auto duration = time - lastMeasurement_;
  if (duration < maxDuration) {
    return;
  }
  lastMeasurement_ = time;
  if(duration > MAX_DECAY_TIME_MS) {
    lastLevel_.assign(channels_, 0.f);
  } else {
    std::size_t nSamples = static_cast<std::size_t>(duration / 1000.f * samplerate_);
    for (std::size_t c = 0; c < channels_; ++c) {
      for (std::size_t n = 0; n < nSamples; ++n) {
        processSample(0.f, c);
      }
    }
  }
}

void LevelMeterCalculator::resetClipping(){
    for (std::size_t c = 0; c < channels_; ++c) {
        lastLevelHasClipped_[c] = false;
    }
}

void LevelMeterCalculator::setConstants() {
  switch (samplerate_) {
    case 8000:
      attack_constant_ = ATTACK_8000;
      release_constant_ = RELEASE_8000;
      break;
    case 11025:
      attack_constant_ = ATTACK_11025;
      release_constant_ = RELEASE_11025;
      break;
    case 22050:
      attack_constant_ = ATTACK_22050;
      release_constant_ = RELEASE_22050;
      break;
    case 44100:
      attack_constant_ = ATTACK_44100;
      release_constant_ = RELEASE_44100;
      break;
    case 48000:
      attack_constant_ = ATTACK_48000;
      release_constant_ = RELEASE_48000;
      break;
    case 88200:
      attack_constant_ = ATTACK_88200;
      release_constant_ = RELEASE_88200;
      break;
    case 96000:
      attack_constant_ = ATTACK_96000;
      release_constant_ = RELEASE_96000;
      break;
    case 192000:
      attack_constant_ = ATTACK_192000;
      release_constant_ = RELEASE_192000;
      break;
    case 384000:
      attack_constant_ = ATTACK_384000;
      release_constant_ = RELEASE_384000;
      break;
    default:
      return calcConstants();
  }
}

void LevelMeterCalculator::calcConstants() {
  if (samplerate_ <= 0 || 384000 < samplerate_) {
    attack_constant_ = 0.f;
    release_constant_ = 0.f;
    return;
  }

  const float ATTACK_TIME = 5; /* milliseconds */
  const float ATTACK_THRESHOLD = -2; /* dB */
  const float RELEASE_TIME = 500; /* milliseconds */
  const float RELEASE_THRESHOLD = -20; /* dB */
  const float PRECISION = 1e-6f;

  int nb_smp_attack = static_cast<int>(ATTACK_TIME / 1000.f * samplerate_);
  int nb_smp_release = static_cast<int>(RELEASE_TIME / 1000.f * samplerate_);

  float attack_threshold =
      static_cast<float>(std::pow(10, ATTACK_THRESHOLD / 20.f));
  float release_threshold =
      static_cast<float>(std::pow(10, RELEASE_THRESHOLD / 20.f));

  release_constant_ = std::pow(release_threshold, 1.f / nb_smp_release);

  int smp = 0;
  float current_value;
  float qppm;
  float attack_const = 0.f;
  smp = 0;
  while (nb_smp_attack > smp) {
    smp = 0;
    attack_const += PRECISION;
    qppm = 0;
    while (qppm <= attack_threshold && smp < nb_smp_attack + 1) {
      current_value = static_cast<float>(std::fabs(
          std::sin(2 * MathConstants<float>::pi * 5000.f * smp / samplerate_)));
      if (current_value > qppm) {
        qppm = current_value - attack_const * (current_value - qppm);
      } else {
        qppm = qppm * release_constant_;
      }
      smp++;
    }
  }
  attack_constant_ = attack_const;
}

}  // namespace plugin
}  // namespace ear
