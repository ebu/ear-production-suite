#include "coord_conv.hpp"
#include <array>
#include <cmath>
#include <boost/optional.hpp>

namespace {
    template<typename Param, typename T>
    float getValueOrZero(T const& element) {
        if(element.template has<Param>()) {
            return element.template get<Param>().get();
        }
        return 0;
    }

    struct AzCartMapping {
        double az;
        double x;
        double y;
        double z;
    };

    struct Cartesian {
        double x;
        double y;
        double z;
    };

    Cartesian operator*(Cartesian const& lhs, double rhs) {
        return {lhs.x * rhs, lhs.y * rhs, lhs.z * rhs};
    }

    Cartesian operator*(double lhs, Cartesian const& rhs) {
        return rhs * lhs;
    }

    struct Polar {
        double az;
        double el;
        double d;
    };

    struct ElevationSpec {
        double top{30.0};
        double top_tilde{45.0};
    };

    std::array<AzCartMapping, 5> mapping {{{0, 0, 1, 0},
                                                  {-30, 1, 1, 0},
                                                  {-110, 1, -1, 0},
                                                  {110, -1, -1, 0},
                                                  {30, -1, 1, 0}}};

    constexpr double pi = 3.14159265358979323846;

    constexpr double radians(double degrees) {
        return degrees * pi / 180.0;
    }

    constexpr double degrees(double radians) {
        return radians * 180.0 / pi;
    }

    struct CartesianExtent {
        double x;
        double y;
        double z;
    };

    struct PolarExtent {
        double width;
        double height;
        double depth;
    };



    bool insideAngleRange(double x, double start, double end, double tol = 0.0) {
        while((end - 360.0) > start) {
            end -= 360.0;
        }
        while(end < start) {
            end += 360.0;
        }
        auto start_tol = start - tol;
        while ((x - 360.0) >= start_tol)  {
            x -= 360.0;
        }
        while (x < start_tol) {
            x += 360.0;
        }
        return x <= end + tol;
    }

    std::pair<AzCartMapping, AzCartMapping> findSector(double az) {
        for(auto i = 0u; i != mapping.size(); ++i) {
            auto j = (i + 1) % mapping.size();
            if(insideAngleRange(az, mapping[j].az, mapping[i].az)) {
                return {mapping[i], mapping[j]};
            }
        }
        throw std::runtime_error("azimuth not found in any sector");
    }

    double toAzimuth(double x, double y, double z) {
        return -(degrees(std::atan2(x, y)));
    }

    std::pair<AzCartMapping, AzCartMapping> findCartSector(double az) {
        for(auto i = 0u; i != mapping.size(); ++i) {
            auto j = (i + 1) % mapping.size();
            if(insideAngleRange(az,
                                toAzimuth(mapping[j].x, mapping[j].y, mapping[j].z),
                                toAzimuth(mapping[i].x, mapping[i].y, mapping[i].z))) {
                return {mapping[i], mapping[j]};
            }
        }
        throw std::runtime_error("azimuth not found in any sector");
    }

    double relativeAngle(double x, double y) {
        while ((y - 360.0) >= x) {
            y -= 360.0;
        }
        while (y < x) {
            y += 360.0;
        }
        return y;
    }

    double mapAzToLinear(double left_az, double right_az, double azimuth) {
        auto mid_az = (left_az + right_az) / 2.0;
        auto az_range = right_az - mid_az;
        auto rel_az = azimuth - mid_az;
        auto gain_r = 0.5 + 0.5 * std::tan(radians(rel_az)) / tan(radians(az_range));
        return std::atan2(gain_r, 1-gain_r) * (2 / pi);
    }

    double mapLinearToAz(double left_az, double right_az, double x) {
        auto mid_az = (left_az + right_az) / 2.0;
        auto az_range = right_az - mid_az;
        auto gain_l_= std::cos(x * pi / 2.0);
        auto gain_r_ = std::sin(x * pi / 2.0);
        auto gain_r = gain_r_ / (gain_l_ + gain_r_);
        auto rel_az = degrees(std::atan(2.0 * (gain_r - 0.5) * std::tan(radians(az_range))));
        return mid_az + rel_az;
    }

    std::pair<double, double> calculateXY(double az, double r_xy) {
        AzCartMapping leftSector{};
        AzCartMapping rightSector{};
        std::tie(leftSector, rightSector) = findSector(az);
        auto relAz = relativeAngle(rightSector.az, az);
        auto relLeftAz = relativeAngle(rightSector.az, leftSector.az);
        auto p = mapAzToLinear(relLeftAz, rightSector.az, relAz);
        auto x = r_xy * (leftSector.x + (rightSector.x - leftSector.x) * p);
        auto y = r_xy * (leftSector.y + (rightSector.y - leftSector.y) * p);
        return {x, y};
    }

    std::pair<double, double> calcZ_r_xy(Polar const& polar, ElevationSpec const& elSpec) {
        double z{}, r_xy{};
        if (abs(polar.el) > elSpec.top) {
            auto el_tilde = elSpec.top_tilde + (90.0 - elSpec.top_tilde) * (abs(polar.el) - elSpec.top) / (90.0 - elSpec.top);
            z = std::copysign(polar.d, polar.el);
            r_xy = polar.d * std::tan(radians(90.0 - el_tilde));
        } else {
            auto el_tilde = elSpec.top_tilde * (polar.el / elSpec.top);
            z = tan(radians(el_tilde)) * polar.d;
            r_xy = polar.d;
        }
        return {z, r_xy};
    }

    Cartesian pointPolarToCart(Polar const& polar) {
        Cartesian cart{};
        double r_xy{};
        std::tie(cart.z, r_xy) = calcZ_r_xy(polar, ElevationSpec());
        std::tie(cart.x, cart.y) = calculateXY(polar.az, r_xy);
        return cart;
    }

    bool xAndYNearZero(Cartesian const& cart, double epsilon) {
        return (std::abs(cart.x) < epsilon && std::abs(cart.y) < epsilon);
    }

    bool zNearZero(Cartesian const& cart, double epsilon) {
        return std::abs(cart.z) < epsilon;
    }

    boost::optional<Polar> nearZeroConversion(Cartesian const& cart) {
        double const epsilon{1e-10};
        if (xAndYNearZero(cart, epsilon)) {
            if (zNearZero(cart, epsilon)) {
                return {{0, 0, 0}};
            } else {
                return {{0.0, std::copysign(90, cart.z), std::abs(cart.z)}};
            }
        }
        return {};
    }

    std::array<double, 4> invert(std::array<double, 4> const& matrix2by2) {
        auto det = (matrix2by2[0] * matrix2by2[3] - matrix2by2[1] * matrix2by2[2]);
        auto mul = 1.0 / det;
        auto inv = std::array<double, 4> {matrix2by2[3] * mul, - matrix2by2[1] * mul,
                                          -matrix2by2[2] * mul, matrix2by2[0] * mul};
        return inv;
    }

    std::array<double, 2> dotProduct(std::array<double, 2> const& rowVec, std::array<double, 4> const& matrix2by2) {
        return {rowVec[0] * matrix2by2[0] + rowVec[1] * matrix2by2[2],
                rowVec[0] * matrix2by2[1] + rowVec[1] * matrix2by2[3]};
    }

    // didn't want to add a dependency to do one linear algebra calc
    std::pair<double, double> calculate_g_lr(Cartesian const& cart, std::pair<AzCartMapping, AzCartMapping> const& sectors) {
        auto const& left = sectors.first;
        auto const& right = sectors.second;
        auto inverse = invert({left.x, left.y, right.x, right.y});
        auto dot = dotProduct({cart.x, cart.y}, inverse);
        return {dot[0], dot[1]};
    }

    Polar convert(Cartesian const& cart,
                  ElevationSpec elSpec) {
        auto sectors = findCartSector(toAzimuth(cart.x, cart.y, 0));
        auto const& left = sectors.first;
        auto const& right = sectors.second;
        auto rel_left_az = relativeAngle(right.az, left.az);
        auto g_lr = calculate_g_lr(cart, sectors);
        auto r_xy = g_lr.first + g_lr.second;
        Polar polar{};
        auto relAz = mapLinearToAz(rel_left_az, right.az, g_lr.second / r_xy);
        polar.az = relativeAngle(-180, relAz);
        auto el_tilde = degrees(std::atan(cart.z / r_xy));

        if(std::abs(el_tilde) > elSpec.top_tilde) {
            auto abs_el = elSpec.top + ((90.0 - elSpec.top) * (std::abs(el_tilde) - elSpec.top_tilde))
                                       / (90.0 - elSpec.top_tilde);
            polar.el = std::copysign(abs_el, el_tilde);
            polar.d = std::abs(cart.z);
        } else {
            polar.el = elSpec.top * el_tilde / elSpec.top_tilde;
            polar.d = r_xy;
        }
        return polar;
    }

    Polar pointCartToPolar(Cartesian const& cart) {
        auto converted = nearZeroConversion(cart);
        if (converted) {
            return *converted;
        } else {
            return convert(cart, ElevationSpec());
        }
    }


    // Converts a polar extent value into a cartesian extent assuming position in directly in front of listener
    // and a radius of 1. See BS.2127, section 10.2
    CartesianExtent whd2xyz(PolarExtent const& polarExtent) {
        double x_size_width{}, y_size_width{}, z_size_height{}, y_size_height{}, y_size_depth{};
        if(polarExtent.width < 180.0) {
            x_size_width = std::sin(radians(polarExtent.width / 2.0));
        } else {
            x_size_width = 1.0;
        }
        y_size_width = (1.0 - std::cos(radians(polarExtent.width / 2.0))) / 2.0;

        if(polarExtent.height < 180.0) {
            z_size_height = std::sin(radians(polarExtent.height / 2.0));
        } else {
            z_size_height = 1.0;
        }
        y_size_height = (1.0 - std::cos(radians(polarExtent.height / 2.0))) / 2.0;
        y_size_depth = polarExtent.depth;
        return {x_size_width, std::max(y_size_width, std::max(y_size_height, y_size_depth)), z_size_height};
    }

    PolarExtent xyz2whd(CartesianExtent const& cartExtent) {
        PolarExtent polarExtent{};
        auto width_from_sx = 2.0 * degrees(std::asin(cartExtent.x));
        auto width_from_sy = 2.0 * degrees(std::acos(1.0 - (2 * cartExtent.y)));
        polarExtent.width = width_from_sx + cartExtent.x * std::max(width_from_sy - width_from_sx, 0.0);

        auto height_from_sz = 2.0 * degrees(std::asin(cartExtent.z));
        auto height_from_sy = 2.0 * degrees(std::acos(1.0 - (2.0 * cartExtent.y)));
        polarExtent.height = height_from_sz + cartExtent.z * std::max(height_from_sy - height_from_sz, 0.0);

        auto equiv_y = whd2xyz({polarExtent.width, polarExtent.height, 0.0}).y;
        polarExtent.depth = std::max(0.0, cartExtent.y - equiv_y);
        return polarExtent;
    }

    double euclidianNorm(double a, double b, double c) {
        return std::sqrt(a * a + b * b + c * c);
    }

    Cartesian toCart(Polar const& polar) {
        Cartesian cart{};
        cart.x = std::sin(-pi * polar.az / 180.0) * std::cos(pi * polar.el / 180.0) * polar.d;
        cart.y = std::cos(-pi * polar.az / 180.0) * std::cos(pi * polar.el / 180.0) * polar.d;
        cart.z = std::sin(pi * polar.el / 180.0) * polar.d;
        return cart;
    }

    // returns a rotation matrix that maps a forward vector to a given azimuth an elevation
    // See BS.2127, section 10.2
    std::array<Cartesian, 3> localCoordinateSystem(double az, double el) {
        return {toCart({az - 90.0, 0, 1}),
                toCart({az, el, 1}),
                toCart({az, el + 90.0, 1})};
    }

    std::array<Cartesian, 3> transpose(std::array<Cartesian, 3> const& mat) {
        return{Cartesian{mat[0].x, mat[1].x, mat[2].x},
               Cartesian{mat[0].y, mat[1].y, mat[2].y},
               Cartesian{mat[0].z, mat[1].z, mat[2].z}};
    }

    std::array<Cartesian, 3> calcMPolarToCart(CartesianExtent const& forwardExtent, Polar const& position) {
        std::array<Cartesian, 3> M{};
        auto rotation = localCoordinateSystem(position.az, position.el);
        M[0] = {forwardExtent.x * rotation[0]};
        M[1] = {forwardExtent.y * rotation[1]};
        M[2] = {forwardExtent.z * rotation[2]};
        return M;
    }

    std::array<Cartesian, 3> calcMCartToPolar(CartesianExtent const& forwardExtent, Polar const& position) {
        std::array<Cartesian, 3> M{};
        auto rotation = transpose(localCoordinateSystem(position.az, position.el));
        M[0] = {forwardExtent.x * rotation[0]};
        M[1] = {forwardExtent.y * rotation[1]};
        M[2] = {forwardExtent.z * rotation[2]};
        return M;
    }

    CartesianExtent polarToCartExtent(Polar const & polarPosition, PolarExtent const& polarExtent) {
        auto cartesianForwardExtent = whd2xyz(polarExtent);
        auto M =  calcMPolarToCart(cartesianForwardExtent, polarPosition);
        return {euclidianNorm(M[0].x, M[1].x, M[2].x),
                euclidianNorm(M[0].y, M[1].y, M[2].y),
                euclidianNorm(M[0].z, M[1].z, M[2].z)};
    }

    PolarExtent cartToPolarExtent(Polar const& polarPosition, CartesianExtent const& cartExtent) {
        auto M = calcMCartToPolar(cartExtent, polarPosition);
        auto cartForwardExtent = CartesianExtent{euclidianNorm(M[0].x, M[1].x, M[2].x),
                                                 euclidianNorm(M[0].y, M[1].y, M[2].y),
                                                 euclidianNorm(M[0].z, M[1].z, M[2].z)};
        return xyz2whd(cartForwardExtent);
    }
}

namespace adm {
    namespace {
        PolarExtent extentFromTuple(std::tuple<adm::Width, adm::Height, adm::Depth> const &admExtent) {
            return {std::get<0>(admExtent).get(),
                    std::get<1>(admExtent).get(),
                    std::get<2>(admExtent).get()};
        }

        ::Polar toPolarFromAdm(std::tuple<adm::Azimuth, adm::Elevation, adm::Distance> const &point) {
            return {std::get<0>(point).get(),
                    std::get<1>(point).get(),
                    std::get<2>(point).get()};
        }

        ::Polar toPolarFromAdm(SphericalPosition const &admPosition) {
            return {toPolarFromAdm(std::make_tuple(
                    admPosition.get<adm::Azimuth>(),
                    admPosition.get<adm::Elevation>(),
                    admPosition.get<adm::Distance>()))};
        }

        adm::SphericalPosition toAdmFromPolar(::Polar const &point) {
            return {Azimuth{static_cast<float>(point.az)},
                    Elevation{static_cast<float>(point.el)},
                    Distance{static_cast<float>(point.d)}};
        }

        adm::Extent toAdmFromPolar(::PolarExtent const &extent) {
            return {Width{static_cast<float>(extent.width)},
                    Height{static_cast<float>(extent.height)},
                    Depth{static_cast<float>(extent.depth)}};
        }
    }

    void pointPolarToCart(double azimuth, double elevation, double distance, double &x, double &y, double &z) {
        auto cart = pointPolarToCart(Polar{azimuth, elevation, distance});
        std::tie(x, y, z) = std::tie(cart.x, cart.y, cart.z);
    }

    void pointCartToPolar(double x, double y, double z, double &azimuth, double &elevation, double &distance) {
        auto polar = pointCartToPolar(::Cartesian{x, y, z});
        std::tie(azimuth, elevation, distance) = std::tie(polar.az, polar.el, polar.d);
    }

    CartesianPosition pointPolarToCart(const SphericalPosition &position) {
        auto cart = pointPolarToCart(::Polar{position.get<adm::Azimuth>().get(),
                                             position.get<adm::Elevation>().get(),
                                             position.get<adm::Distance>().get()});
        return CartesianPosition{X{static_cast<float>(cart.x)},
                                 Y{static_cast<float>(cart.y)},
                                 Z{static_cast<float>(cart.z)}};
    }


    SphericalPosition pointCartToPolar(const CartesianPosition &position) {
        auto polar = pointCartToPolar(::Cartesian{position.get<adm::X>().get(),
                                                  position.get<adm::Y>().get(),
                                                  position.get<adm::Z>().get()});
        return adm::SphericalPosition(adm::Azimuth{static_cast<float>(polar.az)},
                                      adm::Elevation{static_cast<float>(polar.el)},
                                      adm::Distance{static_cast<float>(polar.d)});
    }

SphericalSpeakerPosition pointCartToPolar(const CartesianSpeakerPosition &position) {
    auto polarPos = pointCartToPolar(adm::CartesianPosition{adm::X{getValueOrZero<adm::X>(position)},
                                                            adm::Y{getValueOrZero<adm::Y>(position)},
                                                            adm::Z{getValueOrZero<adm::Z>(position)}});
    return adm::SphericalSpeakerPosition(
        polarPos.get<adm::Azimuth>(),
        polarPos.get<adm::Elevation>(),
        polarPos.get<adm::Distance>());
}

CartesianSpeakerPosition pointPolarToCart(const SphericalSpeakerPosition &position) {

    auto cartPos = pointPolarToCart(SphericalPosition{adm::Azimuth{getValueOrZero<adm::Azimuth>(position)},
                                                            adm::Elevation{getValueOrZero<adm::Elevation>(position)},
                                                            adm::Distance{getValueOrZero<adm::Distance>(position)}});
    return adm::CartesianSpeakerPosition(
        cartPos.get<adm::X>(),
        cartPos.get<adm::Y>(),
        cartPos.get<adm::Z>());
}


adm::Extent toAdmFromCartExtent(CartesianExtent extent) {
        return std::make_tuple(Width{static_cast<float>(extent.x)},
                               Height{static_cast<float>(extent.z)},
                               Depth{static_cast<float>(extent.y)});
    }

    std::tuple<CartesianPosition, Extent> polarToCart(std::tuple<SphericalPosition, Extent> polar) {
        auto const &spherical = std::get<0>(polar);
        auto cart = pointPolarToCart(spherical);
        auto polarPos = toPolarFromAdm(spherical);
        auto const &admPolarExtent = std::get<1>(polar);
        auto extent = toAdmFromCartExtent(polarToCartExtent(polarPos, extentFromTuple(admPolarExtent)));
        return {cart, extent};
    }

    std::tuple<SphericalPosition, Extent> cartToPolar(std::tuple<CartesianPosition, Extent> cartPosAndExtent) {
        auto const &cart = std::get<0>(cartPosAndExtent);
        auto const &extent = std::get<1>(cartPosAndExtent);
        CartesianExtent cartExtent{std::get<0>(extent).get(), std::get<2>(extent).get(), std::get<1>(extent).get()};
        auto z = cart.has<adm::Z>() ? cart.get<adm::Z>() : adm::Z{};
        auto polarPos = pointCartToPolar(
                ::Cartesian{cart.get<adm::X>().get(), cart.get<adm::Y>().get(), z.get()});
        auto polarExtent = cartToPolarExtent(polarPos, cartExtent);
        return std::make_tuple(toAdmFromPolar(polarPos), toAdmFromPolar(polarExtent));
    }
}
