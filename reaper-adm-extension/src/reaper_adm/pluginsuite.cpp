#include "pluginsuite.h"
#include "plugin.h"
#include "track.h"
#include <cmath>
#include <algorithm>

namespace {
int getHoaOrder(std::vector<admplug::ADMChannel> const &channels){
    auto hoaOrder = 0;
    for(size_t i = 0; i < channels.size(); i = i + 1 ){
        auto cf = channels[i].channelFormat();
        if(cf) {
            auto blocks = cf->getElements<adm::AudioBlockFormatHoa>();
            if(blocks.size() > 0) {
                auto tempOrder = blocks.front().get<adm::Order>().get();
                if(tempOrder > hoaOrder) {
                    hoaOrder = tempOrder;
                }
            }
        }
        auto pf = channels[i].packFormat();
        if(pf) {
            auto cfs = pf->getReferences<adm::AudioChannelFormat>();
            for (auto cf : cfs){
                auto blocks = cf->getElements<adm::AudioBlockFormatHoa>();
                if(blocks.size() > 0) {
                    auto tempOrder = blocks.front().get<adm::Order>().get();
                    if(tempOrder > hoaOrder) {
                        hoaOrder = tempOrder;
                    }
                }

            }
        }
    }
    return hoaOrder;
}
}

admplug::UniqueValueAssigner::UniqueValueAssigner(UniqueValueAssigner::SearchCandidate searchCriteria, int minVal, int maxVal, const ReaperAPI & api) :
    UniqueValueAssigner(std::vector<UniqueValueAssigner::SearchCandidate>{searchCriteria}, minVal, maxVal, api)
{
}

admplug::UniqueValueAssigner::UniqueValueAssigner(std::vector<UniqueValueAssigner::SearchCandidate> searchCriteria, int minVal, int maxVal, const ReaperAPI & api) :
    searchCriteria{ searchCriteria }, offset{ offset }, minPossible{ minVal }, maxPossible{ maxVal }
{
    // Must have search criteria
    assert(searchCriteria.size() > 0);

    updateAvailableValues(api);
}

admplug::UniqueValueAssigner::~UniqueValueAssigner()
{
}

void admplug::UniqueValueAssigner::updateAvailableValues(const ReaperAPI & api)
{
    std::size_t range = static_cast<std::size_t>(std::max<int>(0, maxPossible - minPossible + 1));
    std::vector<bool> valueUsage(range, false);

    int trackCount = api.CountTracks(nullptr);

    for(auto& sc : searchCriteria) {
        for(int i = 0; i < trackCount; i++) {
            auto track = TrackInstance{ api.GetTrack(nullptr, i), api };
            auto plugin = track.getPlugin(sc.pluginName);
            if(plugin) {
                PluginInstance *pluginInst = dynamic_cast<PluginInstance*>(plugin.get());
                if(pluginInst) {
                    auto consumedVals = sc.determineUsedValues(*pluginInst);
                    for(auto consumedVal : consumedVals) {
                        auto index = indexFromVal(consumedVal);
                        if(index) {
                            valueUsage[*index] = true;
                        }
                    }
                }
            }
        }
    }

    availableValues.clear();
    for(std::size_t i = 0; i < range; i++) {
        if(!valueUsage[i]) {
            availableValues.push_back(static_cast<int>(i) + minPossible);
        }
    }
}

std::optional<int> admplug::UniqueValueAssigner::getNextAvailableValue(int numberOfConsectivesRequired)
{
    std::optional<int> freeValue;

    std::size_t startingIndex = 0;
    int consecutiveCount = 0;

    for(std::size_t i = 0; i < availableValues.size(); i++) {
        if(i == 0 || availableValues[i - 1] != availableValues[i] - 1) {
            consecutiveCount = 0;
            startingIndex = i;
        }
        consecutiveCount++;
        if(consecutiveCount == numberOfConsectivesRequired) {
            freeValue = availableValues[startingIndex];
            using DiffT = decltype(availableValues.begin())::difference_type;
            auto iteratorOffset = static_cast<DiffT>(startingIndex);
            availableValues.erase(availableValues.begin() + iteratorOffset, availableValues.begin() + iteratorOffset + consecutiveCount);
            break;
        }
    }

    return freeValue;
}

std::optional<std::size_t> admplug::UniqueValueAssigner::indexFromVal(int realValue)
{
    if(realValue >= minPossible && realValue <= maxPossible) {
        return std::max<int>(0, realValue - minPossible);
    }
    return {};
}

std::vector<admplug::ADMChannel> admplug::PluginSuite::reorderAndFilter(std::vector<ADMChannel> const& channels, ReaperAPI const & api)
{
    std::vector<ADMChannel> newChannelOrder;
    if(channels.empty()) return newChannelOrder;
    auto packFormat = channels.at(0).packFormat();
    if(!packFormat) return channels;
    auto typeDefinition = packFormat->get<adm::TypeDescriptor>();

    // This sorts by Order and by Degree (i.e, ACN, which most plugins expect and which the common definitions should be ordered in)
    // It fills missing channels with ADMChannels with null contents.
    if(typeDefinition == adm::TypeDefinition::HOA){

        auto hoaOrder = getHoaOrder(channels); // Gets highest value for Order parameter in `channels`

        int channelsInOrder = (hoaOrder + 1) * (hoaOrder + 1);

        std::vector<ADMChannel> newChannelOrder(channelsInOrder, ADMChannel{ nullptr, nullptr, packFormat, nullptr });

        for(auto channel : channels){
            if(channel.channelFormat()) {
                auto blocks = channel.channelFormat()->getElements<adm::AudioBlockFormatHoa>();
                if(blocks.size() > 0) {
                    auto block = blocks.front();
                    if(block.has<adm::Degree>() && block.has<adm::Order>()) {
                        auto degree = block.get<adm::Degree>().get();
                        auto order = block.get<adm::Order>().get();
                        if(order >= 0) {
                            if(degree >= -order && degree <= order) {
                                int channelsInPrevOrder = (order) * (order); // (would be order+1 squared if for this order;
                                int channelIndex = order + degree + channelsInPrevOrder;
                                if(channelIndex >= 0 && channelIndex < newChannelOrder.size()) {
                                    if(newChannelOrder[channelIndex].channelFormat()) {
                                        // TODO: Already have this HOA channel - need to warn!
                                        assert(false);
                                    } else {
                                        newChannelOrder[channelIndex] = channel;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        return newChannelOrder;
    }

    else {
        // Generalised for any common definition type
        auto tdId = typeDefinition.get();
        auto pfId = packFormat->get<adm::AudioPackFormatId>().get<adm::AudioPackFormatIdValue>().get();
        auto packFormatData = AdmCommonDefinitionHelper::getSingleton()->getPackFormatData(tdId, pfId);

        if(packFormatData) {
        // These must be in the common definition order for the ADM Export Source VST (and EAR DirectSpeakers Input if DS)!!
            for(auto const& targetChannelFormat : packFormatData->relatedChannelFormats) {
                int targetFoundCount = 0;
                for(auto const& admChannel : channels) {
                    auto thisChannelFormat = admChannel.channelFormat();
                    if(thisChannelFormat) {
                        auto thisId = thisChannelFormat->get<adm::AudioChannelFormatId>().get<adm::AudioChannelFormatIdValue>().get();
                        if(thisId == targetChannelFormat->id) {
                            if(targetFoundCount == 0) {
                                newChannelOrder.push_back(admChannel);
                            }
                            targetFoundCount++;
                        }
                    }
                }
                if(targetFoundCount == 0) {
                    // No track found - do a blank.
                    newChannelOrder.push_back(ADMChannel{ nullptr, nullptr, packFormat, nullptr });
                } else if(targetFoundCount > 1) {
                    // TODO: Already have this channel - need to warn!
                    assert(false);
                }

            }
            assert(newChannelOrder.size() == packFormatData->relatedChannelFormats.size()); // ...just to check the logic

            return newChannelOrder;
        }
    }

    newChannelOrder = channels;
    return newChannelOrder;
}

std::optional<std::string> admplug::PluginSuite::getSpatialisationPluginNameFor(adm::TypeDescriptor typeDescriptor)
{
    if(typeDescriptor == adm::TypeDefinition::OBJECTS)
        return getSpatialisationPluginNameForObjects();
    if(typeDescriptor == adm::TypeDefinition::DIRECT_SPEAKERS)
        return getSpatialisationPluginNameForDirectSpeakers();
    if(typeDescriptor == adm::TypeDefinition::HOA)
        return getSpatialisationPluginNameForHoa();
    if(typeDescriptor == adm::TypeDefinition::BINAURAL)
        return getSpatialisationPluginNameForBinaural();
    if(typeDescriptor == adm::TypeDefinition::MATRIX)
        return getSpatialisationPluginNameForMatrix();
    return std::optional<std::string>();
}
