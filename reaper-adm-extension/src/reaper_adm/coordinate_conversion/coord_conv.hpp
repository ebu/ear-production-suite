#include <adm/adm.hpp>
#include <tuple>
namespace adm {
    using Extent = std::tuple<adm::Width, adm::Height, adm::Depth>;
    std::tuple<CartesianPosition, Extent> polarToCart(std::tuple<SphericalPosition, Extent> polar);
    std::tuple<SphericalPosition, Extent> cartToPolar(std::tuple<CartesianPosition, Extent> cart);

    adm::CartesianPosition pointPolarToCart(adm::SphericalPosition const& position);
    adm::SphericalPosition pointCartToPolar(adm::CartesianPosition const& position);

    adm::CartesianSpeakerPosition pointPolarToCart(adm::SphericalSpeakerPosition const& speakerPosition);
    adm::SphericalSpeakerPosition pointCartToPolar(adm::CartesianSpeakerPosition const& speakerPosition);

    // High precision versions
    void pointPolarToCart(double azimuth, double elevation, double distance, double& x, double& y, double& z);
    void pointCartToPolar(double x, double y, double z, double& azimuth, double& elevation, double& distance);

}
