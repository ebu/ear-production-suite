#include "automationpoint.h"

using namespace admplug;
using ns = std::chrono::nanoseconds;

AutomationPoint::AutomationPoint(double val) : AutomationPoint{ns::zero(), ns::zero(), val}
{

}

AutomationPoint::AutomationPoint(std::chrono::nanoseconds timeNs, std::chrono::nanoseconds duration, double val) :
    pointTime{timeNs},
    pointDuration{duration},
    pointValue{val}
{
}

void AutomationPoint::setTimeNs(ns time)
{
    pointTime = time;
}

void AutomationPoint::setDurationNs(ns duration)
{
    pointDuration = duration;
}

void AutomationPoint::setDurationFromEffectiveTimeNs(ns effectiveTime)
{
    pointDuration = effectiveTime - pointTime;
}

double AutomationPoint::time() const
{
    return pointTime.count() / 1000000000.0;
}

double AutomationPoint::duration() const
{
    return pointDuration.count() / 1000000000.0;
}

double AutomationPoint::effectiveTime() const
{
    return effectiveTimeNs().count()  / 1000000000.0;
}

ns AutomationPoint::timeNs() const
{
    return pointTime;
}

ns AutomationPoint::durationNs() const
{
    return pointDuration;
}

ns AutomationPoint::effectiveTimeNs() const
{
    return pointTime + pointDuration;
}

double AutomationPoint::value() const
{
    return pointValue;
}
