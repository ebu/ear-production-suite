#pragma once

#include <gmock/gmock.h>
#include <automationenvelope.h>

namespace admplug {
class MockAutomationEnvelope : public AutomationEnvelope {
public:
    MOCK_METHOD1(addPoint, void(AutomationPoint));
    MOCK_METHOD0(createPoints, void());
    MOCK_METHOD1(createPoints, void(double));
};
}

