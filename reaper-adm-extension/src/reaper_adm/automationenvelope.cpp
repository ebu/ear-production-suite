#include <cmath>
#include <memory>
#include "automationenvelope.h"

using namespace admplug;

DefinedStartEnvelope::DefinedStartEnvelope(TrackEnvelope *trackEnvelope, ReaperAPI const& api) : trackEnvelope{trackEnvelope}, api{api}
{

}

void DefinedStartEnvelope::addPoint(AutomationPoint point)
{
    if(points.empty() && point.duration() > 0) {
        points.emplace_back(point.time(), 0, point.value());
    }
    points.push_back(point);
}

void DefinedStartEnvelope::createPoints()
{
    createPoints(0.0);
}

void DefinedStartEnvelope::createPoints(double pointsOffset)
{
    double earliestPointOnTimeline{ -1.0 };
    double pointOnTimeline{ 0.0 };
    bool sortPoints{false};

    for(auto& point : points) {
        pointOnTimeline = point.time() + point.duration() + pointsOffset;
        if (earliestPointOnTimeline < 0.0 || pointOnTimeline < earliestPointOnTimeline) {
            earliestPointOnTimeline = pointOnTimeline;
        }
        api.InsertEnvelopePoint(trackEnvelope, pointOnTimeline, point.value(), 0, 0, false, &sortPoints);
    }
    api.Envelope_SortPoints(trackEnvelope);

    // Remove Reapers auto-inserted point (will fail silently if it's the only point)
    if (earliestPointOnTimeline > 0.0) {
        api.DeleteEnvelopePointRange(trackEnvelope, 0.0, earliestPointOnTimeline);
        api.Envelope_SortPoints(trackEnvelope);
    }
}

std::vector<AutomationPoint> &DefinedStartEnvelope::getPoints()
{
    return points;
}


WrappingEnvelope::WrappingEnvelope(TrackEnvelope *trackEnvelope, ReaperAPI const& api) :
    automationEnvelope{trackEnvelope, api}
{

}

void admplug::WrappingEnvelope::addPoint(AutomationPoint point)
{
    auto& points = automationEnvelope.getPoints();
    auto firstPoint = points.empty();
    if(!firstPoint) {
      addDiscontinuityPoints(point);
    }
    automationEnvelope.addPoint(point);
}

void WrappingEnvelope::createPoints()
{
    automationEnvelope.createPoints();
}

void WrappingEnvelope::createPoints(double pointsOffset)
{
    automationEnvelope.createPoints(pointsOffset);
}

void WrappingEnvelope::addDiscontinuityPoints(AutomationPoint const& point) {
    auto& points = automationEnvelope.getPoints();

    auto& previousPoint = points.back();
    auto previousValue = previousPoint.value();
    auto previousStartTime = previousPoint.time();
    auto previousDuration = previousPoint.duration();
    auto previousEffectiveTime = previousStartTime + previousDuration;

    auto value = point.value();
    auto startTime = point.time();
    auto duration = point.duration();
    auto effectiveTime = startTime + duration;
    // Fix for floating-point precision errors in calculating effective time by adding start and duration.
    if(startTime < previousEffectiveTime) {
        startTime = previousEffectiveTime;
        duration = effectiveTime - startTime;
    }

    auto distance = fabs(value - previousValue);

    if(isTopToBottomWrap(value, previousValue)) {
      double wrappedDistance = calcWrappedDistance(value, previousValue);

      if(wrappedDistance < distance) {
        double wrapDurationProportion = (wrappedDistance == 0.0)? 0.0 : ((1.0 - previousValue) / wrappedDistance);
        auto wrapDuration = wrapDurationProportion * duration;
        auto wrapTime = startTime + wrapDuration;
        if(previousEffectiveTime != startTime || previousValue != 1.0) {
            automationEnvelope.addPoint(AutomationPoint{ startTime, wrapDuration, 1.0 });
        }
        if(effectiveTime != wrapTime || value != 0.0) {
            automationEnvelope.addPoint(AutomationPoint{ wrapTime, 0.0, 0.0 });
        }
      }
    }

    if(isBottomToTopWrap(value, previousValue)) {
      auto wrappedDistance = calcWrappedDistance(previousValue, value);
      if(wrappedDistance < distance) {
        double wrapDurationProportion = (wrappedDistance == 0.0)? 0.0 : (previousValue / wrappedDistance);
        auto wrapDuration = wrapDurationProportion * duration;
        auto wrapTime = startTime + wrapDuration;
        if(previousEffectiveTime != startTime || previousValue != 0.0) {
            automationEnvelope.addPoint(AutomationPoint{ startTime, wrapDuration, 0.0 });
        }
        if(effectiveTime > wrapTime || value != 1.0) {
            automationEnvelope.addPoint(AutomationPoint{ wrapTime, 0.0, 1.0 });
        }
      }
    }
}

bool WrappingEnvelope::isTopToBottomWrap(double value, double previousValue) const {
    return previousValue > 0.5 && value < 0.5;
}

double WrappingEnvelope::calcWrappedDistance(double lower, double upper) const {
    return lower + (1.0 - upper);
}

bool WrappingEnvelope::isBottomToTopWrap(double value, double previousValue) const {
    return value > 0.5 && previousValue < 0.5;
}
