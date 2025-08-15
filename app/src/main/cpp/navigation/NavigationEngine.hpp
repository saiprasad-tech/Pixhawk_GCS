#pragma once

namespace pixhawk {

class NavigationEngine {
public:
    NavigationEngine();
    ~NavigationEngine();
    
    bool initialize();
    void setDestination(double lat, double lon, double alt);
    void getCurrentPosition(double& lat, double& lon, double& alt);
    bool calculatePath();
    
private:
    double currentLat = 0.0;
    double currentLon = 0.0; 
    double currentAlt = 0.0;
    double targetLat = 0.0;
    double targetLon = 0.0;
    double targetAlt = 0.0;
};

} // namespace pixhawk