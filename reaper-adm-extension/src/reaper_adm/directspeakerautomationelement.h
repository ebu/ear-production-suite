#pragma once
#include <vector>
#include "projectelements.h"
#include "automationpoint.h"

namespace admplug {
class Parameter;

class DirectSpeakersAutomationElement : public DirectSpeakersAutomation
{
public:
    DirectSpeakersAutomationElement(ADMChannel admChannel, std::shared_ptr<TrackElement> parentTrack, std::shared_ptr<TakeElement> parentTake = nullptr);
    void createProjectElements(PluginSuite &pluginSuite, const ReaperAPI &api) override;
    adm::BlockFormatsConstRange<adm::AudioBlockFormatDirectSpeakers> blocks() const override;
    double startTime() const override;
    std::shared_ptr<Track> getTrack() const override;
    ADMChannel channel() const override;
    int channelIndex() const override;
    void apply(const PluginParameter &parameter, const Plugin &plugin) const override;
    void apply(const TrackParameter &parameter, const Track &track) const override;

private:
    std::vector<adm::ElementConstVariant> getAdmElements() const override;
    std::vector<AutomationPoint> pointsFor(const Parameter &parameter) const;
    ADMChannel admChannel;
};

}
