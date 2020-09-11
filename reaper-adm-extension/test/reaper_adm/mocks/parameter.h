#include <gmock/gmock.h>
#include <parameter.h>

namespace admplug {

class MockParameter : public Parameter {
 public:
   MOCK_CONST_METHOD1(forwardMap, AutomationPoint(AutomationPoint));
   MOCK_CONST_METHOD0(admParameter, AdmParameter());
};

class MockPluginParameter : public PluginParameter {
 public:
   MOCK_CONST_METHOD0(index, int());
   MOCK_CONST_METHOD1(forwardMap, AutomationPoint(AutomationPoint));
   MOCK_CONST_METHOD1(reverseMap, AutomationPoint(AutomationPoint));
   MOCK_CONST_METHOD1(forwardMap, double(double));
   MOCK_CONST_METHOD1(reverseMap, double(double));
   MOCK_CONST_METHOD0(admParameter, AdmParameter());
   MOCK_CONST_METHOD1(getEnvelope, std::unique_ptr<AutomationEnvelope>(Plugin const&));
   MOCK_CONST_METHOD2(set, void(Plugin const& plugin, double value));
};

class MockTrackParameter : public TrackParameter {
 public:
   MOCK_CONST_METHOD0(type, TrackParameterType());
   MOCK_CONST_METHOD1(forwardMap, AutomationPoint(AutomationPoint));
   MOCK_CONST_METHOD1(reverseMap, AutomationPoint(AutomationPoint));
   MOCK_CONST_METHOD1(forwardMap, double(double));
   MOCK_CONST_METHOD1(reverseMap, double(double));
   MOCK_CONST_METHOD0(admParameter, AdmParameter());
   MOCK_CONST_METHOD1(getEnvelope, std::unique_ptr<AutomationEnvelope>(Track const&));
   MOCK_CONST_METHOD2(set, void(Track const& pluginOrTrack, double value));
};

}  // namespace admplug
