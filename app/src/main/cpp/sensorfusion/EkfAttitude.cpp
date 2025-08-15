#include "EkfAttitude.hpp"
#include <cmath>

namespace pixhawk {

EkfAttitude::EkfAttitude() {
    orientation = MathQuat(1.0, 0.0, 0.0, 0.0); // Identity quaternion
}

EkfAttitude::~EkfAttitude() = default;

void EkfAttitude::predict(double dt, double gx, double gy, double gz) {
    // Simple integration of angular velocity
    MathQuat deltaQ = MathQuat::fromEuler(gx * dt, gy * dt, gz * dt);
    orientation = orientation * deltaQ;
    orientation.normalize();
}

void EkfAttitude::updateAccel(double ax, double ay, double az) {
    // Simplified accelerometer update - just normalize the measurement
    double norm = std::sqrt(ax*ax + ay*ay + az*az);
    if (norm > 0.1) { // Avoid division by very small numbers
        ax /= norm;
        ay /= norm;
        az /= norm;
        
        // This is a very simplified update - in reality would use proper EKF equations
        // Just doing a small correction towards gravity vector alignment
        const double alpha = 0.05; // Small correction factor
        
        // Expected gravity vector from current orientation
        double gx_expected = 2 * (orientation.x * orientation.z - orientation.w * orientation.y);
        double gy_expected = 2 * (orientation.w * orientation.x + orientation.y * orientation.z);
        double gz_expected = orientation.w*orientation.w - orientation.x*orientation.x - orientation.y*orientation.y + orientation.z*orientation.z;
        
        // Error between measured and expected gravity
        double error_x = az - gx_expected; // Note: assuming accelerometer z is down
        double error_y = ay - gy_expected;
        double error_z = ax - gz_expected;
        
        // Apply small correction (simplified)
        orientation.x += alpha * error_x;
        orientation.y += alpha * error_y; 
        orientation.z += alpha * error_z;
        orientation.normalize();
    }
}

void EkfAttitude::updateMag(double mx, double my, double mz) {
    // Simplified magnetometer update
    double norm = std::sqrt(mx*mx + my*my + mz*mz);
    if (norm > 0.01) {
        mx /= norm;
        my /= norm;
        mz /= norm;
        
        // Very simple heading correction - would be much more complex in reality
        const double beta = 0.02;
        orientation.z += beta * mx; // Simplified
        orientation.normalize();
    }
}

MathQuat EkfAttitude::getQuaternion() const {
    return orientation;
}

void EkfAttitude::getEulerAngles(double& roll, double& pitch, double& yaw) const {
    orientation.toEuler(roll, pitch, yaw);
}

} // namespace pixhawk