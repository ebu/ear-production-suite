#ifndef AUTOMATIONPOINT_H
#define AUTOMATIONPOINT_H
#include <chrono>

namespace admplug {

class AutomationPoint
{
public:
    explicit AutomationPoint(double val);
    AutomationPoint(std::chrono::nanoseconds timeNs, std::chrono::nanoseconds duration, double val);

    void setTimeNs(std::chrono::nanoseconds time);
    void setDurationNs(std::chrono::nanoseconds duration);
    void setDurationFromEffectiveTimeNs(std::chrono::nanoseconds effectiveTime);

    double time() const;
    double duration() const;
    double effectiveTime() const;

    std::chrono::nanoseconds timeNs() const;
    std::chrono::nanoseconds durationNs() const;
    std::chrono::nanoseconds effectiveTimeNs() const;

    double value() const;

private:
    std::chrono::nanoseconds pointTime;
    std::chrono::nanoseconds pointDuration;
    double pointValue;
};

}

#endif // AUTOMATIONPOINT_H
