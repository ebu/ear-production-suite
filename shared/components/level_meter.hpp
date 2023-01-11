#pragma once

#include "JuceHeader.h"

#include "level_meter_calculator.hpp"
#include "look_and_feel/colours.hpp"
#include <numeric>

template <typename T>
T clamp(const T& n, const T& lower, const T& upper) {
	return std::max(lower, std::min(n, upper));
}

namespace ear {
namespace plugin {
namespace ui {

class LevelMeter : public Component, private Timer {
public:
	LevelMeter() {
		setColour(backgroundColourId, EarColours::Transparent);
		setColour(outlineColorId, EarColours::Area06dp);
		setColour(highlightColourId, EarColours::Text.withAlpha(Emphasis::high));
		setColour(clippedColourId,
			EarColours::PrimaryHighlight.withAlpha(Emphasis::high));
	}

	enum Orientation { horizontal, vertical };

    enum MeterMode { Individual, PeakChannel };

	void setOrientation(Orientation orientation) { orientation_ = orientation; }

	void setMeter(std::weak_ptr<ear::plugin::LevelMeterCalculator> calculator,
		int channel) {
		calculator_ = calculator;
		if (channels_.size() != 1 ||
            channels_[0] != channel ||
            values_.size() != 1 ||
            meterMode_ != Individual) {
            meterMode_ = Individual;
			channels_ = { channel };
			values_ = { 0.f };
			if (!isTimerRunning()) startTimer(50);
		}
	}

	void setMeter(std::weak_ptr<ear::plugin::LevelMeterCalculator> calculator,
		std::vector<int> &channels) {
		calculator_ = calculator;
		if (channels_ != channels ||
            values_.size() != channels_.size() ||
            meterMode_ != Individual) {
            meterMode_ = Individual;
			channels_ = channels;
			values_ = std::vector<float>(channels_.size(), 0.f);
			if (!isTimerRunning()) startTimer(50);
		}
	}

    void setMeterPeakChannel(std::weak_ptr<ear::plugin::LevelMeterCalculator> calculator,
        std::vector<int> &channels) {
        calculator_ = calculator;
        if (channels_ != channels || values_.size() != 1 ||
            meterMode_ != PeakChannel) {
            meterMode_ = PeakChannel;
            channels_ = channels;
            values_ = std::vector<float>(1, 0.f);
            if (!isTimerRunning()) startTimer(50);
        }
    }

	void timerCallback() override {
		if (auto meter = calculator_.lock()) {
			meter->decayIfNeeded();
            if(meterMode_ == PeakChannel) {
                float maxVal = 0.0;
                for(int i = 0; i < channels_.size(); ++i) {
                    auto level = meter->getLevel(channels_.at(i));
                    if(level > maxVal) {
                        maxVal = level;
                    }
                }
                values_.at(0) = maxVal;
            } else {
                for(int i = 0; i < channels_.size(); ++i) {
                    values_.at(i) = meter->getLevel(channels_.at(i));
                }
            }
			repaint();
		}
	}

	~LevelMeter() {
		stopTimer();
	}

	void paint(Graphics& g) override {
		g.fillAll(findColour(outlineColorId));
		g.setColour(findColour(outlineColorId));
		g.drawRect(getLocalBounds());

		g.setColour(findColour(highlightColourId));
		auto area = getLocalBounds().toFloat();
		area.reduce(outlineWidth_, outlineWidth_);
		float channelHeight =
			area.getHeight() / static_cast<float>(values_.size());
		float channelWidth = area.getWidth() / static_cast<float>(values_.size());
		for (int i = 0; i < values_.size(); ++i) {
			float scalingFactor =
				std::pow(clamp<float>(values_.at(i), 0.f, 1.f), 0.3);
			if (orientation_ == Orientation::horizontal) {
				g.fillRect(area.removeFromTop(channelHeight).removeFromLeft(scalingFactor * area.getWidth()));
			}
			else {
				g.fillRect(area.removeFromLeft(channelWidth)
					.removeFromBottom(scalingFactor * area.getHeight()));
			}
		}

		if (averageBarEnabled_) {
			auto channelsInMeter = static_cast<float>(values_.size());
			float averageValue = std::reduce(values_.begin(), values_.end()) / channelsInMeter;
			float scalingFactorFromAverage = std::pow(clamp<float>(averageValue, 0.f, 1.f), 0.3);
			int averageLevelScaled = static_cast<int>(scalingFactorFromAverage * area.getWidth());
			g.setColour(EarColours::Primary);
			g.setOpacity(1);
			g.fillRect(averageLevelScaled, 0, 5,
				getLocalBounds().getHeight());
		}
	}

	void enableAverage(bool averageEnabled) {
		averageBarEnabled_ = averageEnabled;
		repaint();
	}

	enum ColourIds {
		backgroundColourId = 0x00020001,
		outlineColorId = 0x00020002,
		highlightColourId = 0x00020003,
		clippedColourId = 0x00020004
	};

private:
	std::vector<int> channels_;
	std::vector<float> values_;
    MeterMode meterMode_{ Individual };

	Orientation orientation_ = Orientation::horizontal;
	float outlineWidth_ = 1.f;
	std::weak_ptr<ear::plugin::LevelMeterCalculator> calculator_;
    bool averageBarEnabled_{ false };

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LevelMeter)
};

}  // namespace ui
}  // namespace plugin
}  // namespace ear
