#pragma once

#include <map>
#include <vector>
#include <optional>
#include <adm/adm.hpp>
#include "pluginsuite.h"
#include "plugin.h"

#include "exportaction_issues.h"
#include "parameter.h"
#include "reaperapi.h"

inline std::chrono::nanoseconds toNs(double seconds) {
    auto sTime = std::chrono::duration<double>(seconds);
    return std::chrono::duration_cast<std::chrono::nanoseconds>(sTime);
}

inline double fromNs(std::chrono::nanoseconds ns) {
    return ns.count() / 1000000000.0;
}

using namespace admplug;

class CumulatedPointData
{
public:
    CumulatedPointData(std::chrono::nanoseconds regionStart, std::chrono::nanoseconds regionEnd);
    ~CumulatedPointData() {};

    std::vector<AdmAuthoringError> useEnvelopeDataForParameter(TrackEnvelope& envelope, Parameter& parameter, AdmParameter admParameter, ReaperAPI const& api);
    std::vector<AdmAuthoringError> useConstantValueForParameter(AdmParameter admParameter, double value);

    std::vector<double> getSortedPointTimes();
    std::map<AdmParameter, std::vector<double>>* getParameterMapAtTime(double time);
    std::vector<AdmParameter> getParametersAtTime(double time);
    std::vector<double> getValuesForParameterAtTime(double time, AdmParameter admParameter);
    std::vector<double> getSortedTimesOfValuesForParameter(AdmParameter admParameter);
    void finaliseSphericalPositionParameters(ReaperAPI const& api);
    void finaliseCartesianPositionParameters(ReaperAPI const& api);
    void finaliseOtherParameters(ReaperAPI const& api);
    std::optional<double> getAdmImpliedValueForParameterAtTime(double time, AdmParameter admParameter, Parameter* parameter);
    bool haveEnvelopeFor(AdmParameter admParameter);
    bool haveDataFor(AdmParameter admParameter);
    bool multipleValuesForSingleParameterAtTime(double time);

    std::optional<std::vector<std::shared_ptr<adm::AudioBlockFormatObjects>>> generateAudioBlockFormatObjects(std::shared_ptr<admplug::PluginSuite> pluginSuite, PluginInstance* pluginInst, ReaperAPI const& api);
    std::optional<std::vector<std::shared_ptr<adm::AudioBlockFormatDirectSpeakers>>> generateAudioBlockFormatDirectSpeakers(std::shared_ptr<admplug::PluginSuite> pluginSuite, PluginInstance* pluginInst, ReaperAPI const& api);
    std::optional<std::vector<std::shared_ptr<adm::AudioBlockFormatBinaural>>> generateAudioBlockFormatBinaural(std::shared_ptr<admplug::PluginSuite> pluginSuite, PluginInstance* pluginInst, ReaperAPI const& api);
    std::optional<std::vector<std::shared_ptr<adm::AudioBlockFormatMatrix>>> generateAudioBlockFormatMatrix(std::shared_ptr<admplug::PluginSuite> pluginSuite, PluginInstance* pluginInst, ReaperAPI const& api);
    std::optional<std::vector<std::shared_ptr<adm::AudioBlockFormatHoa>>> generateAudioBlockFormatHoa(std::shared_ptr<admplug::PluginSuite> pluginSuite, PluginInstance* pluginInst, ReaperAPI const& api);

private:
    struct AdmDataSource {
        TrackEnvelope* envelope;
        Parameter* parameter;
        std::vector<std::pair<double, double>> nonLinearRegions;
    };

    std::map<AdmParameter, AdmDataSource> admDataSources;
    std::map<double, std::map<AdmParameter, std::vector<double>>> pointsData;

    void newPointData(double time, AdmParameter admParameter, double value);
    void createValuesForParameterAtAllPointTimes(AdmParameter admParameter, double defaultVal, ReaperAPI const& api, bool createEvenIfAlreadyDefault = true);
    void ensureFinalPointPresent(AdmParameter admParameter, ReaperAPI const& api);
    int approximateNonLinearCurves(AdmParameter admParameter, ReaperAPI const& api);

    std::chrono::nanoseconds regionStart;
    std::chrono::nanoseconds regionEnd;
};