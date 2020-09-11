#pragma once
#include <array>
#include "projectelements.h"
#include <array>

class MediaItem;
class MediaItem_Take;

namespace admplug {

class MediaTakeElement : public TakeElement {
public:
    MediaTakeElement(std::shared_ptr<adm::AudioObject const> obj,
                    std::shared_ptr<TrackElement> = nullptr,
                     MediaItem* referenceItem = nullptr);
    void createProjectElements(PluginSuite &pluginSuite, const ReaperAPI &api) override;
    void setSource(PCM_source * source) override;
    void addChannel(ADMChannel channel) override;
    bool hasChannel(ADMChannel channel) override;
    std::vector<ADMChannel> channels() const override;
    void setChannels(std::vector<ADMChannel> channels) override;
    double startTime() const override;
private:
    std::shared_ptr<adm::AudioObject const> object;
    std::array<char, 4096> takeNameBuffer;
    std::vector<ADMChannel> admChannels;
    MediaItem* referenceItem;
    MediaItem* mediaItem{nullptr};
    MediaItem_Take* mediaItemTake{nullptr};
    PCM_source* pcmSource{nullptr};
    double position{0.0};
    void createMediaItem(const ReaperAPI &api);
    void nameTakeFromElementName(admplug::ReaperAPI const & api);
    void setMediaItemPosition(const ReaperAPI &api);
    double getOriginalMediaItemStartOffset(const ReaperAPI &api) const;
    void setMediaItemDuration(const ReaperAPI &api);
    void setMediaItemLengthFromDurationProperty(const ReaperAPI &api);
    void setMediaItemLengthFromSourceLength(const ReaperAPI &api);
    void createTake(const ReaperAPI &api);
    std::vector<adm::ElementConstVariant> getAdmElements() const override;
};

}
