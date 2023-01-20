
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
static int64_t MAX_DECAY_TIME_MS = 2000;

// signal state thresholds
static float SIGNAL_PRESENCE_THRESHOLD = 0.00005;
static float SIGNAL_CLIPPED_THRESHOLD = 1.0;

// expected block period multiplier
/// This sets the limit for determining no more blocks of audio are being received.
/// It is usually much more than the block period as the blocks are not sent
///  at perfect intervals, and particularly anticipative processing can mean
///  there are actually surges of blocks of audio followed by large gaps.
static float BLOCK_PERIOD_MULTIPLIER = 2.0;

LevelMeterCalculator::LevelMeterCalculator(std::size_t channels,
                                           std::size_t samplerate)
    : lastMeasurement_(juce::Time::currentTimeMillis()) {
    setup(channels, samplerate);
}

void LevelMeterCalculator::setup(std::size_t channels, std::size_t samplerate) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        channels_ = channels;
        samplerate_ = samplerate;
        setConstants();
    }
    resetClipping();
    resetLevels();
}

void LevelMeterCalculator::processForClippingOnly(const AudioBuffer<float>& buffer) {
    std::lock_guard<std::mutex> lock(mutex_);
    size_t channelsToProcess = std::min(static_cast<size_t>(buffer.getNumChannels()), channels_);
    for(std::size_t c = 0; c < channelsToProcess; ++c) {
        if(lastLevelHasClipped_[c]) {
            break; // Already set - no need to check samples
        }
        for(std::size_t n = 0; n < buffer.getNumSamples(); ++n) {
            auto sample(buffer.getSample(c, n));
            auto absSample = std::abs(sample);
            if(absSample > SIGNAL_CLIPPED_THRESHOLD) {
                lastLevelHasClipped_[c] = true;
                break; // Found a sample which clips - no further checking required
            }
        }
    }
}

void LevelMeterCalculator::process(const AudioBuffer<float>& buffer) {
    std::lock_guard<std::mutex> lock(mutex_);
    lastMeasurement_ = juce::Time::currentTimeMillis();
    if(blocksize_ != buffer.getNumSamples()) {
        blocksize_ = buffer.getNumSamples();
        blockPeriodLimitMs_ = ((static_cast<float>(blocksize_) / static_cast<float>(samplerate_)) * 1000.f) * BLOCK_PERIOD_MULTIPLIER;
    }
    size_t channelsToProcess = std::min(static_cast<size_t>(buffer.getNumChannels()), channels_);
    for(std::size_t c = 0; c < channelsToProcess; ++c) {
        bool hasSignal(false);
        bool hasClipped(false);
        for(std::size_t n = 0; n < buffer.getNumSamples(); ++n) {
            auto sample(buffer.getSample(c, n));
            auto absSample = std::abs(sample);
            processSample(sample, c);
            if(absSample > SIGNAL_PRESENCE_THRESHOLD) {
                hasSignal = true;
            }
            if(absSample > SIGNAL_CLIPPED_THRESHOLD || lastLevelHasClipped_[c] == true) {
                hasClipped = true;
            }
        }
        lastLevelHasSignal_[c] = hasSignal;
        lastLevelHasClipped_[c] = hasClipped;
    }
}

bool LevelMeterCalculator::hasSignal(int channel) {
    std::lock_guard<std::mutex> lock(mutex_);
    if(channel >= 0 && channel < lastLevelHasSignal_.size()) {
        return lastLevelHasSignal_[channel];
    } else {
        return false;
    }
}

bool LevelMeterCalculator::thisTrackHasClipped() {
    std::lock_guard<std::mutex> lock(mutex_);
    for(size_t i(0); i < channels_; i++) {
        if(lastLevelHasClipped_[i]) {
            return true;
        }
    }
    return false;
}

bool LevelMeterCalculator::thisChannelHasClipped(int channel) {
    std::lock_guard<std::mutex> lock(mutex_);
    if(lastLevelHasClipped_[channel]) {
        return true;
    } else {
        return false;
    }
}

float LevelMeterCalculator::getLevel(std::size_t channel) {
    const std::lock_guard<std::mutex> lock(mutex_);
    if(channel >= 0 && channel < lastLevel_.size()) {
        return lastLevel_[channel];
    } else {
        return 0.f;
    }
}

void LevelMeterCalculator::processSample(float currentValue,
                                         std::size_t channel) {
    auto currentValueAbs = std::abs(currentValue);
    if(currentValueAbs > lastLevel_[channel]) {
        lastLevel_[channel] =
            currentValueAbs - attack_constant_ * (currentValueAbs - lastLevel_[channel]);
    } else {
        lastLevel_[channel] = lastLevel_[channel] * release_constant_;
    }
}

void LevelMeterCalculator::decayIfNeeded() {
    std::lock_guard<std::mutex> lock(mutex_);
    juce::int64 time = juce::Time::currentTimeMillis();
    auto duration = time - lastMeasurement_;
    if(duration >= blockPeriodLimitMs_) {
        // If here, we've not received any audio in expected period
        lastMeasurement_ = time;
        lastLevelHasSignal_.assign(channels_, false);
        if(duration > MAX_DECAY_TIME_MS) {
            lastLevel_.assign(channels_, 0.f);
        } else {
            std::size_t nSamples = static_cast<std::size_t>(duration / 1000.f * samplerate_);
            for(std::size_t c = 0; c < channels_; ++c) {
                for(std::size_t n = 0; n < nSamples; ++n) {
                    processSample(0.f, c);
                }
            }
        }
    }
}

void LevelMeterCalculator::resetClipping() {
    std::lock_guard<std::mutex> lock(mutex_);
    lastLevelHasSignal_.clear();
    lastLevelHasSignal_.assign(channels_, false);
    lastLevelHasClipped_.clear();
    lastLevelHasClipped_.assign(channels_, false);
}

void LevelMeterCalculator::resetLevels()
{
    std::lock_guard<std::mutex> lock(mutex_);
    lastLevel_.clear();
    lastLevel_.assign(channels_, 0.);
}

void LevelMeterCalculator::setConstants() {
    switch(samplerate_) {
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
    if(samplerate_ <= 0 || 384000 < samplerate_) {
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
    while(nb_smp_attack > smp) {
        smp = 0;
        attack_const += PRECISION;
        qppm = 0;
        while(qppm <= attack_threshold && smp < nb_smp_attack + 1) {
            current_value = static_cast<float>(std::fabs(
                std::sin(2 * MathConstants<float>::pi * 5000.f * smp / samplerate_)));
            if(current_value > qppm) {
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
