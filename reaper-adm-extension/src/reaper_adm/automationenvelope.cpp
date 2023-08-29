#include <cmath>
#include <memory>
#include "automationenvelope.h"

namespace {
std::chrono::nanoseconds nsMultiply(std::chrono::nanoseconds const &ns, double const multiplier) {
    double asDouble = (double)ns.count() * multiplier;
    long long asLongLong = asDouble + 0.5; // correct rounding
    return std::chrono::nanoseconds(asLongLong);
}
}

using namespace admplug;

DefinedStartEnvelope::DefinedStartEnvelope(TrackEnvelope *trackEnvelope, ReaperAPI const& api) : trackEnvelope{trackEnvelope}, api{api}
{

}

void DefinedStartEnvelope::addPoint(AutomationPoint point)
{
    if(points.empty() && point.durationNs() > std::chrono::nanoseconds::zero()) {
        points.emplace_back(point.timeNs(), std::chrono::nanoseconds::zero(), point.value());
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
    bool noSortPoints{true};

    for(auto& point : points) {
        pointOnTimeline = point.effectiveTime() + pointsOffset;
        if (earliestPointOnTimeline < 0.0 || pointOnTimeline < earliestPointOnTimeline) {
            earliestPointOnTimeline = pointOnTimeline;
        }
        api.InsertEnvelopePoint(trackEnvelope, pointOnTimeline, point.value(), 0, 0, false, &noSortPoints);
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

void WrappingEnvelope::addDiscontinuityPoints(AutomationPoint const& currentPoint) {
    auto previousPoint = automationEnvelope.getPoints().back();
    auto distance = fabs(currentPoint.value() - previousPoint.value());

    if(isTopToBottomWrap(currentPoint.value(), previousPoint.value())) {
      double wrappedDistance = calcWrappedDistance(currentPoint.value(), previousPoint.value());

      if(wrappedDistance < distance) {
        double wrapDurationProportion = (wrappedDistance == 0.0)? 0.0 : ((1.0 - previousPoint.value()) / wrappedDistance);
        std::chrono::nanoseconds wrapDuration{nsMultiply(currentPoint.durationNs(), wrapDurationProportion)};
        std::chrono::nanoseconds wrapTime{ currentPoint.timeNs() + wrapDuration };
        if(previousPoint.effectiveTimeNs() != currentPoint.timeNs() || previousPoint.value() != 1.0) {
            automationEnvelope.addPoint(AutomationPoint{ currentPoint.timeNs(), wrapDuration, 1.0 });
        }
        if(currentPoint.effectiveTimeNs() != wrapTime || currentPoint.value() != 0.0) {
            automationEnvelope.addPoint(AutomationPoint{ wrapTime, std::chrono::nanoseconds::zero(), 0.0 });
        }
      }
    }

    if(isBottomToTopWrap(currentPoint.value(), previousPoint.value())) {
      auto wrappedDistance = calcWrappedDistance(previousPoint.value(), currentPoint.value());
      if(wrappedDistance < distance) {
        double wrapDurationProportion = (wrappedDistance == 0.0)? 0.0 : (previousPoint.value() / wrappedDistance);
        std::chrono::nanoseconds wrapDuration{nsMultiply(currentPoint.durationNs(), wrapDurationProportion)};
        std::chrono::nanoseconds wrapTime{ currentPoint.timeNs() + wrapDuration };
        if(previousPoint.effectiveTimeNs() != currentPoint.timeNs() || previousPoint.value() != 0.0) {
            automationEnvelope.addPoint(AutomationPoint{ currentPoint.timeNs(), wrapDuration, 0.0 });
        }
        if(currentPoint.effectiveTimeNs() > wrapTime || currentPoint.value() != 1.0) {
            automationEnvelope.addPoint(AutomationPoint{ wrapTime, std::chrono::nanoseconds::zero(), 1.0 });
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
