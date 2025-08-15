#pragma once
#include "MathQuat.hpp"

namespace pixhawk {

class EkfAttitude {
public:
    EkfAttitude();
    ~EkfAttitude();
    
    void predict(double dt, double gx, double gy, double gz);
    void updateAccel(double ax, double ay, double az);
    void updateMag(double mx, double my, double mz);
    
    MathQuat getQuaternion() const;
    void getEulerAngles(double& roll, double& pitch, double& yaw) const;
    
private:
    MathQuat orientation;
    double gyroNoise = 0.01;
    double accelNoise = 0.1;
    double magNoise = 0.05;
};

} // namespace pixhawk