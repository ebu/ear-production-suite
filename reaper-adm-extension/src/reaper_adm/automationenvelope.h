#ifndef AUTOMATIONENVELOPE_H
#define AUTOMATIONENVELOPE_H

#include <vector>
#include "automationpoint.h"
#include "reaperapi.h"

namespace admplug {

/**
 * @brief The AutomationEnvelope class
 * A sequence of AutomationPoints, with facility to add additional points when required for proper interpolation of values by the host
 */
class AutomationEnvelope {
public:
    virtual ~AutomationEnvelope() = default;
    virtual void addPoint(AutomationPoint point) = 0;
    virtual void createPoints() = 0;
    virtual void createPoints(double pointsOffset) = 0;
};

class DefinedStartEnvelope : public AutomationEnvelope {
public:
    DefinedStartEnvelope(TrackEnvelope* trackEnvelope, ReaperAPI const& api);
    void addPoint(AutomationPoint point) override;
    void createPoints() override;
    void createPoints(double pointsOffset) override;
    std::vector<AutomationPoint>& getPoints();
private:
    std::vector<AutomationPoint> points;
    TrackEnvelope* trackEnvelope;
    ReaperAPI const& api;
};

class WrappingEnvelope : public AutomationEnvelope {
public:
    WrappingEnvelope(TrackEnvelope* trackEnvelope, ReaperAPI const& api);
    void addPoint(AutomationPoint point) override;
    void createPoints() override;
    void createPoints(double pointsOffset) override;
private:
    DefinedStartEnvelope automationEnvelope;
    bool isTopToBottomWrap(double value, double previousValue) const;
    bool isBottomToTopWrap(double value, double previousValue) const;
    void addDiscontinuityPoints(const AutomationPoint &point);
    double calcWrappedDistance(double lower, double upper) const;
};

}

#endif // AUTOMATIONENVELOPE_H
