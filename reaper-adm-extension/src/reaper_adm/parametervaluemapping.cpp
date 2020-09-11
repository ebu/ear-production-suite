#include <cmath>
#include <algorithm>
#include "parametervaluemapping.h"

using namespace admplug;


AutomationPoint admplug::ParameterValueMapping::operator()(AutomationPoint point) const
{
    return forwardMap(point);
}

AutomationPoint admplug::ParameterValueMapping::forwardMap(AutomationPoint point) const
{
    return AutomationPoint(point.time(), point.duration(), forwardMap(point.value()));
}

AutomationPoint admplug::ParameterValueMapping::reverseMap(AutomationPoint point) const
{
    return AutomationPoint(point.time(), point.duration(), reverseMap(point.value()));
}




FunctionalMapping::FunctionalMapping(std::function<double(double)> forwardMapper, std::function<double(double)> reverseMapper) : forwardMapper{ forwardMapper }, reverseMapper{ reverseMapper }
{
}

double admplug::FunctionalMapping::forwardMap(double val) const
{
    return forwardMapper(val);
}

double admplug::FunctionalMapping::reverseMap(double val) const
{
    return reverseMapper(val);
}

CompositeMapping::CompositeMapping(std::vector<std::shared_ptr<const ParameterValueMapping> > mappings) : mappings{mappings}
{
}

double admplug::CompositeMapping::forwardMap(double val) const
{
    for(int i = 0; i < mappings.size(); i++) {
        val = mappings[i]->forwardMap(val);
    }
    return val;
}

double admplug::CompositeMapping::reverseMap(double val) const
{
    for(int i = (mappings.size() - 1); i >= 0; i--) {
        val = mappings[i]->reverseMap(val);
    }
    return val;
}

void CompositeMapping::addMapping(std::shared_ptr<ParameterValueMapping> mapping)
{
    mappings.push_back(mapping);
}


ParameterRange::ParameterRange(double min, double max) :min{min}, max{max}
{
}

std::shared_ptr<ParameterValueMapping const> ParameterRange::modulus() const {
    auto minimum = std::min(min, max);
    auto maximum = std::max(min, max);
    return std::make_shared<FunctionalMapping>(
        [minimum, maximum](double val){
        if(val == maximum || val == minimum) {
            // Prevents values right at extremities getting wrapped
            return val;
        }
        auto range = maximum - minimum;
        return fmod(val - minimum, range) + minimum;
    },
        [](double val) {
        // Can't properly undo without original data
        return val;
    }
    );
}

std::shared_ptr<ParameterValueMapping const> ParameterRange::clipper() const {
    auto minimum = min;
    auto maximum = max;
    return std::make_shared<FunctionalMapping>(
        [minimum, maximum](double val) {
        auto clipped = std::max(val, minimum);
        clipped = std::min(clipped, maximum);
        return clipped;
    },
        [](double val) {
        // Can't properly undo without original data
        return val;
    }
    );
}

std::shared_ptr<ParameterValueMapping const> ParameterRange::normaliser() const {
    auto minimum = min;
    auto maximum = max;
    return std::make_shared<FunctionalMapping>(
        [minimum, maximum](double val) {
        auto range = maximum - minimum;
        return ((val - minimum) / range);
    },
        [minimum, maximum](double val) {
        auto range = maximum - minimum;
        return ((val * range) + minimum);
    }
    );
}


double admplug::Inversion::forwardMap(double val) const
{
    return -val;
}

double admplug::Inversion::reverseMap(double val) const
{
    return -val;
}



using MappingPtr = std::shared_ptr<ParameterValueMapping const>;
std::shared_ptr<CompositeMapping> admplug::getCombinedMapping(std::initializer_list<MappingPtr> list) {
    std::vector<MappingPtr> mappings{ list };
    return std::make_shared<CompositeMapping>(mappings);
}



std::shared_ptr<const ParameterValueMapping> map::sequence(std::initializer_list<std::shared_ptr<const ParameterValueMapping> > mappings) {
    return std::make_shared<CompositeMapping>(std::move(mappings));
}

std::shared_ptr<const ParameterValueMapping> map::normalise(const ParameterRange &range) {
    return range.normaliser();
}

std::shared_ptr<const ParameterValueMapping> map::clip() {
    auto clipRange = ParameterRange{0.0, 1.0};
    return clipRange.clipper();
}

std::shared_ptr<const ParameterValueMapping> map::linearToDb(const ParameterRange& range) {
    auto toDb = std::make_shared<LinearToDb>();
    return map::sequence({toDb, range.normaliser(), range.clipper()});
}

std::shared_ptr<const ParameterValueMapping> map::clip(const ParameterRange &range) {
    return range.clipper();
}

std::shared_ptr<const ParameterValueMapping> map::wrap(const ParameterRange &range) {
    return range.modulus();
}

std::shared_ptr<const ParameterValueMapping> map::invert() {
    return std::make_shared<Inversion>();
}



double LinearToDb::forwardMap(double val) const
{
    if(val <=0.00001) {
        return -100;
    }
    return 20 * log10(val);
}

double LinearToDb::reverseMap(double val) const
{
    return pow(10, val/20);
}
