#include "exportaction_parameterprocessing.h"
#include "envelopecreator.h"

#include <assert.h>
#include <sstream>
#include <cmath>

#define CURVE_APPROXIMATION_STEP_MS 100
#define CURVE_APPROXIMATION_DEVIATION_THRESHOLD 0.05
#define VALUE_TOLERANCE 0.00001 // Value has to be within 0.001% of target to be considered the same

namespace {
    bool valueWithinTolerance(double actual, double target, double range = 1.0) {
        if(actual == target) return true;
        double variance = range * (double)VALUE_TOLERANCE;
        return ((actual > (target - variance)) && (actual < (target + variance)));
    }
}

CumulatedPointData::CumulatedPointData(std::chrono::nanoseconds regionStart, std::chrono::nanoseconds regionEnd) : regionStart{ regionStart }, regionEnd{ regionEnd }
{
    // We must ensure point data is provided at regionStart for the initial block
    //  and point data at the regionEnd to ensure parameter values are held throughout
    // If we just put an entry for that time in pointData, it will get populated during finalisation if not fulfilled by usual means
    auto startPoint = fromNs(regionStart);
    auto endPoint = fromNs(regionEnd);
    pointsData.insert({ startPoint, std::map<AdmParameter, std::vector<double>>() });
    pointsData.insert({ endPoint, std::map<AdmParameter, std::vector<double>>() });
}

std::vector<AdmAuthoringError> CumulatedPointData::useEnvelopeDataForParameter(TrackEnvelope& envelope, Parameter& parameter, AdmParameter admParameter, ReaperAPI const & api)
{
    // This function reads and parses the envelope state chunks.
    // As well as point time and value, the state chunk includes shape data.
    // This means we can appropriately handle non-linear shapes.

    // Other data of interest in the state chunk;
    /*
    <PARMENV 1:49 0.000000 1.000000 0.500000
    ACT 1 -1
    VIS 1 1 1
    LANEHEIGHT 0 0
    ARM 1
    DEFSHAPE 0 -1 -1
    PT 0 0.5 0
    PT 0.5 1 1
    ...
    PT 12 0.5 0
    >
    */
    int envelopeScalingMode = api.GetEnvelopeScalingMode(&envelope);
    std::vector<AdmAuthoringError> errors;

    if(admDataSources.count(admParameter) > 0) {
        assert(false);
        errors.push_back(AdmAuthoringError("Attempting to assign an envelope as a data source to an ADM parameter which already has a data source."));
        return errors;
    }
    if(getSortedTimesOfValuesForParameter(admParameter).size() > 0) {
        assert(false);
        errors.push_back(AdmAuthoringError("Attempting to assign an envelope as a data source to an ADM parameter which already has parameter data."));
        return errors;
    }
    api.Envelope_SortPoints(&envelope); // Means the stored non-linear points will be presorted

    // Grab values from points data in state chunk
    char chunk[512000]; // enough for about 12000 automation points
    bool res = api.GetEnvelopeStateChunk(&envelope, chunk, 512000, false);
    if(!res) {
        assert(false);
        errors.push_back(AdmAuthoringError("Could not get data for envelope to use as ADM parameter data source."));
        return errors;
    }
    std::istringstream chunkSs(chunk);
    std::string line;
    bool chunkComplete = false; // Set when we find the end of the chunk so we know we read it all.

    std::vector<double> nonLinearPointTimes;

    std::optional<double> normValueToPreceed; // For square envelopes, we need to insert an additional point at the next point time to hold the value.

    while (std::getline(chunkSs, line)) {
        if (line.rfind("PT", 0) == 0) {
            // starts with PT, so point data
            std::string::size_type end = line.find(" ");
            double pointTime;
            double normValue; // Note, values in chunk are already correct - no need to ScaleFromEnvelopeMode
            int shape = EnvelopeShape::Linear; // Default - linear
            bool complete = false;

            try{
                line.erase(0, end + 1);
                if(line.size() > 0) pointTime = std::stod(line, &end);

                line.erase(0, end + 1);
                if(line.size() > 0) normValue = std::stod(line, &end);

                complete = true;

                line.erase(0, end + 1);
                if(line.size() > 0) shape = std::stoi(line, &end);

            } catch(std::invalid_argument e){
                errors.push_back(AdmAuthoringError("Could not parse data for envelope point."));
            }

            assert(complete);
            if(complete){
                // Got time and value, and maybe shape (assumed 0 or "linear" if wasn't present)
                if(pointTime >= 0.0) {
                    if(normValueToPreceed.has_value()) {
                        // Store a point using the previous points value - used to hold envelopes for square shapes.
                        newPointData(pointTime, admParameter, parameter.reverseMap(*normValueToPreceed));
                        normValueToPreceed.reset();
                    }
                    // Store current point
                    newPointData(pointTime, admParameter, parameter.reverseMap(normValue));
                    // Behaviour dependent on current envelope shape
                    if(shape == EnvelopeShape::Square) {
                        normValueToPreceed = normValue;
                    } else if(shape != EnvelopeShape::Linear || envelopeScalingMode != 0) { // if the envelopeScalingMode is NOT 0, no ranges are linear
                        nonLinearPointTimes.push_back(pointTime);
                    }
                } else {
                    errors.push_back(AdmAuthoringError("Automation point found before 0:00.000 - will ignore."));
                }
            } else {
                errors.push_back(AdmAuthoringError("Could not find all required values for envelope point whilst trying to parse envelope data."));
            }

        } else if(line == ">") {
            chunkComplete = true;
        }
    }

    assert(chunkComplete);
    if(!chunkComplete){
        errors.push_back(AdmAuthoringError("Envelope data larger than expected. Some parameter values may be missing."));
    }

    // Figure out non-linear regions
    auto allPointTimes = getSortedTimesOfValuesForParameter(admParameter);
    std::vector<std::pair<double, double>> nonLinearRegions;
    for(auto& startTime : nonLinearPointTimes) {
        for(auto& pointTime : allPointTimes) {
            if(pointTime > startTime) {
                // Region found (has both a start and end time)
                nonLinearRegions.push_back(std::make_pair(startTime, pointTime));
                break;
            }
        }
    }
    // Fill in non-linear points
    for( auto const& [admParameter, admDataSource] : admDataSources )
    {
        approximateNonLinearCurves(admParameter, api);
    }

    if(DefaultEnvelopeCreator::isWrappedParam(admParameter)) {
    // Figure out all potential wrap-around opportunities
    // Add "reinforcement" points to make it clear which direction the wrap occurs
        int newDataPoints = 0;
        do { // Keep creating these reinforcement points until no more are required.
            newDataPoints = 0;
            auto allPointTimes = getSortedTimesOfValuesForParameter(admParameter);
            for(int pointTimeIndex = 0; pointTimeIndex < (allPointTimes.size() - 1); pointTimeIndex++) {
                double startTime = allPointTimes[pointTimeIndex];
                double endTime = allPointTimes[pointTimeIndex + 1];
                auto startValues = getValuesForParameterAtTime(startTime, admParameter);
                auto endValues = getValuesForParameterAtTime(endTime, admParameter);
                double startValueNorm = parameter.forwardMap(startValues.back());
                double endValueNorm = parameter.forwardMap(endValues.front());
                double change = std::abs((double)(startValueNorm - endValueNorm));
                double period = endTime - startTime;
                if(change >= 0.5 && period > 0.001) {
                    double midTime = (period / 2.0) + startTime;
                    double envValAtTime;
                    api.Envelope_Evaluate(&envelope, midTime, 0, 0, &envValAtTime, nullptr, nullptr, nullptr);
                    double normValAtTime = api.ScaleFromEnvelopeMode(envelopeScalingMode, envValAtTime);
                    newPointData(midTime, admParameter, parameter.reverseMap(normValAtTime));
                    newDataPoints++;
                }
            }
        } while(newDataPoints > 0);
    }

    // Register it
    admDataSources.insert(std::make_pair(admParameter, AdmDataSource{ &envelope, &parameter, nonLinearRegions }));

    return errors;
}

std::vector<AdmAuthoringError> CumulatedPointData::useConstantValueForParameter(AdmParameter admParameter, double value)
{
    std::vector<AdmAuthoringError> errors;

    if(admDataSources.count(admParameter) > 0) {
        assert(false);
        errors.push_back(AdmAuthoringError("Attempting to assign a constant value to an ADM parameter which already has a data source."));
        return errors;
    }
    if(getSortedTimesOfValuesForParameter(admParameter).size() > 0) {
        assert(false);
        errors.push_back(AdmAuthoringError("Attempting to assign a constant value to an ADM parameter which already has parameter data."));
        return errors;
    }
    // No need to convert/scale here - it's not from an envelope
    newPointData(0.0, admParameter, value);
    return errors;
}

void CumulatedPointData::newPointData(double time, AdmParameter admParameter, double value)
{
    auto pointsDataIt = pointsData.find(time);
    if(pointsDataIt == pointsData.end()) {
        // Nothing at this timestamp yet
        std::map < AdmParameter, std::vector<double>> paramValuePair;
        paramValuePair.insert(std::make_pair(admParameter, std::vector<double>{value}));
        pointsData.insert(std::make_pair(time, paramValuePair));
    } else {
        auto& pointsDataForTime = pointsDataIt->second;
        auto pointsDataParamIt = pointsDataForTime.find(admParameter);
        if(pointsDataParamIt == pointsDataForTime.end()) {
            // Nothing for this parameter at this timestamp
            pointsDataForTime.insert(std::make_pair(admParameter, std::vector<double>{value}));
        } else {
            // Append parameter to existing  at this timestamp yet
            pointsDataParamIt->second.push_back(value);
        }
    }
}

std::vector<double> CumulatedPointData::getSortedPointTimes()
{
    std::vector<double> times;
    for(auto const& [time, valuesMap] : pointsData) {
        times.push_back(time);
    }
    return times; // Map is inherently sorted, so vector is sorted
}

std::map<AdmParameter, std::vector<double>>* CumulatedPointData::getParameterMapAtTime(double time)
{
    auto paramMapIt = pointsData.find(time);
    if(paramMapIt == pointsData.end()) return nullptr;
    return &(paramMapIt->second);
}

std::vector<AdmParameter> CumulatedPointData::getParametersAtTime(double time)
{
    std::vector<AdmParameter> admParameters;
    auto pointsDataAtTime = getParameterMapAtTime(time);
    if(!pointsDataAtTime) return admParameters;

    for(auto const& [admParameter, values] : *pointsDataAtTime) {
        if(values.size() > 0) {
            admParameters.push_back(admParameter);
        }
    }

    return admParameters;
}

std::vector<double> CumulatedPointData::getValuesForParameterAtTime(double time, AdmParameter admParameter)
{
    std::vector<double> values;
    auto pointsDataAtTime = getParameterMapAtTime(time);
    if(!pointsDataAtTime) return values;

    auto pointsDataAtTimeIt = pointsDataAtTime->find(admParameter);
    if(pointsDataAtTimeIt != pointsDataAtTime->end()) {
        return pointsDataAtTimeIt->second;
    }

    return values;
}

std::vector<double> CumulatedPointData::getSortedTimesOfValuesForParameter(AdmParameter admParameter)
{
    std::vector<double> times;

    for(auto const& [time, valuesMap] : pointsData)
    {
        auto valuesMapIt = valuesMap.find(admParameter);
        if(valuesMapIt != valuesMap.end() && valuesMapIt->second.size() > 0) {
            times.push_back(time);
        }

    }

    return times; // Map is inherently sorted, so vector is sorted
}

void CumulatedPointData::finaliseSphericalPositionParameters(ReaperAPI const& api)
{
    // Ensure we have positional parameters on all data points
    createValuesForParameterAtAllPointTimes(AdmParameter::OBJECT_AZIMUTH, 0.0, api);
    ensureFinalPointPresent(AdmParameter::OBJECT_AZIMUTH, api);
    createValuesForParameterAtAllPointTimes(AdmParameter::OBJECT_ELEVATION, 0.0, api);
    ensureFinalPointPresent(AdmParameter::OBJECT_ELEVATION, api);
    createValuesForParameterAtAllPointTimes(AdmParameter::SPEAKER_AZIMUTH, 0.0, api);
    ensureFinalPointPresent(AdmParameter::SPEAKER_AZIMUTH, api);
    createValuesForParameterAtAllPointTimes(AdmParameter::SPEAKER_ELEVATION, 0.0, api);
    ensureFinalPointPresent(AdmParameter::SPEAKER_ELEVATION, api);
}

void CumulatedPointData::finaliseCartesianPositionParameters(ReaperAPI const & api)
{
    // Ensure we have positional parameters on all data points
    createValuesForParameterAtAllPointTimes(AdmParameter::OBJECT_X, 0.0, api);
    ensureFinalPointPresent(AdmParameter::OBJECT_X, api);
    createValuesForParameterAtAllPointTimes(AdmParameter::OBJECT_Y, 0.0, api);
    ensureFinalPointPresent(AdmParameter::OBJECT_Y, api);
    // Note that there are no cartesian parameters for DirectSpeakers in BS.2076-2
}

void CumulatedPointData::finaliseOtherParameters(ReaperAPI const & api)
{
    // Iterate through existing params at all times.
    // If any parameter is evaluated to not default, we should create a value here
    for(int admParameterIndex = 0; admParameterIndex != (int)AdmParameter::NONE; admParameterIndex++) {
        auto admParameter = (AdmParameter)admParameterIndex;
        auto admParameterDefaultVal = getAdmParameterDefault(admParameter);

        if(admParameterDefaultVal.has_value()) {
            createValuesForParameterAtAllPointTimes(admParameter, *admParameterDefaultVal, api, false);
        }

        ensureFinalPointPresent(admParameter, api);
    }
}

int CumulatedPointData::approximateNonLinearCurves(AdmParameter admParameter, ReaperAPI const & api)
{
    auto admDataSourcesIt = admDataSources.find(admParameter);
    if(admDataSourcesIt == admDataSources.end()) return 0;
    auto admDataSource = &(admDataSourcesIt->second);
    int newPointTimesCount = 0;

    for(auto& region : admDataSource->nonLinearRegions) {

        // - Firstly search for existing timestamps within the non-linear regions and fill those parameter values in - it's a free win - param data with no additional block created.

        for(auto& time : getSortedPointTimes()) {
            if(time <= region.first) continue;
            if(time >= region.second) break;
            double envValAtTime;
            api.Envelope_Evaluate(admDataSource->envelope, time, 0, 0, &envValAtTime, nullptr, nullptr, nullptr);
            int envelopeScalingMode = api.GetEnvelopeScalingMode(admDataSource->envelope);
            double normValAtTime = api.ScaleFromEnvelopeMode(envelopeScalingMode, envValAtTime);
            newPointData(time, admParameter, admDataSource->parameter->reverseMap(normValAtTime));
        }

        // - Next, use equally spaced, absolute check points - this increases the chance of concurrent nonlinear parameter regions to share the same time stamp and therefore the same block.

        long timeInMs = long(region.first * 1000.0);
        // Start at next approximation step
        timeInMs = timeInMs - (timeInMs % CURVE_APPROXIMATION_STEP_MS) + CURVE_APPROXIMATION_STEP_MS;

        while(true) {
            double pointTime = (timeInMs / 1000.0);
            if(pointTime > region.second) break;

            double envValAtTime;
            api.Envelope_Evaluate(admDataSource->envelope, pointTime, 0, 0, &envValAtTime, nullptr, nullptr, nullptr);
            int envelopeScalingMode = api.GetEnvelopeScalingMode(admDataSource->envelope);
            double realValAtTime = api.ScaleFromEnvelopeMode(envelopeScalingMode, envValAtTime);

            auto impliedValAtTime = getAdmImpliedValueForParameterAtTime(pointTime, admParameter, admDataSource->parameter);
            assert(impliedValAtTime.has_value()); // This should definitely return a value - we're bound by 2 known points.
            double deviation = std::abs((double)(realValAtTime - *impliedValAtTime));

            if(deviation > CURVE_APPROXIMATION_DEVIATION_THRESHOLD) {
                newPointData(pointTime, admParameter, admDataSource->parameter->reverseMap(realValAtTime));
                newPointTimesCount++;
            }

            timeInMs += CURVE_APPROXIMATION_STEP_MS;
        }

    }

    return newPointTimesCount;
}

std::optional<double> CumulatedPointData::getAdmImpliedValueForParameterAtTime(double targetTime, AdmParameter admParameter, Parameter* parameter)
{
    auto existingValuesAtTime = getValuesForParameterAtTime(targetTime, admParameter);
    if(existingValuesAtTime.size() > 0) return parameter->forwardMap(existingValuesAtTime.back());

    std::optional<double> pointBefore{};
    for(auto pointTime : getSortedTimesOfValuesForParameter(admParameter)) {
        if(pointTime < targetTime) {
            if(pointBefore.has_value()) {
                if(pointTime > *pointBefore) pointBefore = pointTime;
            } else { // No value yet
                pointBefore = pointTime;
            }
        } else if(pointTime > targetTime) {
            // Got just past targetTime now... make sure we had a pointBefore
            if(pointBefore.has_value()) {
                double beforeTime = *pointBefore;
                double afterTime = pointTime;
                double beforeValue = parameter->forwardMap(getValuesForParameterAtTime(beforeTime, admParameter).back());
                double afterValue = parameter->forwardMap(getValuesForParameterAtTime(afterTime, admParameter).back());
                // Interpolate between the two values.
                double progression = (targetTime - beforeTime) / (afterTime - beforeTime); // How far progressed targetTime is from beforeTime (0.0) towards afterTime (1.0)
                return std::optional<double>(beforeValue + ((afterValue - beforeValue) * progression));
            }
            return std::optional<double>(); // We've got beyond targetTime without finding a pointBefore
        }
    }

    // Couldn't find point at time or two neighbouring points around time.
    return std::optional<double>();
}

bool CumulatedPointData::haveEnvelopeFor(AdmParameter admParameter)
{
    auto admDataSourcesIt = admDataSources.find(admParameter);
    if(admDataSourcesIt != admDataSources.end()) {
        return admDataSourcesIt->second.envelope != nullptr;
    }
    return false;
}

bool CumulatedPointData::haveDataFor(AdmParameter admParameter)
{
    for(auto const& [time, valuesMap] : pointsData)
    {
        auto valuesMapIt = valuesMap.find(admParameter);
        if(valuesMapIt != valuesMap.end()) {
            if(valuesMapIt->second.size() > 0) return true;
        }
    }
    return false;
}

bool CumulatedPointData::multipleValuesForSingleParameterAtTime(double time)
{
    auto pointsDataAtTime = getParameterMapAtTime(time);
    if(!pointsDataAtTime) return false;

    for(auto& [admParameter, values] : *pointsDataAtTime) {
        if(values.size() > 1) return true;
    }

    return false;
}

std::optional<std::vector<std::shared_ptr<adm::AudioBlockFormatObjects>>> CumulatedPointData::generateAudioBlockFormatObjects(std::shared_ptr<admplug::PluginSuite> pluginSuite, PluginInstance * pluginInst, ReaperAPI const & api)
{
    // Use spherical if no plug-in suite/instance provided, or determine from plug-in suite
    bool useSph = !pluginSuite || !pluginInst || pluginSuite->pluginUsesSphericalCoordinates(pluginInst); // Use spherical if no plug-in suite provided, or determine from plug-in suite

    // Fill in missing parameter values so that all blocks will have position data (mandatory)
    if(useSph) {
        finaliseSphericalPositionParameters(api);
    } else {
        finaliseCartesianPositionParameters(api);
    }
    // Fill in other parameters at all point times if they don't equate to the default
    finaliseOtherParameters(api);

    // Now create the blocks
    std::vector<std::shared_ptr<adm::AudioBlockFormatObjects>> blocks;

    auto times = getSortedPointTimes();
    auto lastEndTime = toNs(0.0);

    auto audioObjectDuration = regionEnd - regionStart;

    for(auto timeIt = times.begin(); timeIt != times.end(); timeIt++) {

        const auto timeNs = toNs(*timeIt);
        
        // Use front or back of parameter values vectors
        bool processBack = (*timeIt == 0.0); // For starting block, no point doing front and back - front would be ineffective.

        while(true) {

            std::shared_ptr<adm::AudioBlockFormatObjects> block;
   
            auto rtime = lastEndTime - regionStart;
            auto duration = timeNs - lastEndTime;
            auto endTime = rtime + duration;

            if(rtime < audioObjectDuration && endTime >= std::chrono::nanoseconds::zero()) {

                // Can't have negative Rtime
                if(rtime < std::chrono::nanoseconds::zero()) {
                    rtime = std::chrono::nanoseconds::zero();
                    duration = endTime;
                }

                if(useSph) {
                    auto azVals = getValuesForParameterAtTime(*timeIt, AdmParameter::OBJECT_AZIMUTH);
                    auto azVal = processBack ? azVals.back() : azVals.front();
                    auto elVals = getValuesForParameterAtTime(*timeIt, AdmParameter::OBJECT_ELEVATION);
                    auto elVal = processBack ? elVals.back() : elVals.front();

                    auto sphPos = adm::SphericalPosition((adm::Azimuth)azVal, (adm::Elevation)elVal);

                    auto distVals = getValuesForParameterAtTime(*timeIt, AdmParameter::OBJECT_DISTANCE);
                    if(distVals.size() > 0) {
                        sphPos.set((adm::Distance)(processBack ? distVals.back() : distVals.front()));
                    }

                    block = std::make_shared<adm::AudioBlockFormatObjects>(sphPos);

                } else {
                    auto xVals = getValuesForParameterAtTime(*timeIt, AdmParameter::OBJECT_X);
                    auto xVal = processBack ? xVals.back() : xVals.front();
                    auto yVals = getValuesForParameterAtTime(*timeIt, AdmParameter::OBJECT_Y);
                    auto yVal = processBack ? yVals.back() : yVals.front();

                    auto cartPos = adm::CartesianPosition((adm::X)xVal, (adm::Y)yVal);

                    auto zVals = getValuesForParameterAtTime(*timeIt, AdmParameter::OBJECT_Z);
                    if(zVals.size() > 0) {
                        cartPos.set((adm::Z)(processBack ? zVals.back() : zVals.front()));
                    }

                    block = std::make_shared<adm::AudioBlockFormatObjects>(cartPos);
                }

                for(auto const&[admParameter, values] : *getParameterMapAtTime(*timeIt)) {
                    switch(admParameter) {
                        case AdmParameter::OBJECT_GAIN:
                            block->set(adm::Gain::fromLinear(processBack ? values.back() : values.front()));
                            break;
                        case AdmParameter::OBJECT_HEIGHT:
                            block->set((adm::Height)(processBack ? values.back() : values.front()));
                            break;
                        case AdmParameter::OBJECT_WIDTH:
                            block->set((adm::Width)(processBack ? values.back() : values.front()));
                            break;
                        case AdmParameter::OBJECT_DEPTH:
                            block->set((adm::Depth)(processBack ? values.back() : values.front()));
                            break;
                        case AdmParameter::OBJECT_DIFFUSE:
                            block->set((adm::Diffuse)(processBack ? values.back() : values.front()));
                            break;

                            // TODO: Add any other parameters we want to support here
                    }
                }

                // Block rtime and duration
                block->set((adm::Rtime)rtime);
                block->set((adm::Duration)duration);

                // Use JumpPosition for second block if multiple points at same position
                if (multipleValuesForSingleParameterAtTime(*timeIt) && processBack) {
                    block->set(adm::JumpPosition((adm::JumpPositionFlag)true));
                }

                blocks.push_back(block);
            }

            lastEndTime = timeNs;

            // If we have already created an additional point or we don't need to create an additional point anyway, quit.
            if(processBack || !multipleValuesForSingleParameterAtTime(*timeIt)) break;
            processBack = true; // Create an additional block for this time

        }
    }

    return std::optional<std::vector<std::shared_ptr<adm::AudioBlockFormatObjects>>>(blocks);
}

void CumulatedPointData::createValuesForParameterAtAllPointTimes(AdmParameter admParameter, double defaultVal, ReaperAPI const & api, bool createEvenIfAlreadyDefault)
{
    auto allTimes = getSortedPointTimes();
    int newDataPoints = 0;

    auto admDataSourcesIt = admDataSources.find(admParameter);
    if(admDataSourcesIt != admDataSources.end()) {
        // We have an envelope we can refer to to get the true interpolated value
        auto env = admDataSourcesIt->second.envelope;
        int envelopeScalingMode = api.GetEnvelopeScalingMode(env);
        auto param = admDataSourcesIt->second.parameter;
        for(auto& time : allTimes) {
            if(getValuesForParameterAtTime(time, admParameter).size() == 0) {
                // Need to create a value for this parameter at this time
                double envValAtTime;
                api.Envelope_Evaluate(env, time, 0, 0, &envValAtTime, nullptr, nullptr, nullptr);
                double realValAtTime = api.ScaleFromEnvelopeMode(envelopeScalingMode, envValAtTime);
                auto convValAtTime = param->reverseMap(realValAtTime);
                if(createEvenIfAlreadyDefault || !valueWithinTolerance(convValAtTime, defaultVal)) {
                    newPointData(time, admParameter, convValAtTime);
                }
            }
        }
        return;
    }

    auto valueTimes = getSortedTimesOfValuesForParameter(admParameter);
    if(valueTimes.size() > 0){
        // No envelope, but we have a value we can use.
        // Sanity check; If it is a non-automated parameter, we should expect only one value and it should be at time 0.
        assert(valueTimes.size() == 1);
        assert(valueTimes[0] == 0.0);

        double val = getValuesForParameterAtTime(valueTimes[0], admParameter).back();
        if(createEvenIfAlreadyDefault || !valueWithinTolerance(val, defaultVal)) {
            for(auto& time : allTimes) {
                if(getValuesForParameterAtTime(time, admParameter).size() == 0) {
                    newPointData(time, admParameter, val);
                }
            }
        }

        return;
    }

    // This would be unusual (because we should at least have set one by getting the FX param value if there was no envelope), but we need to fall back to a default value.
    // It could occur if the plugin suite did not say it supported this parameter (but it should have done if it's a mandatory parameter such as coordinate!)
    if(createEvenIfAlreadyDefault) {
        for(auto& time : allTimes) {
            // Need to put the default value in at all times
            if(getValuesForParameterAtTime(time, admParameter).size() == 0) {
                newPointData(time, admParameter, defaultVal);
            }
        }
    }

    return;
}

void CumulatedPointData::ensureFinalPointPresent(AdmParameter admParameter, ReaperAPI const & api)
{
    // We have to do this due to REAPER's "hold" behaviour after a last envelope point
    // Conversely, ADM would return to default values
    // - so we need ensure we have an AudioBlock to cover the full duration by ensuring we have a "final" point

    auto pointTimes = getSortedTimesOfValuesForParameter(admParameter);
    if(pointTimes.size() == 0) return; // No points ever created - we're not using this parameter

    auto allTimes = getSortedPointTimes();
    if(allTimes.size() == 0) return; // Should never happen, but safe-guard
    auto reqFinalPointTime = allTimes.back();

    if(pointTimes.back() == reqFinalPointTime) return; // Already have parameter value at this time

    auto lastValues = getValuesForParameterAtTime(pointTimes.back(), admParameter);
    if(lastValues.size() == 0) return; // Should never happen, but safe-guard
    auto lastValue = lastValues.back();
    newPointData(reqFinalPointTime, admParameter, lastValue);
}
