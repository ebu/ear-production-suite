#include <algorithm>
#include <exception>
#include <vector>
#include "parameter.h"
#include "reaperapi.h"
#include "automationenvelope.h"
#include "track.h"
#include "plugin.h"
#include "envelopecreator.h"

using namespace admplug;

std::optional<double> admplug::getAdmParameterDefault(AdmParameter admParameter){
    // TODO: Should probably be a map, or AdmParameter should be a class or something...
    // TODO: These defaults should be extracted from libadm
    if(admParameter == AdmParameter::OBJECT_DISTANCE) return { 1.0 };
    if(admParameter == AdmParameter::OBJECT_Z) return { 0.0 };
    if(admParameter == AdmParameter::OBJECT_HEIGHT) return { 0.0 };
    if(admParameter == AdmParameter::OBJECT_WIDTH) return { 0.0 };
    if(admParameter == AdmParameter::OBJECT_DEPTH) return { 0.0 };
    if(admParameter == AdmParameter::OBJECT_GAIN) return { 1.0 };
    if(admParameter == AdmParameter::OBJECT_DIFFUSE) return { 0.0 };
    if(admParameter == AdmParameter::NFC_REF_DIST) return { 0.0 };
    return {};
}

AutomationPoint admplug::Parameter::forwardMap(AutomationPoint point) const
{
    return AutomationPoint(point.timeNs(), point.durationNs(), forwardMap(point.value()));
}

AutomationPoint admplug::Parameter::reverseMap(AutomationPoint point) const
{
    return AutomationPoint(point.timeNs(), point.durationNs(), reverseMap(point.value()));
}

std::unique_ptr<AutomationEnvelope> PluginParameter::getEnvelope(const Plugin &plugin) const
{
    DefaultEnvelopeCreator creator{*this};
    return plugin.getEnvelope(*this, creator);
}

void PluginParameter::set(const Plugin &plugin, double value) const
{
    plugin.setParameter(*this, value);
}

std::unique_ptr<AutomationEnvelope> TrackParameter::getEnvelope(const Track &track) const
{
    DefaultEnvelopeCreator creator{*this};
    return track.getEnvelope(*this, creator);
}

void TrackParameter::set(const Track &track, double value) const
{
    track.setParameter(*this, value);
}

MappedPluginParameter::MappedPluginParameter(int parameterIndex, AdmParameter parameter, std::shared_ptr<ParameterValueMapping const> mapping) :
    parameterIndex{parameterIndex},
    parameter{parameter},
    mapping{mapping} {}

AdmParameter MappedPluginParameter::admParameter() const {
    return parameter;
}

double MappedPluginParameter::forwardMap(double val) const {
    return mapping->forwardMap(val);
}

double admplug::MappedPluginParameter::reverseMap(double val) const
{
    return mapping->reverseMap(val);
}

int MappedPluginParameter::index() const{
    return parameterIndex;
}

MappedTrackParameter::MappedTrackParameter(TrackParameterType parameterType, AdmParameter parameter, std::shared_ptr<ParameterValueMapping const> mapping) :
    parameterType{parameterType},
    parameter{parameter},
    mapping{mapping} {}

AdmParameter MappedTrackParameter::admParameter() const {
    return parameter;
}

double MappedTrackParameter::forwardMap(double val) const {
    return mapping->forwardMap(val);
}

double admplug::MappedTrackParameter::reverseMap(double val) const
{
    return mapping->reverseMap(val);
}

TrackParameterType MappedTrackParameter::type() const
{
    return parameterType;
}

std::unique_ptr<PluginParameter> admplug::createPluginParameter(int parameterIndex, AdmParameter parameter, std::shared_ptr<const ParameterValueMapping> mapping)
{
    return std::make_unique<MappedPluginParameter>(parameterIndex, parameter, std::move(mapping));
}

std::unique_ptr<PluginParameter> admplug::createPluginParameter(int parameterIndex, AdmParameter parameter, ParameterRange range) {
    auto mapping = map::sequence({map::clip(range), map::normalise(range)});
    return createPluginParameter(parameterIndex, parameter, mapping);
}

std::unique_ptr<PluginParameter> admplug::createPluginParameter(int parameterIndex, ParameterRange range) {
    return std::make_unique<SimplePluginParameter>(parameterIndex, range);
}

std::unique_ptr<TrackParameter> admplug::createTrackParameter(TrackParameterType parameterType, AdmParameter parameter, std::shared_ptr<const ParameterValueMapping> mapping)
{
    return std::make_unique<MappedTrackParameter>(parameterType, parameter, std::move(mapping));
}

std::unique_ptr<TrackParameter> admplug::createTrackParameter(TrackParameterType parameterType, AdmParameter parameter, ParameterRange range)
{
    auto mapping = map::sequence({map::clip(range), map::normalise(range)});
    return std::make_unique<MappedTrackParameter>(parameterType, parameter, std::move(mapping));
}


SimplePluginParameter::SimplePluginParameter(int parameterIndex, ParameterRange parameterRange) : parameterIndex{parameterIndex}, parameterRange{parameterRange}
{
}

double admplug::SimplePluginParameter::forwardMap(double val) const
{
    auto mapping = map::sequence({parameterRange.clipper(), parameterRange.normaliser()});
    return mapping->forwardMap(val);
}

double admplug::SimplePluginParameter::reverseMap(double val) const
{
    auto mapping = map::sequence({parameterRange.clipper(), parameterRange.normaliser()});
    return mapping->reverseMap(val);
}

AdmParameter admplug::SimplePluginParameter::admParameter() const
{
    return AdmParameter::NONE;
}

int admplug::SimplePluginParameter::index() const
{
    return parameterIndex;
}
