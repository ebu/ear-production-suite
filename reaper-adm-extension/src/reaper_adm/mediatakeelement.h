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
    void addChannelOfOriginal(int channelNum) override;
    bool hasChannelOfOriginal(int channelNum) override;
    int channelCount() const override;
    std::vector<int> channelsOfOriginal() const override;
    double startTime() const override;
    std::shared_ptr<adm::AudioObject const> getAudioObject();

private:
    bool hasMediaItem(ReaperAPI const& api, TrackElement& track);

    std::shared_ptr<adm::AudioObject const> object;
    std::vector<int> channelsOfOriginal_;
    MediaItem* referenceItem;
    PCM_source* pcmSource{nullptr};
    double position{0.0};
    MediaItem* createMediaItem(const ReaperAPI &api, TrackElement& track);
    void nameTakeFromOriginalChannels(admplug::ReaperAPI const & api, MediaItem_Take* take);
    void setMediaItemPosition(const ReaperAPI &api, MediaItem* item);
    double getOriginalMediaItemStartOffset(const ReaperAPI &api) const;
    void setMediaItemDuration(const ReaperAPI &api, MediaItem* item);
    void setMediaItemLengthFromDurationProperty(const ReaperAPI &api, MediaItem* item);
    void setMediaItemLengthFromSourceLength(const ReaperAPI &api, MediaItem* item);
    MediaItem_Take* createTake(const ReaperAPI &api, MediaItem* item);
    std::vector<adm::ElementConstVariant> getAdmElements() const override;
};

}
