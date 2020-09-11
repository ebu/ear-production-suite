#include <algorithm>
#include "pcmgroupregistry.h"
#include "pcmgroup.h"
#include "projectelements.h"

using namespace admplug;
using WrappedNodeRef = std::reference_wrapper<TakeElement>;

PCMGroupRegistry::PCMGroupRegistry()
{

}

void PCMGroupRegistry::add(std::shared_ptr<TakeElement> take, IPCMGroup const& group)
{
    auto [itemIt, success] = groups.insert(std::make_pair(PCMGroup{group}, std::vector< std::shared_ptr<TakeElement>>({ take })));
    if(!success) {
       itemIt->second.emplace_back(take);
    }
}

std::vector<IPCMGroup const*> PCMGroupRegistry::allGroups() const
{
    std::vector<IPCMGroup const*> pcmGroups;
    std::transform(groups.begin(), groups.end(), std::back_inserter(pcmGroups),
                   [](auto& kvPair) {
        return &kvPair.first;
    });
    return pcmGroups;
}

void PCMGroupRegistry::setTakeSourceFor(IPCMGroup const& group, PCM_source* source) const
{
    if(auto groupRefPairIt = groups.find(group);
            groupRefPairIt != groups.end()) {
        for(auto element : groupRefPairIt->second) {
            element->setSource(source);
        }
    }
}
