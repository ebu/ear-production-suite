#pragma once
#include <optional>
#include <algorithm>
#include <adm/elements.hpp>
#include "automationpoint.h"
#include "automationenvelope.h"
#include "parameter.h"

namespace {
    bool pointsTimeSorter(admplug::AutomationPoint &pointA, admplug::AutomationPoint &pointB) {
        if(pointA.timeNs() == pointB.timeNs()) return pointA.durationNs() < pointB.durationNs();
        return pointA.timeNs() < pointB.timeNs();
    }
}

namespace admplug {

namespace detail {
template<typename ElementT>
bool hasComponentValue(ElementT&) {
    return true;
}

template<typename ElementT, typename Head, typename... Tail>
bool hasComponentValue(ElementT const& element) {
    if(element.template has<Head>()) {
            auto child = element.template get<Head>();
            return hasComponentValue<Head, Tail...>(child);
    }
    return false;
}

template<typename ElementT>
auto getComponentValue(ElementT const& element) {
    return element.get();
}

template<typename ElementT, typename Head, typename... Tail>
auto getComponentValue(ElementT const& element) {
    auto child = element.template get<Head>();
    return getComponentValue<Head, Tail...>(child);
}



template <typename InputT, typename... ParameterChain>
auto getOptionalAdmComponent(InputT const& input) {
    using ValueT = decltype(getComponentValue<InputT, ParameterChain...>(input));
    if(hasComponentValue<InputT, ParameterChain...>(input)) {
        return std::optional<ValueT>(detail::getComponentValue<InputT, ParameterChain...>(input));
    }
    return std::optional<ValueT>{};
}

template <typename... ParameterChain>
auto getAdmComponent(adm::AudioBlockFormatObjects const& block) {
    return getOptionalAdmComponent<adm::AudioBlockFormatObjects, ParameterChain...>(block);

}

template <typename...ParameterChain>
auto getAdmComponent(adm::AudioBlockFormatDirectSpeakers const& block) {
    return getOptionalAdmComponent<adm::AudioBlockFormatDirectSpeakers, ParameterChain...>(block);
}

template<typename Param, typename T>
float getParamValueOrZero(T const& element) {
  if (element.template has<Param>()) {
    return element.template get<Param>().get();
  }
  return 0;
}

using ns = std::chrono::nanoseconds;

template<typename TimedParameterT, typename BlockT>
ns getValueOrZero(BlockT block) {
    if(block.template has<TimedParameterT>()) {
        return block.template get<TimedParameterT>().get().asNanoseconds();
    }
    return ns::zero();
}

template<typename BlockT>
std::pair<ns, ns> getStartAndDuration(BlockT const& block) {
    return std::make_pair(getValueOrZero<adm::Rtime>(block), getValueOrZero<adm::Duration>(block));
}

template<typename...ParameterChain, typename BlockT>
std::optional<AutomationPoint> getPoint(Parameter const& param, BlockT const& block) {
    auto [start, duration] = getStartAndDuration(block);
    auto value = getAdmComponent<ParameterChain...>(block);
    if(value) {
        return param.forwardMap(AutomationPoint(start, duration, static_cast<double>(*value)));
    }
    return std::optional<AutomationPoint>{};
}

class CartesianToSpherical {
public:
    CartesianToSpherical(float x, float y, float z);

    template<typename ComponentT>
    double get();
private:
    constexpr double toDegrees(double radians) const {
        return 180.0 * radians / PI;
    }
    static constexpr double POLE_EPSILON = 0.0001;
    static constexpr double PI = 3.141592653589793238462643383279502884197169399375;
    double x{0};
    double y{0};
    double z{0};
};

template<>
inline double CartesianToSpherical::get<adm::Azimuth>() {
    if(y * y > POLE_EPSILON) {
        return toDegrees(atan2(-x, y));
    }
    return 0;
}

template<>
inline double CartesianToSpherical::get<adm::Distance>() {
    return sqrt(x*x + y*y + z*z);
}

template<>
inline double CartesianToSpherical::get<adm::Elevation>() {
    auto r = get<adm::Distance>();
    if(r * r > POLE_EPSILON) {
        return toDegrees(asin(z / r));
    }
    return 0;
}

class RoomCartesianToSphericalSpeaker {
public:
    explicit RoomCartesianToSphericalSpeaker(adm::AudioBlockFormatDirectSpeakers block);
    template <typename Param>
    double get();

private:
    void calculate();
    std::optional<adm::SphericalSpeakerPosition> position;
    adm::AudioBlockFormatDirectSpeakers block;
};

template <>
inline double RoomCartesianToSphericalSpeaker::get<adm::Azimuth>() {
    calculate();
    return position->get<adm::Azimuth>().get();
}

template <>
inline double RoomCartesianToSphericalSpeaker::get<adm::Elevation>() {
    calculate();
    return position->get<adm::Elevation>().get();
}

template <>
inline double RoomCartesianToSphericalSpeaker::get<adm::Distance>() {
    calculate();
    return position->get<adm::Distance>().get();
}

class RoomCartesianToSpherical {
public:
    explicit RoomCartesianToSpherical(adm::AudioBlockFormatObjects block);
    template <typename Param>
    double get();

private:
    template <typename T>
    T getValueOrDefault() {
        if(block.template has<T>()) {
            return block.template get<T>();
        }
        return T{0};
    }

    bool hasExtent();
    void calculate();
    std::optional<adm::SphericalPosition> position;
    std::tuple<adm::Width, adm::Height, adm::Depth> extent;
    adm::AudioBlockFormatObjects block;
};

template <>
inline double RoomCartesianToSpherical::get<adm::Azimuth>() {
    calculate();
    return position->get<adm::Azimuth>().get();
}

template <>
inline double RoomCartesianToSpherical::get<adm::Elevation>() {
    calculate();
    return position->get<adm::Elevation>().get();
}

template <>
inline double RoomCartesianToSpherical::get<adm::Distance>() {
    calculate();
    return position->get<adm::Distance>().get();
}

template <>
inline double RoomCartesianToSpherical::get<adm::Width>() {
    calculate();
    return std::get<adm::Width>(extent).get();
}

template <>
inline double RoomCartesianToSpherical::get<adm::Height>() {
    calculate();
    return std::get<adm::Height>(extent).get();
}

template <>
inline double RoomCartesianToSpherical::get<adm::Depth>() {
    calculate();
    return std::get<adm::Depth>(extent).get();
}

std::optional<RoomCartesianToSpherical>
getConverter(adm::AudioBlockFormatObjects const& block);

std::optional<RoomCartesianToSphericalSpeaker> getConverter(adm::AudioBlockFormatDirectSpeakers const&);

template<typename ComponentT, typename BlockT>
std::optional<AutomationPoint> convertSphericalPoint(Parameter const& param, BlockT const& block) {
    auto converter = getConverter(block);
    if(converter) {
        auto [start, duration] = getStartAndDuration(block);
        auto value = converter->template get<ComponentT>();
        return param.forwardMap(AutomationPoint{start, duration, value});
    }
    return std::optional<AutomationPoint>{};
}

template<typename ComponentT>
std::optional<AutomationPoint> getSphericalPoint(Parameter const& param, adm::AudioBlockFormatObjects const& block) {
    auto point = getPoint<adm::SphericalPosition, ComponentT>(param, block);
    if(point) {
        return point;
    } else {
        return convertSphericalPoint<ComponentT>(param, block);
    }
}

template<typename ComponentT>
std::optional<AutomationPoint> getSphericalPoint(Parameter const& param, adm::AudioBlockFormatDirectSpeakers const& block) {
    auto point = getPoint<adm::SphericalSpeakerPosition, ComponentT>(param, block);
    if(point) {
        return point;
    } else {
        return convertSphericalPoint<ComponentT>(param, block);
    }
}

template<typename ComponentT>
std::optional<AutomationPoint> getSphericalPoint(Parameter const& param, adm::AudioBlockFormatHoa const& block) {
    auto point = getPoint<adm::SphericalSpeakerPosition, ComponentT>(param, block);
    if(point) {
        return point;
    } else {
        return convertSphericalPoint<ComponentT>(param, block);
    }
}

std::vector<AutomationPoint> simplify(std::vector<AutomationPoint> const& points);
void fixEffectiveTimeOverlaps(std::vector<AutomationPoint> &points);

template<typename ParameterT, typename AutomatableT>
void applyAutomation(std::vector<AutomationPoint> points,
                     double startTime,
                     const ParameterT &parameter,
                     const AutomatableT &automatable)
{
    if(!points.empty()) {
        std::sort(points.begin(), points.end(), pointsTimeSorter); // fixEffectiveTimeOverlaps and simplify assumes the points are ordered by time, so do it
        fixEffectiveTimeOverlaps(points);
        points = simplify(points);

        parameter.set(automatable, points.front().value());

        if(points.size() > 1) {
            auto envelope = parameter.getEnvelope(automatable);
            for(auto& point : points) {
                envelope->addPoint(point);
            }
            envelope->createPoints(startTime);
        }
    }
}

}
}
