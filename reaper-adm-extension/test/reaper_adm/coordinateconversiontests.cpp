#include <coordinate_conversion/coord_conv.hpp>
#include <catch2/catch_all.hpp>
#include <array>
#include <string>
#include <random>
#include <fstream>
#include <sstream>

using Catch::Approx;

namespace {
    adm::CartesianPosition makeCart(double x, double y, double z) {
        return adm::CartesianPosition{adm::X{static_cast<float>(x)},
                                      adm::Y{static_cast<float>(y)},
                                      adm::Z{static_cast<float>(z)}};
    }

    adm::SphericalPosition makePolar(double az, double el, double d) {
        return adm::SphericalPosition{adm::Azimuth{static_cast<float>(az)},
                                      adm::Elevation{static_cast<float>(el)},
                                      adm::Distance{static_cast<float>(d)}};
    }

    std::tuple<adm::Elevation, adm::Z> elZ(float el, float z) {
        return std::make_tuple(adm::Elevation(el), adm::Z(z));
    }

    std::tuple<adm::Azimuth, adm::X, adm::Y> azXY(float az, float x, float y) {
        return std::make_tuple(adm::Azimuth{az}, adm::X{x}, adm::Y{y});
    }

    std::vector<std::vector<double>> getRandomCoordinates(double min, double max, int count) {
        std::vector<std::vector<double>> coordinates;
        coordinates.reserve(count);
        std::random_device r;
        auto seed = r();
        INFO("Seed: " + std::to_string(seed));
        std::default_random_engine engine{seed};
        std::uniform_real_distribution<double> dist{min, max};
        for(auto i = 0; i != count; ++i) {
            coordinates.push_back({dist(engine), dist(engine), dist(engine)});
        }
        return coordinates;
    }

    void checkThatAllClose(adm::Extent const& input, adm::Extent const& output, adm::Extent const& expected, double eps = 1e-10) {
        adm::Width inW, outW, expectW;
        adm::Height inH, outH, expectH;
        adm::Depth inD, outD, expectD;
        std::tie(inW, inH, inD) = input;
        std::tie(outW, outH, outD) = output;
        std::tie(expectW, expectH, expectD) = expected;
        std::stringstream ss;
        ss << "input : (" << inW << ", " << inH << ", " << inD << ")\n";
        ss << "output : (" << outW << ", " << outH << ", " << outD << ")\n";
        ss << "expected : (" << expectW << ", " << expectH << ", " << expectD << ")\n";
        INFO(ss.str());
        using Catch::Matchers::WithinAbs;
        CHECK(outW.get() == Approx(expectW.get()).margin(eps));
        CHECK(outH.get() == Approx(expectH.get()).margin(eps));
        CHECK(outD.get() == Approx(expectD.get()).margin(eps));
    }

    void checkThatAllClose(adm::SphericalPosition const& input, adm::CartesianPosition const& output, adm::CartesianPosition const& expected, double eps = 1e-10) {
        adm::X outX, expectX;
        adm::Y outY, expectY;
        adm::Z outZ, expectZ;
        adm::Azimuth inAz;
        adm::Elevation inEl;
        adm::Distance inD;
        std::tie(inAz, inEl, inD) = std::make_tuple(input.get<adm::Azimuth>(), input.get<adm::Elevation>(), input.get<adm::Distance>());
        std::tie(outX, outY, outZ) = std::make_tuple(output.get<adm::X>(), output.get<adm::Y>(), output.get<adm::Z>());
        std::tie(expectX, expectY, expectZ) = std::make_tuple(expected.get<adm::X>(), expected.get<adm::Y>(), expected.get<adm::Z>());

        std::stringstream ss;
        ss << "input (polar)   : (" << inAz << ", " << inEl << ", " << inD << ")\n";
        ss << "output (cart)   : (" << outX << ", " << outY << ", " << outZ << ")\n";
        ss << "expected (cart) : (" << expectX << ", " << expectY << ", " << expectZ << ")\n";
        INFO(ss.str());

        using Catch::Matchers::WithinAbs;
        CHECK(outX.get() == Approx(expectX.get()).margin(eps));
        CHECK(outY.get() == Approx(expectY.get()).margin(eps));
        CHECK(outZ.get() == Approx(expectZ.get()).margin(eps));
    }

    void checkThatAllClose(adm::CartesianPosition const& input, adm::SphericalPosition const& output, adm::SphericalPosition const& expected, double eps = 1e-10) {
        adm::Azimuth outAz, expectAz;
        adm::Elevation outEl, expectEl;
        adm::Distance outD, expectD;
        adm::X inX;
        adm::Y inY;
        adm::Z inZ;
        std::tie(outAz, outEl, outD) = std::make_tuple(output.get<adm::Azimuth>(),
                                                       output.get<adm::Elevation>(),
                                                       output.get<adm::Distance>());
        std::tie(inX, inY, inZ) = std::make_tuple(input.get<adm::X>(),
                                                  input.get<adm::Y>(),
                                                  input.get<adm::Z>());
        std::tie(expectAz, expectEl, expectD) = std::make_tuple(expected.get<adm::Azimuth>(),
                                                                expected.get<adm::Elevation>(),
                                                                expected.get<adm::Distance>());

        std::stringstream ss;
        ss << "input (cartPos) (cartExtent)     : (" << inX << ", " << inY << ", " << inZ << ")\n";
        ss << "output (polarPos) (polarExtent)   : (" << outEl << ", " << outAz << ", " << outD << ")\n";
        ss << "expected (polarPos) : (" << expectEl << ", " << expectAz << ", " << expectD << ")\n";
        INFO(ss.str());

        using Catch::Matchers::WithinAbs;
        CHECK(outAz.get() == Approx(expectAz.get()).margin(eps));
        CHECK(outEl.get() == Approx(expectEl.get()).margin(eps));
        CHECK(outD.get() == Approx(expectD.get()).margin(eps));
    }
}

TEST_CASE("Test conversion corners") {
    std::array<std::tuple<adm::Elevation, adm::Z>, 3> el_z {
            elZ(-30, -1),
            elZ(0, 0),
            elZ(30,1)
    };

    std::array<std::tuple<adm::Azimuth, adm::X, adm::Y>, 5> az_x_y{
            azXY(0, 0, 1),
            azXY(-30, 1, 1),
            azXY(30, -1, 1),
            azXY(-110, 1, -1),
            azXY(110, -1, -1)
    };

    std::array<adm::Distance, 3> distances{
            adm::Distance{0.5f},
            adm::Distance{1},
            adm::Distance{2}
    };

    {
        adm::Elevation el;
        adm::Z z;
        for(auto const& el_z_pair : el_z) {
            std::tie(el, z) = el_z_pair;
            adm::Azimuth az;
            adm::X x;
            adm::Y y;
            for(auto const& az_x_y_tuple : az_x_y) {
                std::tie(az, x, y) = az_x_y_tuple;
                for(auto const& d : distances) {
                    if(el == Approx(0) || az != Approx(0)) {
                        auto polar = adm::SphericalPosition(az, el, d);
                        auto convertedCart = pointPolarToCart(polar);
                        auto expectedCart = adm::CartesianPosition{adm::X{x.get() * d.get()},
                                                                   adm::Y{y.get() * d.get()},
                                                                   adm::Z{z.get() * d.get()}};
                        checkThatAllClose(polar,
                                          pointPolarToCart(polar),
                                          expectedCart);
                        auto convertedPolar = pointCartToPolar(expectedCart);
                        checkThatAllClose(expectedCart,
                                          convertedPolar,
                                          polar);
                    }
                }
            }
        }
    }
}
TEST_CASE("Test conversion poles") {
    using namespace adm;
    std::array<float, 2> minusPlus{-1.0, 1.0};
    std::array<float, 3> distances{0.5, 1.0, 2.0};
    for(auto sign : minusPlus) {
        for(auto d : distances) {
            auto polar = adm::SphericalPosition(Azimuth{0}, Elevation{sign * 90.0f}, Distance{d});
            auto convertedCart = pointPolarToCart(polar);
            auto expectedCart = adm::CartesianPosition(X{0}, Y{0}, Z{sign * d});
            checkThatAllClose(polar,
                              convertedCart,
                              expectedCart);
            auto convertedPolar = pointCartToPolar(expectedCart);
            checkThatAllClose(expectedCart,
                              convertedPolar,
                              polar);
        }
    }
}

TEST_CASE("Test conversion centre") {
    using namespace adm;
    std::array<float, 3> azimuths{-90, 0, 90};
    std::array<float, 3> elevations{-90, 0, 90};
    for(auto az : azimuths) {
        for(auto el : elevations) {
            auto polar = adm::SphericalPosition(Azimuth{az}, Elevation{el}, Distance(0.0f));
            auto convertedCart = pointPolarToCart(polar);
            auto expectedCart = adm::CartesianPosition(X{0.0f}, Y{0.0f}, Z{0.0f});
            checkThatAllClose(polar,
                              convertedCart,
                              expectedCart);
        }
    }
    auto convertedPolar = pointCartToPolar(adm::CartesianPosition(adm::X{0.f},
                                                                  adm::Y{0.f},
                                                                  adm::Z{0.f}));
    auto convertedDistance = convertedPolar.get<adm::Distance>().get();
    REQUIRE(convertedDistance == Approx(0.0));
}


// using the double versions of the function to check consistency with the python code.
// The intermediate conversion to float via the adm types can result in an error of the order 1e-7 which would fail test
TEST_CASE("Test conversion reversible") {
    auto cartPositions = getRandomCoordinates(-2, 1, 1000);
    for(auto const& pos : cartPositions) {
        INFO("(x, y, z): " << "(" << pos[0] << ", " << pos[1] << ", " << pos[2] << ")");
        double az{}, el{}, d{};
        REQUIRE_NOTHROW(adm::pointCartToPolar(pos[0], pos[1], pos[2], az, el, d));
        INFO("(az, el, d): " << "(" << az << ", " << el << ", " << d << ")");
        double x{}, y{}, z{};
        REQUIRE_NOTHROW(adm::pointPolarToCart(az, el, d, x, y, z));
        checkThatAllClose(makePolar(az, el, d), makeCart(x, y, z), makeCart(pos[0], pos[1], pos[2]), 1e-10);
    }
}

namespace {
    boost::optional<std::tuple<adm::CartesianPosition, adm::Extent, adm::SphericalPosition, adm::Extent>> nextPosition(std::istream& stream) {
        std::string line{};
        std::getline(stream, line);
        std::stringstream lineStream{line};
        float val;
        std::vector<float> values;
        while(lineStream >> val) {
            values.push_back(val);
            char _;
            lineStream >> _;
        }
        if(values.size() != 12) {
            return {};
        }
        auto cartesianPosition = adm::CartesianPosition{adm::X{values[0]},
                                                         adm::Y{values[1]},
                                                         adm::Z(values[2])};
        // flipped as ear works with x,y,z extent and libadm w,h,d
        // in cartesian h = z, d = y
        auto cartesianExtent = adm::Extent{adm::Width{values[3]},
                                           adm::Height{values[5]},
                                           adm::Depth{values[4]}};
        auto sphericalPosition = adm::SphericalPosition{adm::Azimuth{values[6]},
                                                        adm::Elevation{values[7]},
                                                        adm::Distance{values[8]}};
        auto sphericalExtent = adm::Extent{adm::Width{values[9]},
                                           adm::Height{values[10]},
                                           adm::Depth{values[11]}};

        return {std::make_tuple(cartesianPosition, cartesianExtent, sphericalPosition, sphericalExtent)};
    }
}

TEST_CASE("Test cartesian to polar extent conversion consistent with python EAR") {
    std::ifstream testData{"data/pos_extent_cart2polar.txt"};
    REQUIRE(testData.good());
    auto positions = nextPosition(testData);
    adm::CartesianPosition inputPosition;
    adm::Extent inputExtent;
    adm::SphericalPosition expectedOutput;
    adm::Extent expectedExtent;
    int n{100};
    while(positions) {
        std::tie(inputPosition, inputExtent, expectedOutput, expectedExtent) = *positions;
        auto output = cartToPolar(std::make_tuple(inputPosition, inputExtent));
        checkThatAllClose(inputExtent, std::get<1>(output), expectedExtent, 10e-5);
        --n;
        positions = nextPosition(testData);
    }
    REQUIRE(n == 0);
}
