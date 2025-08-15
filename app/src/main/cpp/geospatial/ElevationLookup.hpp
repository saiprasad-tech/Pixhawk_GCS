#pragma once

namespace pixhawk {

class ElevationLookup {
public:
    ElevationLookup();
    ~ElevationLookup();
    
    bool initialize();
    double getElevation(double lat, double lon) const;
    
private:
    bool initialized = false;
};

} // namespace pixhawk