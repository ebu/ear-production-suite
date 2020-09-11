#include "admchannel.h"
#include <adm/elements.hpp>

using namespace admplug;

ADMChannel::ADMChannel(std::shared_ptr<const adm::AudioChannelFormat> channelFormat,
                       std::shared_ptr<const adm::AudioPackFormat> packFormat,
                       std::shared_ptr<const adm::AudioTrackUid> uid) : admChannelFormat{ channelFormat }, admPackFormat{ packFormat }, uid{ uid }
{

}

std::shared_ptr<adm::AudioTrackUid const> ADMChannel::trackUid() const
{
    return uid;
}

std::shared_ptr<const adm::AudioChannelFormat> ADMChannel::channelFormat() const
{
    return admChannelFormat;
}

std::shared_ptr<const adm::AudioPackFormat> admplug::ADMChannel::packFormat() const
{
    return admPackFormat;
}

std::string ADMChannel::name() const
{
    return admChannelFormat->get<adm::AudioChannelFormatName>().get();
}

std::string ADMChannel::speakerLabel() const {
    auto directBlocks = admChannelFormat->getElements<adm::AudioBlockFormatDirectSpeakers>();
    if(!directBlocks.empty()) {
        auto firstBlock = directBlocks.front();
        if(firstBlock.has<adm::SpeakerLabels>()) {
            return firstBlock.get<adm::SpeakerLabels>().front().get();
        }
    }
    return std::string{};
}

std::string ADMChannel::formatId() const
{
    return adm::formatId(admChannelFormat->get<adm::AudioChannelFormatId>());
}
