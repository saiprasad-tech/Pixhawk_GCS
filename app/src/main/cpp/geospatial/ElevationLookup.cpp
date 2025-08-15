#include "ElevationLookup.hpp"
#include <cmath>

namespace pixhawk {

ElevationLookup::ElevationLookup() = default;
ElevationLookup::~ElevationLookup() = default;

bool ElevationLookup::initialize() {
    initialized = true;
    return true;
}

double ElevationLookup::getElevation(double lat, double lon) const {
    if (!initialized) return 0.0;
    
    // Simple terrain elevation simulation
    double dlat = lat * M_PI / 180.0;
    double dlon = lon * M_PI / 180.0;
    
    // Create some hills and valleys
    double elevation = 500.0 + 200.0 * std::sin(dlat * 2.0) * std::cos(dlon * 1.5);
    elevation += 100.0 * std::sin(dlat * 5.0) * std::sin(dlon * 3.0);
    
    return std::max(0.0, elevation); // No negative elevations (below sea level)
}

} // namespace pixhawk