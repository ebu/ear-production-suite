#ifndef TRACK_H
#define TRACK_H

#include <gmock/gmock.h>
#include <track.h>
#include <envelopecreator.h>
#include <automationenvelope.h>
#include <plugin.h>
#include <projectelements.h>
#include <color.h>

namespace admplug {

class MockTrack : public Track {
public:
    MOCK_CONST_METHOD2(getEnvelope, std::unique_ptr<AutomationEnvelope>(TrackParameter const&, EnvelopeCreator const&));
    MOCK_METHOD1(createPlugin, std::unique_ptr<Plugin>(std::string));
    MOCK_METHOD2(deletePlugin, int(std::string, bool));
    MOCK_METHOD1(getPlugin, std::unique_ptr<Plugin>(std::string pluginName));
    MOCK_METHOD1(getPlugin, std::unique_ptr<Plugin>(int pluginIndex));
    MOCK_METHOD1(getPlugins, std::vector<std::unique_ptr<Plugin>>(std::string pluginName));
    MOCK_METHOD0(getPlugins, std::vector<std::unique_ptr<Plugin>>());
    MOCK_CONST_METHOD2(setParameter, void(TrackParameter const&, double value));
    MOCK_METHOD1(setAsVCASlave, void(TrackGroup const&));
    MOCK_METHOD1(setAsVCAMaster, void(TrackGroup const&));
    MOCK_METHOD1(setColor, void(Color));
    MOCK_CONST_METHOD0(get, MediaTrack*());
    MOCK_CONST_METHOD0(stillExists, bool());
    MOCK_METHOD1(setChannelCount, void(int));
    MOCK_CONST_METHOD0(getChannelCount, int());
    MOCK_METHOD1(moveToBefore, void(int));
    MOCK_METHOD0(disableMasterSend, void());
    MOCK_METHOD0(getName, std::string());
    MOCK_METHOD1(setName, void(std::string));
    MOCK_METHOD0(hideFromTrackControlPanel, void());
    MOCK_CONST_METHOD0(isPluginChainBypassed, bool());
    MOCK_METHOD4(route, void(Track&, int channelCount, int firstSourceChannel, int firstDestinationChannel));
};
}


#endif // TRACK_H
