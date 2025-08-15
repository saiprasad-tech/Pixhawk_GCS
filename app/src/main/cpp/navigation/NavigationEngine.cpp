#include "NavigationEngine.hpp"

namespace pixhawk {

NavigationEngine::NavigationEngine() = default;
NavigationEngine::~NavigationEngine() = default;

bool NavigationEngine::initialize() {
    return true;
}

void NavigationEngine::setDestination(double lat, double lon, double alt) {
    targetLat = lat;
    targetLon = lon;
    targetAlt = alt;
}

void NavigationEngine::getCurrentPosition(double& lat, double& lon, double& alt) {
    lat = currentLat;
    lon = currentLon;
    alt = currentAlt;
}

bool NavigationEngine::calculatePath() {
    return true;
}

} // namespace pixhawk