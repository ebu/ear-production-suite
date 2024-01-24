#ifndef PARAMETISED_H
#define PARAMETISED_H

#include "../include_gmock.h"
#include <parameter.h>
#include <parametised.h>
#include <plugin.h>
#include <track.h>
#include <envelopecreator.h>

namespace admplug {

class MockPlugin : public Plugin {
public:
    MOCK_CONST_METHOD2(getEnvelope, std::unique_ptr<AutomationEnvelope>(PluginParameter const&, EnvelopeCreator const&));
    MOCK_CONST_METHOD2(setParameter, void(PluginParameter const&, double value));
    MOCK_CONST_METHOD1(getParameter, std::optional<double> (const PluginParameter &parameter));
};

}

#endif // PARAMETISED_H
