#include "envelopecreator.h"
#include "automationenvelope.h"
#include <algorithm>

using namespace admplug;

const std::vector<AdmParameter> DefaultEnvelopeCreator::wrappedParams = {
    AdmParameter::OBJECT_AZIMUTH,
    AdmParameter::SPEAKER_AZIMUTH
};

DefaultEnvelopeCreator::DefaultEnvelopeCreator(const Parameter &param) : admParameter{param.admParameter()} {

}

std::unique_ptr<AutomationEnvelope> DefaultEnvelopeCreator::create(TrackEnvelope *envelope, const ReaperAPI &api) const {

    auto found = std::find(wrappedParams.cbegin(),
                           wrappedParams.cend(),
                           admParameter);
    if(!(found == std::end(wrappedParams))) {
        return std::make_unique<WrappingEnvelope>(envelope, api);
    }
    return std::make_unique<DefinedStartEnvelope>(envelope, api);
}

bool admplug::DefaultEnvelopeCreator::isWrappedParam(AdmParameter testParameter)
{
    return std::find(wrappedParams.begin(), wrappedParams.end(), testParameter) != wrappedParams.end();
}

EnvelopeCreator::~EnvelopeCreator() = default;
