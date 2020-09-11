#include "communicators.h"

CommunicatorBase::CommunicatorBase(int samplesPort, int commandPort) : samplesPort{ samplesPort }, commandPort{ commandPort }
{
    startSocket();
}

CommunicatorBase::~CommunicatorBase()
{
    endSocket();
}

void CommunicatorBase::startSocket() {
    auto resOpen = commandSocket.open();
    assert(resOpen == 0);
    auto resDial = commandSocket.dial(commandPort);
    assert(resDial == 0);
    resOpen = samplesSocket.open();
    assert(resOpen == 0);
    resDial = samplesSocket.dial(samplesPort);
    assert(resDial == 0);
}

void CommunicatorBase::endSocket() {
    if (renderingState) setRenderingState(false);
    samplesSocket.close();
    commandSocket.close();
}

void CommunicatorBase::setRenderingState(bool state) {
    if(renderingState == state) return;
    commandSocket.doCommand(state? commandSocket.Command::StartRender : commandSocket.Command::StopRender);
    renderingState = state;
}

bool CommunicatorBase::nextFrameAvailable() {
    if(getReportedChannelCount() == 0) return false;
    if (latestBlockMessage == nullptr || latestBlockMessage->atSeqReadEnd()) {
        // Need next block
        latestBlockMessage.reset();
        latestBlockMessage = samplesSocket.receiveBlock();
        if (!latestBlockMessage->success() || latestBlockMessage->getSize() <= 0) {
            assert(latestBlockMessage->getResult() == NNG_EAGAIN || latestBlockMessage->getResult() == NNG_ETIMEDOUT);
            latestBlockMessage.reset();
        }
    }
    return latestBlockMessage != nullptr;
}

bool CommunicatorBase::copyNextFrame(float* buf, bool bypassAvailabilityCheck) {
    if(!bypassAvailabilityCheck && !nextFrameAvailable()) return false;

    // No specific channels to extract - dump entire frame
    if(latestBlockMessage->seqReadAndPut(buf, getReportedChannelCount())) {
        return true;
    } else {
        // Error copying frame - frame might not have been ready.
        // This should never occur.
        // If you bypassAvailabilityCheck, you should have called nextFrameAvailable yourself prior to this call.
        assert(false);
        return false;
    }
}

CommunicatorRegistry & CommunicatorRegistry::getInstance()
{
    static CommunicatorRegistry instance; // Guaranteed to be destroyed, Instantiated on first use.
    return instance;
}