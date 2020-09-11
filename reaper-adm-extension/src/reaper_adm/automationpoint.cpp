#include "automationpoint.h"

using namespace admplug;
using ns = std::chrono::nanoseconds;

AutomationPoint::AutomationPoint(double val) : AutomationPoint{ns::zero(), ns::zero(), val}
{

}

AutomationPoint::AutomationPoint(std::chrono::nanoseconds timeNs, std::chrono::nanoseconds duration, double val) :
    pointTime{timeNs.count() / 1000000000.0},
    pointDuration{duration.count() / 1000000000.0},
    pointValue{val} {
}

AutomationPoint::AutomationPoint(double timeSeconds, double duration, double val) : pointTime{timeSeconds}, pointDuration{duration}, pointValue{val}
{
}

double AutomationPoint::time() const
{
    return pointTime;
}

double AutomationPoint::duration() const
{
    return pointDuration;
}

double AutomationPoint::value() const
{
    return pointValue;
}
