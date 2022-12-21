#ifndef PARAMETER_H
#define PARAMETER_H
#include <memory>
#include <string>
#include <optional>
#include "automationpoint.h"
#include "parametervaluemapping.h"

class MediaTrack;
class TrackEnvelope;

namespace admplug {

enum class AdmParameter : int32_t {
    OBJECT_AZIMUTH,
    OBJECT_ELEVATION,
    OBJECT_DISTANCE,
    OBJECT_X,
    OBJECT_Y,
    OBJECT_Z,
    OBJECT_HEIGHT,
    OBJECT_WIDTH,
    OBJECT_DEPTH,
    OBJECT_GAIN, // TODO: Gain param applies to more than just objects
    OBJECT_DIFFUSE,
    OBJECT_DIVERGENCE,
    OBJECT_DIVERGENCE_AZIMUTH_RANGE,
    SPEAKER_AZIMUTH,
    SPEAKER_ELEVATION,
    SPEAKER_DISTANCE,
    NFC_REF_DIST,
    NONE
};

std::optional<double> getAdmParameterDefault(AdmParameter admParameter);

enum class TrackParameterType : int32_t {
    VOLUME
};

class Plugin;
class Track;
class ReaperAPI;
class AutomationEnvelope;

class Parameter
{
public:
    virtual ~Parameter() = default;
    virtual AutomationPoint forwardMap(AutomationPoint point) const;
    virtual AutomationPoint reverseMap(AutomationPoint point) const;
    virtual double forwardMap(double val) const = 0;
    virtual double reverseMap(double val) const = 0;
    virtual AdmParameter admParameter() const = 0;
};

class PluginParameter : public Parameter {
public:
    virtual ~PluginParameter() = default;
    using Parameter::forwardMap;
    using Parameter::reverseMap;
    virtual int index() const = 0;
    virtual double forwardMap(double val) const = 0;
    virtual double reverseMap(double val) const = 0;
    virtual AdmParameter admParameter() const = 0;
    virtual std::unique_ptr<AutomationEnvelope> getEnvelope(Plugin const& plugin) const;
    virtual void set(Plugin const& plugin, double value) const;
};

class TrackParameter : public Parameter {
public:
    virtual ~TrackParameter() = default;
    using Parameter::forwardMap;
    using Parameter::reverseMap;
    virtual TrackParameterType type() const = 0;
    virtual double forwardMap(double val) const = 0;
    virtual double reverseMap(double val) const = 0;
    virtual AdmParameter admParameter() const = 0;
    virtual std::unique_ptr<AutomationEnvelope> getEnvelope(Track const& track) const;
    virtual void set(Track const& track, double value) const;
};

class MappedPluginParameter : public PluginParameter {
public:
    MappedPluginParameter(int parameterIndex,
                          AdmParameter parameter,
                          std::shared_ptr<ParameterValueMapping const> mapping);
    using PluginParameter::forwardMap;
    using PluginParameter::reverseMap;
    double forwardMap(double val) const override;
    double reverseMap(double val) const override;
    AdmParameter admParameter() const override;
    int index() const override;

private:
    int parameterIndex;
    AdmParameter parameter;
    std::shared_ptr<ParameterValueMapping const> mapping;
};

class SimplePluginParameter : public PluginParameter {
public:
    SimplePluginParameter(int parameterIndex, ParameterRange range);
    using PluginParameter::forwardMap;
    using PluginParameter::reverseMap;
    double forwardMap(double val) const override;
    double reverseMap(double val) const override;
    AdmParameter admParameter() const override;
    int index() const override;
private:
    int parameterIndex;
    ParameterRange parameterRange;
};

class MappedTrackParameter : public TrackParameter {
public:
    MappedTrackParameter(TrackParameterType parameterType,
                         AdmParameter parameter,
                         std::shared_ptr<ParameterValueMapping const> mapping);
    using TrackParameter::forwardMap;
    using TrackParameter::reverseMap;
    double forwardMap(double val) const override;
    double reverseMap(double val) const override;
    AdmParameter admParameter() const override;
    TrackParameterType type() const override;

private:
    TrackParameterType parameterType;
    AdmParameter parameter;
    std::shared_ptr<ParameterValueMapping const> mapping;
};


std::unique_ptr<PluginParameter> createPluginParameter(int parameterIndex, AdmParameter parameter, std::shared_ptr<ParameterValueMapping const> mapping);
std::unique_ptr<PluginParameter> createPluginParameter(int parameterIndex, ParameterRange range);
std::unique_ptr<PluginParameter> createPluginParameter(int parameterIndex, AdmParameter parameter, ParameterRange range);
std::unique_ptr<TrackParameter> createTrackParameter(TrackParameterType parameterType, AdmParameter parameter, std::shared_ptr<ParameterValueMapping const> mapping);
std::unique_ptr<TrackParameter> createTrackParameter(TrackParameterType parameterType, AdmParameter parameter, ParameterRange range);



}

#endif // PARAMETER_H
