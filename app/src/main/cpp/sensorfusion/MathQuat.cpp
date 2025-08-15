#include "MathQuat.hpp"
#include <cmath>

namespace pixhawk {

MathQuat::MathQuat() : w(1.0), x(0.0), y(0.0), z(0.0) {}

MathQuat::MathQuat(double w, double x, double y, double z) : w(w), x(x), y(y), z(z) {}

MathQuat MathQuat::operator*(const MathQuat& other) const {
    return MathQuat(
        w * other.w - x * other.x - y * other.y - z * other.z,
        w * other.x + x * other.w + y * other.z - z * other.y,
        w * other.y - x * other.z + y * other.w + z * other.x,
        w * other.z + x * other.y - y * other.x + z * other.w
    );
}

MathQuat MathQuat::conjugate() const {
    return MathQuat(w, -x, -y, -z);
}

void MathQuat::normalize() {
    double norm = std::sqrt(w*w + x*x + y*y + z*z);
    if (norm > 0.0) {
        w /= norm;
        x /= norm;
        y /= norm;
        z /= norm;
    }
}

void MathQuat::toEuler(double& roll, double& pitch, double& yaw) const {
    // Roll (x-axis rotation)
    double sinr_cosp = 2 * (w * x + y * z);
    double cosr_cosp = 1 - 2 * (x * x + y * y);
    roll = std::atan2(sinr_cosp, cosr_cosp);

    // Pitch (y-axis rotation)
    double sinp = 2 * (w * y - z * x);
    if (std::abs(sinp) >= 1) {
        pitch = std::copysign(M_PI / 2, sinp); // use 90 degrees if out of range
    } else {
        pitch = std::asin(sinp);
    }

    // Yaw (z-axis rotation)
    double siny_cosp = 2 * (w * z + x * y);
    double cosy_cosp = 1 - 2 * (y * y + z * z);
    yaw = std::atan2(siny_cosp, cosy_cosp);
}

MathQuat MathQuat::fromEuler(double roll, double pitch, double yaw) {
    double cr = std::cos(roll * 0.5);
    double sr = std::sin(roll * 0.5);
    double cp = std::cos(pitch * 0.5);
    double sp = std::sin(pitch * 0.5);
    double cy = std::cos(yaw * 0.5);
    double sy = std::sin(yaw * 0.5);

    MathQuat q;
    q.w = cr * cp * cy + sr * sp * sy;
    q.x = sr * cp * cy - cr * sp * sy;
    q.y = cr * sp * cy + sr * cp * sy;
    q.z = cr * cp * sy - sr * sp * cy;

    return q;
}

} // namespace pixhawk