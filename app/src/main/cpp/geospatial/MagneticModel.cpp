#include "MagneticModel.hpp"
#include <cmath>

namespace pixhawk {

MagneticModel::MagneticModel() = default;
MagneticModel::~MagneticModel() = default;

bool MagneticModel::initialize() {
    initialized = true;
    return true;
}

double MagneticModel::getDeclination(double lat, double lon, double alt) const {
    if (!initialized) return 0.0;
    
    // Simplified magnetic declination approximation
    // Real implementation would use WMM coefficients
    double dlat = lat * M_PI / 180.0;
    double dlon = lon * M_PI / 180.0;
    
    return 15.0 * std::sin(dlat) * std::cos(dlon * 2.0) + 2.0 * std::cos(dlat);
}

double MagneticModel::getInclination(double lat, double lon, double alt) const {
    if (!initialized) return 0.0;
    
    // Simplified magnetic inclination
    return 60.0 * std::sin(lat * M_PI / 180.0);
}

double MagneticModel::getIntensity(double lat, double lon, double alt) const {
    if (!initialized) return 50000.0; // nT
    
    // Simplified total field intensity
    double dlat = lat * M_PI / 180.0;
    return 50000.0 + 5000.0 * std::cos(dlat);
}

} // namespace pixhawk