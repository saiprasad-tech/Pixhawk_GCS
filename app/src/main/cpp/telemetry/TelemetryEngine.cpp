#include "TelemetryEngine.hpp"
#include <cmath>
#include <random>
#include <cstring>
#include <algorithm>

#ifdef PIXHAWKCORE_VERBOSE
#include <android/log.h>
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "TelemetryEngine", __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "TelemetryEngine", __VA_ARGS__)
#else
#define LOGI(...) 
#define LOGE(...)
#endif

namespace pixhawk {

TelemetryMessage::TelemetryMessage() {
    type = MessageType::HEARTBEAT;
    timestamp_ms = 0;
    seq = 0;
    memset(&data, 0, sizeof(data));
    strcpy(data.mode, "MANUAL");
}

TelemetryEngine::TelemetryEngine() {
    LOGI("TelemetryEngine constructor");
    
    // Initialize ring buffer
    for (int i = 0; i < RING_BUFFER_SIZE; i++) {
        ringBuffer[i] = TelemetryMessage();
    }
}

TelemetryEngine::~TelemetryEngine() {
    LOGI("TelemetryEngine destructor");
    stop();
}

bool TelemetryEngine::start() {
    if (running.load()) {
        LOGI("TelemetryEngine already running");
        return true;
    }
    
    LOGI("Starting TelemetryEngine");
    running.store(true);
    
    try {
        workerThread = std::thread(&TelemetryEngine::workerLoop, this);
        LOGI("Worker thread started successfully");
        return true;
    } catch (const std::exception& e) {
        LOGE("Failed to start worker thread: %s", e.what());
        running.store(false);
        return false;
    }
}

void TelemetryEngine::stop() {
    if (!running.load()) {
        return;
    }
    
    LOGI("Stopping TelemetryEngine");
    running.store(false);
    
    if (workerThread.joinable()) {
        workerThread.join();
        LOGI("Worker thread joined successfully");
    }
}

bool TelemetryEngine::isRunning() const {
    return running.load();
}

std::vector<TelemetryMessage> TelemetryEngine::getBatch(int maxCount) {
    std::vector<TelemetryMessage> batch;
    
    if (maxCount <= 0) {
        return batch;
    }
    
    int currentWrite = writeIndex.load();
    int currentRead = readIndex.load();
    
    // Calculate available messages
    int available;
    if (currentWrite >= currentRead) {
        available = currentWrite - currentRead;
    } else {
        available = RING_BUFFER_SIZE - currentRead + currentWrite;
    }
    
    int count = std::min(available, maxCount);
    batch.reserve(count);
    
    // Read from ring buffer (oldest first)
    for (int i = 0; i < count; i++) {
        int idx = (currentRead + i) % RING_BUFFER_SIZE;
        batch.push_back(ringBuffer[idx]);
    }
    
    return batch;
}

TelemetryStats TelemetryEngine::getStats() {
    std::lock_guard<std::mutex> lock(statsMutex);
    
    TelemetryStats stats{};
    auto currentTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
    
    // Clean old data
    const_cast<TelemetryEngine*>(this)->cleanOldStats(currentTime);
    
    stats.window_start_ms = currentTime - STATS_WINDOW_MS;
    stats.message_count = static_cast<int32_t>(recentTimestamps.size());
    
    // Calculate rate
    if (stats.message_count > 1) {
        double timeSpan = (recentTimestamps.back() - recentTimestamps.front()) / 1000.0;
        if (timeSpan > 0) {
            stats.rate_hz = stats.message_count / timeSpan;
        }
    }
    
    // Calculate averages
    if (!recentAltitudes.empty()) {
        double sum = 0;
        for (double alt : recentAltitudes) {
            sum += alt;
        }
        stats.avg_altitude = sum / recentAltitudes.size();
    }
    
    if (!recentBatteryVoltages.empty()) {
        double sum = 0;
        for (double volt : recentBatteryVoltages) {
            sum += volt;
        }
        stats.avg_batt_v = sum / recentBatteryVoltages.size();
    }
    
    return stats;
}

void TelemetryEngine::workerLoop() {
    LOGI("Worker thread started");
    
    while (running.load()) {
        pushSimulatedTick();
        
        std::this_thread::sleep_for(std::chrono::milliseconds(TICK_INTERVAL_MS));
    }
    
    LOGI("Worker thread ended");
}

void TelemetryEngine::pushSimulatedTick() {
    simTime += TICK_INTERVAL_MS / 1000.0;
    
    // Generate different message types with different frequencies
    static int tickCount = 0;
    tickCount++;
    
    TelemetryMessage msg;
    
    if (tickCount % 10 == 0) {
        // Heartbeat every 1 second (10 ticks)
        msg = createHeartbeat();
    } else if (tickCount % 2 == 0) {
        // Attitude every 200ms (2 ticks) 
        msg = createAttitude();
    } else if (tickCount % 5 == 0) {
        // GPS every 500ms (5 ticks)
        msg = createGps();
    } else if (tickCount % 20 == 0) {
        // Battery every 2 seconds (20 ticks)
        msg = createBattery();
    } else {
        // Default to attitude for most ticks
        msg = createAttitude();
    }
    
    // Store in ring buffer
    int writeIdx = writeIndex.load();
    ringBuffer[writeIdx] = msg;
    writeIndex.store((writeIdx + 1) % RING_BUFFER_SIZE);
    
    // Update read index if we're overwriting
    int readIdx = readIndex.load();
    if (writeIdx == readIdx) {
        readIndex.store((readIdx + 1) % RING_BUFFER_SIZE);
    }
    
    // Update statistics
    updateStats(msg);
}

void TelemetryEngine::updateStats(const TelemetryMessage& msg) {
    std::lock_guard<std::mutex> lock(statsMutex);
    
    recentTimestamps.push_back(msg.timestamp_ms);
    
    if (msg.type == MessageType::GPS) {
        recentAltitudes.push_back(msg.data.alt);
    }
    
    if (msg.type == MessageType::BATTERY) {
        recentBatteryVoltages.push_back(msg.data.voltage);
    }
    
    cleanOldStats(msg.timestamp_ms);
}

void TelemetryEngine::cleanOldStats(int64_t currentTime) {
    int64_t cutoff = currentTime - STATS_WINDOW_MS;
    
    // Remove old timestamps
    auto it = std::lower_bound(recentTimestamps.begin(), recentTimestamps.end(), cutoff);
    recentTimestamps.erase(recentTimestamps.begin(), it);
    
    // For other vectors, we need to be more careful since they're not necessarily sorted
    // For simplicity, we'll just limit their size
    const size_t MAX_SIZE = 1000;
    if (recentAltitudes.size() > MAX_SIZE) {
        recentAltitudes.erase(recentAltitudes.begin(), recentAltitudes.begin() + (recentAltitudes.size() - MAX_SIZE));
    }
    if (recentBatteryVoltages.size() > MAX_SIZE) {
        recentBatteryVoltages.erase(recentBatteryVoltages.begin(), recentBatteryVoltages.begin() + (recentBatteryVoltages.size() - MAX_SIZE));
    }
}

TelemetryMessage TelemetryEngine::createHeartbeat() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<> dis(0.0, 1.0);
    
    TelemetryMessage msg;
    msg.type = MessageType::HEARTBEAT;
    msg.timestamp_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
    msg.seq = messageSeq.fetch_add(1);
    
    // Simulate mode changes occasionally
    if (dis(gen) < 0.05) {
        simArmed = !simArmed;
    }
    
    msg.data.armed = simArmed;
    if (simArmed) {
        strcpy(msg.data.mode, "STABILIZE");
    } else {
        strcpy(msg.data.mode, "MANUAL");
    }
    
    return msg;
}

TelemetryMessage TelemetryEngine::createAttitude() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<> noise(-0.5, 0.5);
    
    TelemetryMessage msg;
    msg.type = MessageType::ATTITUDE;
    msg.timestamp_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
    msg.seq = messageSeq.fetch_add(1);
    
    // Simulate gentle movement with noise
    msg.data.yaw = std::fmod(simTime * 2.0, 360.0) + noise(gen);
    msg.data.pitch = 5.0 * std::sin(simTime * 0.1) + noise(gen);
    msg.data.roll = 3.0 * std::cos(simTime * 0.15) + noise(gen);
    
    return msg;
}

TelemetryMessage TelemetryEngine::createGps() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<> noise(-0.00001, 0.00001);
    static std::uniform_real_distribution<> altNoise(-1.0, 1.0);
    
    TelemetryMessage msg;
    msg.type = MessageType::GPS;
    msg.timestamp_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
    msg.seq = messageSeq.fetch_add(1);
    
    // Simulate slow drift in position and altitude changes
    msg.data.lat = simLatitude + simTime * 0.0001 + noise(gen);
    msg.data.lon = simLongitude + simTime * 0.0001 + noise(gen);
    
    // Simulate altitude changes (climbing/descending)
    simAltitude += std::sin(simTime * 0.01) * 0.1 + altNoise(gen);
    simAltitude = std::max(0.0, simAltitude); // Don't go below ground
    msg.data.alt = simAltitude;
    
    return msg;
}

TelemetryMessage TelemetryEngine::createBattery() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<> noise(-0.05, 0.05);
    
    TelemetryMessage msg;
    msg.type = MessageType::BATTERY;
    msg.timestamp_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
    msg.seq = messageSeq.fetch_add(1);
    
    // Simulate slow battery drain
    simBatteryVoltage -= simTime * 0.0001; // Very slow drain
    simBatteryVoltage = std::max(10.0, simBatteryVoltage); // Don't go too low
    
    msg.data.voltage = simBatteryVoltage + noise(gen);
    msg.data.current = 5.0 + 2.0 * std::sin(simTime * 0.1) + noise(gen) * 0.5;
    
    // Calculate remaining percentage roughly
    double percentage = (simBatteryVoltage - 10.0) / (12.6 - 10.0) * 100.0;
    msg.data.remaining = static_cast<int32_t>(std::max(0.0, std::min(100.0, percentage)));
    
    return msg;
}

} // namespace pixhawk