#pragma once
#include <vector>
#include "projectelements.h"
#include "automationpoint.h"
#include "admchannel.h"

namespace admplug {
class Parameter;


class HoaAutomationElement : public HoaAutomation {
public:
    HoaAutomationElement(ADMChannel admChannel, std::shared_ptr<TrackElement> parentTrack, std::shared_ptr<TakeElement> parentTake = nullptr);
    void createProjectElements(PluginSuite &pluginSuite, const ReaperAPI &api) override;
    adm::BlockFormatsConstRange<adm::AudioBlockFormatHoa> blocks() const override;
    double startTime() const override;
    std::shared_ptr<Track> getTrack() const override;
    ADMChannel channel() const override;
    void apply(PluginParameter const& parameter, Plugin const& plugin) const override;
    void apply(TrackParameter const& parameter, Track const& track) const override;
    int channelIndex() const override;

private:
    std::vector<adm::ElementConstVariant> getAdmElements() const override;
    std::shared_ptr<TakeElement> parent;
    std::vector<AutomationPoint> pointsFor(const Parameter &parameter) const;
    ADMChannel admChannel;
};

}
