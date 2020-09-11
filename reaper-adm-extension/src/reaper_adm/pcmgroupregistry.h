#pragma once

#include <memory>
#include <vector>
#include <map>
#include <functional>

class PCM_source;

namespace admplug {
class IPCMGroup;
class PCMGroup;
class TakeElement;

class IPCMGroupRegistry
{
public:
    virtual ~IPCMGroupRegistry() = default;
    virtual void add(std::shared_ptr<TakeElement> take, IPCMGroup const& group) = 0;
    virtual std::vector<IPCMGroup const*> allGroups() const = 0;
    virtual void setTakeSourceFor(const IPCMGroup &group, PCM_source *source) const = 0;
};

class PCMGroupRegistry : public IPCMGroupRegistry
{
public:
    PCMGroupRegistry();
    void add(std::shared_ptr<TakeElement> take, IPCMGroup const& group) override;
    std::vector<IPCMGroup const*> allGroups() const override;
    void setTakeSourceFor(const IPCMGroup &group, PCM_source *source) const override;
private:
    std::map<PCMGroup, std::vector<std::shared_ptr<TakeElement>>> groups;
};

}
