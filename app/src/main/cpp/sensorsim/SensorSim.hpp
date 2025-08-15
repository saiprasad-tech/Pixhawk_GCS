#pragma once

namespace pixhawk {

class SensorSim {
public:
    SensorSim();
    ~SensorSim();
    
    void generateAccelData(double& x, double& y, double& z);
    void generateGyroData(double& x, double& y, double& z);
    void generateMagData(double& x, double& y, double& z);
    
private:
    double time = 0.0;
};

} // namespace pixhawk