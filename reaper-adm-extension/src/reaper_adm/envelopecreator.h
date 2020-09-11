#pragma once
#include <memory>
#include "parameter.h"

class TrackEnvelope;

namespace admplug {

class AutomationEnvelope;
class Parameter;
class ReaperAPI;

class EnvelopeCreator {
public:
    virtual ~EnvelopeCreator();
    virtual std::unique_ptr<AutomationEnvelope> create(TrackEnvelope* envelope, ReaperAPI const& api) const = 0;
};

class DefaultEnvelopeCreator : public EnvelopeCreator {
public:
    DefaultEnvelopeCreator(Parameter const& param);
    std::unique_ptr<AutomationEnvelope> create(TrackEnvelope* envelope, ReaperAPI const& api) const;
    static bool isWrappedParam(AdmParameter testParameter);
private:
    AdmParameter admParameter;
    static const std::vector<AdmParameter> wrappedParams;
};

}
