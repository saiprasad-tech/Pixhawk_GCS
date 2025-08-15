#pragma once

namespace pixhawk {

class MagneticModel {
public:
    MagneticModel();
    ~MagneticModel();
    
    bool initialize();
    double getDeclination(double lat, double lon, double alt = 0.0) const;
    double getInclination(double lat, double lon, double alt = 0.0) const;
    double getIntensity(double lat, double lon, double alt = 0.0) const;
    
private:
    bool initialized = false;
};

} // namespace pixhawk