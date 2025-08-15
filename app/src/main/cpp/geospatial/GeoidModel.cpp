#include "GeoidModel.hpp"
#include <cmath>

namespace pixhawk {

GeoidModel::GeoidModel() = default;
GeoidModel::~GeoidModel() = default;

bool GeoidModel::initialize() {
    initialized = true;
    return true;
}

double GeoidModel::getGeoidHeight(double lat, double lon) const {
    if (!initialized) return 0.0;
    
    // Simple approximation for demonstration
    return -30.0 + 60.0 * std::sin(lat * M_PI / 180.0) * std::cos(lon * M_PI / 360.0);
}

double GeoidModel::getGeoidSeparation(double lat, double lon) const {
    return getGeoidHeight(lat, lon);
}

} // namespace pixhawk