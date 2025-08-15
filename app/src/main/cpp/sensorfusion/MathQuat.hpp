#pragma once

namespace pixhawk {

class MathQuat {
public:
    double w, x, y, z;
    
    MathQuat();
    MathQuat(double w, double x, double y, double z);
    
    MathQuat operator*(const MathQuat& other) const;
    MathQuat conjugate() const;
    void normalize();
    void toEuler(double& roll, double& pitch, double& yaw) const;
    
    static MathQuat fromEuler(double roll, double pitch, double yaw);
};

} // namespace pixhawk