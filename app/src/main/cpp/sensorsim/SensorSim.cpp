#include "SensorSim.hpp"
#include <cmath>

namespace pixhawk {

SensorSim::SensorSim() = default;
SensorSim::~SensorSim() = default;

void SensorSim::generateAccelData(double& x, double& y, double& z) {
    time += 0.01;
    x = 0.1 * std::sin(time);
    y = 0.1 * std::cos(time * 1.1);
    z = 9.81 + 0.05 * std::sin(time * 2.0);
}

void SensorSim::generateGyroData(double& x, double& y, double& z) {
    x = 0.01 * std::sin(time * 0.5);
    y = 0.02 * std::cos(time * 0.7);
    z = 0.005 * std::sin(time * 1.3);
}

void SensorSim::generateMagData(double& x, double& y, double& z) {
    x = 0.2 + 0.01 * std::sin(time * 0.2);
    y = 0.1 + 0.01 * std::cos(time * 0.3);
    z = 0.4 + 0.005 * std::sin(time * 0.1);
}

} // namespace pixhawk