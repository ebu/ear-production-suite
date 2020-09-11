#ifndef AUTOMATIONPOINT_H
#define AUTOMATIONPOINT_H
#include <chrono>

namespace admplug {

class AutomationPoint
{
public:
    explicit AutomationPoint(double val);
    AutomationPoint(std::chrono::nanoseconds timeNs, std::chrono::nanoseconds duration, double val);
    AutomationPoint(double timeSeconds, double duration, double val);
    double time() const;
    double duration() const;
    double value() const;
private:
    double pointTime;
    double pointDuration;
    double pointValue;
};

}

#endif // AUTOMATIONPOINT_H
