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
    searchCriteria{ searchCriteria }, minPossible{ minVal }, maxPossible{ maxVal }
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
