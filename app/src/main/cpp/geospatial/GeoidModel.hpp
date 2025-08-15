#pragma once

namespace pixhawk {

class GeoidModel {
public:
    GeoidModel();
    ~GeoidModel();
    
    bool initialize();
    double getGeoidHeight(double lat, double lon) const;
    double getGeoidSeparation(double lat, double lon) const;
    
private:
    bool initialized = false;
};

} // namespace pixhawk