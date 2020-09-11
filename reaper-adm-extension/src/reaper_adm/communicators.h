#pragma once
#include "nng_wrappers.h"

class CommunicatorBase
{
public:
    CommunicatorBase(int samplesPort, int commandPort);
    ~CommunicatorBase();

    virtual void updateInfo() {};

    void setRenderingState(bool state);
    bool getRenderingState() { return renderingState; }

    virtual bool nextFrameAvailable();
    virtual bool copyNextFrame(float* buf, bool bypassAvailabilityCheck = false);

    virtual int getReportedSampleRate() { return 0; }
    virtual int getReportedChannelCount() { return 0; }

    int getSamplesPort() { return samplesPort; }
    int getCommandPort() { return commandPort; }

protected:
    void startSocket();
    void endSocket();

    int commandPort;
    int samplesPort;
    bool renderingState{ false };

    SamplesReceiver samplesSocket;
    CommandSender commandSocket;

    std::shared_ptr<TypedNngMsg<float>> latestBlockMessage;
};

class CommunicatorRegistry
{
private:
    CommunicatorRegistry() {}
    std::vector<std::weak_ptr<CommunicatorBase>> communicators;

public:
    CommunicatorRegistry(CommunicatorRegistry const&) = delete;
    void operator=(CommunicatorRegistry const&)  = delete;

    static CommunicatorRegistry& getInstance();

    template <typename CommunicatorType>
    static std::shared_ptr<CommunicatorType> getCommunicator(int samplesPort, int commandPort) {
        return CommunicatorRegistry::getInstance().findOrCreateCommunicator<CommunicatorType>(samplesPort, commandPort);
    }

    template <typename CommunicatorType>
    std::shared_ptr<CommunicatorType> findOrCreateCommunicator(int samplesPort, int commandPort) {
        int index = 0;
        while(index < communicators.size()) {
            auto communicator = communicators[index].lock();
            if(!communicator) {
                // All usages have been released - no longer exists, remove from vector
                communicators.erase(communicators.begin() + index);
            } else {
                // Do the port numbers match?
                if(communicator->getCommandPort() == commandPort && communicator->getSamplesPort() == samplesPort) {
                    // Is the class of the expected type?
                    auto castClass = std::dynamic_pointer_cast<CommunicatorType>(communicator);
                    if(castClass) {
                        return castClass;
                    }
                }
                index++;
            }
        }
        // If we reach here, we don't have a matching communicator - make one!
        auto communicator = std::make_shared<CommunicatorType>(samplesPort, commandPort);
        communicators.push_back(std::weak_ptr<CommunicatorType>(communicator));
        return communicator;

    }

};
