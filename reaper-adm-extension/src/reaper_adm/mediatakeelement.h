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
    bool hasMediaItem(ReaperAPI const& api, TrackElement& track);

    std::shared_ptr<adm::AudioObject const> object;
    std::array<char, 4096> takeNameBuffer;
    std::vector<ADMChannel> admChannels;
    MediaItem* referenceItem;
    PCM_source* pcmSource{nullptr};
    double position{0.0};
    MediaItem* createMediaItem(const ReaperAPI &api, TrackElement& track);
    void nameTakeFromElementName(admplug::ReaperAPI const & api, MediaItem_Take* take);
    void setMediaItemPosition(const ReaperAPI &api, MediaItem* item);
    double getOriginalMediaItemStartOffset(const ReaperAPI &api) const;
    void setMediaItemDuration(const ReaperAPI &api, MediaItem* item);
    void setMediaItemLengthFromDurationProperty(const ReaperAPI &api, MediaItem* item);
    void setMediaItemLengthFromSourceLength(const ReaperAPI &api, MediaItem* item);
    MediaItem_Take* createTake(const ReaperAPI &api, MediaItem* item);
    std::vector<adm::ElementConstVariant> getAdmElements() const override;
};

}
