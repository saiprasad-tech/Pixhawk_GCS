#pragma once

#include <vector>
#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <chrono>

namespace pixhawk {

enum class MessageType : int {
    HEARTBEAT = 0,
    ATTITUDE = 1,
    GPS = 2,
    BATTERY = 3
};

struct TelemetryMessage {
    MessageType type;
    int64_t timestamp_ms;
    int32_t seq;
    
    // Union-like data fields - interpretation depends on type
    struct {
        double yaw, pitch, roll;        // ATTITUDE
        double lat, lon, alt;           // GPS  
        double voltage, current;        // BATTERY
        int32_t remaining;              // BATTERY percentage
        bool armed;                     // HEARTBEAT
        char mode[16];                  // HEARTBEAT mode string
    } data;
    
    TelemetryMessage();
};

struct TelemetryStats {
    double rate_hz;
    double avg_altitude;
    double avg_batt_v;
    int64_t window_start_ms;
    int32_t message_count;
};

class TelemetryEngine {
public:
    TelemetryEngine();
    ~TelemetryEngine();
    
    // Control methods
    bool start();
    void stop();
    bool isRunning() const;
    
    // Data access methods  
    std::vector<TelemetryMessage> getBatch(int maxCount);
    TelemetryStats getStats();
    
private:
    static constexpr int RING_BUFFER_SIZE = 10000;
    static constexpr int STATS_WINDOW_MS = 5000;
    static constexpr int TICK_INTERVAL_MS = 100; // 10 Hz
    
    // Ring buffer for messages
    TelemetryMessage ringBuffer[RING_BUFFER_SIZE];
    std::atomic<int> writeIndex{0};
    std::atomic<int> readIndex{0};
    
    // Thread control
    std::atomic<bool> running{false};
    std::thread workerThread;
    
    // Statistics tracking
    mutable std::mutex statsMutex;
    std::vector<int64_t> recentTimestamps;
    std::vector<double> recentAltitudes;
    std::vector<double> recentBatteryVoltages;
    
    // Sequence counter
    std::atomic<int32_t> messageSeq{0};
    
    // Worker thread function
    void workerLoop();
    
    // Generate simulated telemetry
    void pushSimulatedTick();
    
    // Helper methods
    void updateStats(const TelemetryMessage& msg);
    void cleanOldStats(int64_t currentTime);
    TelemetryMessage createHeartbeat();
    TelemetryMessage createAttitude(); 
    TelemetryMessage createGps();
    TelemetryMessage createBattery();
    
    // Simulation state
    double simTime = 0.0;
    bool simArmed = false;
    double simBatteryVoltage = 12.6;
    double simAltitude = 100.0;
    double simLatitude = 37.7749;
    double simLongitude = -122.4194;
};

} // namespace pixhawk