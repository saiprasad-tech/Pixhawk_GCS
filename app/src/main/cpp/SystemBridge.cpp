#include <jni.h>
#include <string>
#include <sstream>
#include <memory>
#include <android/asset_manager_jni.h>

// Include all our headers
#include "telemetry/TelemetryEngine.hpp"
#include "navigation/NavigationEngine.hpp"
#include "sensorsim/SensorSim.hpp"
#include "sensorfusion/EkfAttitude.hpp"
#include "sensorfusion/MathQuat.hpp"
#include "geospatial/GeoidModel.hpp"
#include "geospatial/MagneticModel.hpp"
#include "geospatial/ElevationLookup.hpp"
#include "logparser/LogParser.hpp"

#ifdef PIXHAWKCORE_VERBOSE
#include <android/log.h>
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "SystemBridge", __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "SystemBridge", __VA_ARGS__)
#else
#define LOGI(...) 
#define LOGE(...)
#endif

using namespace pixhawk;

// Global instances
static std::unique_ptr<TelemetryEngine> g_telemetryEngine;
static std::unique_ptr<NavigationEngine> g_navigationEngine;
static std::unique_ptr<SensorSim> g_sensorSim;
static std::unique_ptr<EkfAttitude> g_ekfAttitude;
static std::unique_ptr<GeoidModel> g_geoidModel;
static std::unique_ptr<MagneticModel> g_magneticModel;
static std::unique_ptr<ElevationLookup> g_elevationLookup;
static std::unique_ptr<LogParser> g_logParser;

static bool g_systemsInitialized = false;

// Helper function to create JSON responses
std::string createJsonResponse(bool success, const std::string& data = "", const std::string& error = "") {
    std::ostringstream json;
    json << "{\"ok\":" << (success ? "true" : "false");
    
    if (!data.empty()) {
        json << "," << data;
    }
    
    if (!error.empty()) {
        json << ",\"error\":\"" << error << "\"";
    }
    
    json << "}";
    return json.str();
}

extern "C" {

JNIEXPORT jstring JNICALL
Java_com_pixhawk_gcslab_SystemBridge_initSystems(JNIEnv *env, jobject /* this */, jobject assetManager) {
    LOGI("Initializing systems");
    
    try {
        // Initialize all subsystems
        g_telemetryEngine = std::make_unique<TelemetryEngine>();
        g_navigationEngine = std::make_unique<NavigationEngine>();
        g_sensorSim = std::make_unique<SensorSim>();
        g_ekfAttitude = std::make_unique<EkfAttitude>();
        g_geoidModel = std::make_unique<GeoidModel>();
        g_magneticModel = std::make_unique<MagneticModel>();
        g_elevationLookup = std::make_unique<ElevationLookup>();
        g_logParser = std::make_unique<LogParser>();
        
        // Initialize components that need it
        if (!g_navigationEngine->initialize()) {
            return env->NewStringUTF(createJsonResponse(false, "", "Failed to initialize navigation engine").c_str());
        }
        
        if (!g_geoidModel->initialize()) {
            return env->NewStringUTF(createJsonResponse(false, "", "Failed to initialize geoid model").c_str());
        }
        
        if (!g_magneticModel->initialize()) {
            return env->NewStringUTF(createJsonResponse(false, "", "Failed to initialize magnetic model").c_str());
        }
        
        if (!g_elevationLookup->initialize()) {
            return env->NewStringUTF(createJsonResponse(false, "", "Failed to initialize elevation lookup").c_str());
        }
        
        g_systemsInitialized = true;
        LOGI("All systems initialized successfully");
        
        return env->NewStringUTF(createJsonResponse(true).c_str());
        
    } catch (const std::exception& e) {
        LOGE("Exception during initialization: %s", e.what());
        return env->NewStringUTF(createJsonResponse(false, "", std::string("Exception: ") + e.what()).c_str());
    }
}

JNIEXPORT jstring JNICALL
Java_com_pixhawk_gcslab_SystemBridge_startTelemetry(JNIEnv *env, jobject /* this */) {
    LOGI("Starting telemetry");
    
    if (!g_systemsInitialized || !g_telemetryEngine) {
        return env->NewStringUTF(createJsonResponse(false, "", "Systems not initialized").c_str());
    }
    
    try {
        bool success = g_telemetryEngine->start();
        if (success) {
            LOGI("Telemetry started successfully");
            return env->NewStringUTF(createJsonResponse(true).c_str());
        } else {
            LOGE("Failed to start telemetry");
            return env->NewStringUTF(createJsonResponse(false, "", "Failed to start telemetry engine").c_str());
        }
    } catch (const std::exception& e) {
        LOGE("Exception starting telemetry: %s", e.what());
        return env->NewStringUTF(createJsonResponse(false, "", std::string("Exception: ") + e.what()).c_str());
    }
}

JNIEXPORT jstring JNICALL
Java_com_pixhawk_gcslab_SystemBridge_stopTelemetry(JNIEnv *env, jobject /* this */) {
    LOGI("Stopping telemetry");
    
    if (!g_systemsInitialized || !g_telemetryEngine) {
        return env->NewStringUTF(createJsonResponse(false, "", "Systems not initialized").c_str());
    }
    
    try {
        g_telemetryEngine->stop();
        LOGI("Telemetry stopped successfully");
        return env->NewStringUTF(createJsonResponse(true).c_str());
    } catch (const std::exception& e) {
        LOGE("Exception stopping telemetry: %s", e.what());
        return env->NewStringUTF(createJsonResponse(false, "", std::string("Exception: ") + e.what()).c_str());
    }
}

JNIEXPORT jstring JNICALL
Java_com_pixhawk_gcslab_SystemBridge_getTelemetryBatch(JNIEnv *env, jobject /* this */, jint maxCount) {
    if (!g_systemsInitialized || !g_telemetryEngine) {
        return env->NewStringUTF(createJsonResponse(false, "", "Systems not initialized").c_str());
    }
    
    try {
        auto messages = g_telemetryEngine->getBatch(maxCount);
        
        std::ostringstream dataJson;
        dataJson << "\"messages\":[";
        
        for (size_t i = 0; i < messages.size(); ++i) {
            if (i > 0) dataJson << ",";
            
            const auto& msg = messages[i];
            dataJson << "{";
            dataJson << "\"type\":\"";
            
            switch (msg.type) {
                case MessageType::HEARTBEAT:
                    dataJson << "HEARTBEAT\",";
                    dataJson << "\"seq\":" << msg.seq << ",";
                    dataJson << "\"ts_ms\":" << msg.timestamp_ms << ",";
                    dataJson << "\"mode\":\"" << msg.data.mode << "\",";
                    dataJson << "\"armed\":" << (msg.data.armed ? "true" : "false");
                    break;
                case MessageType::ATTITUDE:
                    dataJson << "ATTITUDE\",";
                    dataJson << "\"seq\":" << msg.seq << ",";
                    dataJson << "\"ts_ms\":" << msg.timestamp_ms << ",";
                    dataJson << "\"yaw\":" << msg.data.yaw << ",";
                    dataJson << "\"pitch\":" << msg.data.pitch << ",";
                    dataJson << "\"roll\":" << msg.data.roll;
                    break;
                case MessageType::GPS:
                    dataJson << "GPS\",";
                    dataJson << "\"seq\":" << msg.seq << ",";
                    dataJson << "\"ts_ms\":" << msg.timestamp_ms << ",";
                    dataJson << "\"lat\":" << msg.data.lat << ",";
                    dataJson << "\"lon\":" << msg.data.lon << ",";
                    dataJson << "\"alt\":" << msg.data.alt;
                    break;
                case MessageType::BATTERY:
                    dataJson << "BATTERY\",";
                    dataJson << "\"seq\":" << msg.seq << ",";
                    dataJson << "\"ts_ms\":" << msg.timestamp_ms << ",";
                    dataJson << "\"voltage\":" << msg.data.voltage << ",";
                    dataJson << "\"current\":" << msg.data.current << ",";
                    dataJson << "\"remaining\":" << msg.data.remaining;
                    break;
            }
            
            dataJson << "}";
        }
        
        dataJson << "]";
        
        return env->NewStringUTF(createJsonResponse(true, dataJson.str()).c_str());
        
    } catch (const std::exception& e) {
        LOGE("Exception getting telemetry batch: %s", e.what());
        return env->NewStringUTF(createJsonResponse(false, "", std::string("Exception: ") + e.what()).c_str());
    }
}

JNIEXPORT jstring JNICALL
Java_com_pixhawk_gcslab_SystemBridge_getTelemetryStats(JNIEnv *env, jobject /* this */) {
    if (!g_systemsInitialized || !g_telemetryEngine) {
        return env->NewStringUTF(createJsonResponse(false, "", "Systems not initialized").c_str());
    }
    
    try {
        auto stats = g_telemetryEngine->getStats();
        
        std::ostringstream dataJson;
        dataJson << "\"rate_hz\":" << stats.rate_hz << ",";
        dataJson << "\"avg_altitude\":" << stats.avg_altitude << ",";
        dataJson << "\"avg_batt_v\":" << stats.avg_batt_v << ",";
        dataJson << "\"message_count\":" << stats.message_count;
        
        return env->NewStringUTF(createJsonResponse(true, dataJson.str()).c_str());
        
    } catch (const std::exception& e) {
        LOGE("Exception getting telemetry stats: %s", e.what());
        return env->NewStringUTF(createJsonResponse(false, "", std::string("Exception: ") + e.what()).c_str());
    }
}

// Additional legacy methods
JNIEXPORT jstring JNICALL
Java_com_pixhawk_gcslab_SystemBridge_getAttitude(JNIEnv *env, jobject /* this */) {
    if (!g_systemsInitialized || !g_ekfAttitude) {
        return env->NewStringUTF(createJsonResponse(false, "", "Systems not initialized").c_str());
    }
    
    try {
        double roll, pitch, yaw;
        g_ekfAttitude->getEulerAngles(roll, pitch, yaw);
        
        std::ostringstream dataJson;
        dataJson << "\"roll\":" << roll << ",";
        dataJson << "\"pitch\":" << pitch << ",";
        dataJson << "\"yaw\":" << yaw;
        
        return env->NewStringUTF(createJsonResponse(true, dataJson.str()).c_str());
    } catch (const std::exception& e) {
        return env->NewStringUTF(createJsonResponse(false, "", std::string("Exception: ") + e.what()).c_str());
    }
}

JNIEXPORT jstring JNICALL
Java_com_pixhawk_gcslab_SystemBridge_getPath(JNIEnv *env, jobject /* this */) {
    if (!g_systemsInitialized || !g_navigationEngine) {
        return env->NewStringUTF(createJsonResponse(false, "", "Systems not initialized").c_str());
    }
    
    try {
        double lat, lon, alt;
        g_navigationEngine->getCurrentPosition(lat, lon, alt);
        
        std::ostringstream dataJson;
        dataJson << "\"lat\":" << lat << ",";
        dataJson << "\"lon\":" << lon << ",";
        dataJson << "\"alt\":" << alt;
        
        return env->NewStringUTF(createJsonResponse(true, dataJson.str()).c_str());
    } catch (const std::exception& e) {
        return env->NewStringUTF(createJsonResponse(false, "", std::string("Exception: ") + e.what()).c_str());
    }
}

JNIEXPORT jstring JNICALL
Java_com_pixhawk_gcslab_SystemBridge_getDeclination(JNIEnv *env, jobject /* this */, jdouble lat, jdouble lon) {
    if (!g_systemsInitialized || !g_magneticModel) {
        return env->NewStringUTF(createJsonResponse(false, "", "Systems not initialized").c_str());
    }
    
    try {
        double declination = g_magneticModel->getDeclination(lat, lon);
        
        std::ostringstream dataJson;
        dataJson << "\"declination\":" << declination;
        
        return env->NewStringUTF(createJsonResponse(true, dataJson.str()).c_str());
    } catch (const std::exception& e) {
        return env->NewStringUTF(createJsonResponse(false, "", std::string("Exception: ") + e.what()).c_str());
    }
}

JNIEXPORT jstring JNICALL
Java_com_pixhawk_gcslab_SystemBridge_getGeoidSeparation(JNIEnv *env, jobject /* this */, jdouble lat, jdouble lon) {
    if (!g_systemsInitialized || !g_geoidModel) {
        return env->NewStringUTF(createJsonResponse(false, "", "Systems not initialized").c_str());
    }
    
    try {
        double separation = g_geoidModel->getGeoidSeparation(lat, lon);
        
        std::ostringstream dataJson;
        dataJson << "\"separation\":" << separation;
        
        return env->NewStringUTF(createJsonResponse(true, dataJson.str()).c_str());
    } catch (const std::exception& e) {
        return env->NewStringUTF(createJsonResponse(false, "", std::string("Exception: ") + e.what()).c_str());
    }
}

JNIEXPORT jstring JNICALL
Java_com_pixhawk_gcslab_SystemBridge_getLogSummary(JNIEnv *env, jobject /* this */, jstring logData) {
    if (!g_systemsInitialized || !g_logParser) {
        return env->NewStringUTF(createJsonResponse(false, "", "Systems not initialized").c_str());
    }
    
    try {
        const char* logStr = env->GetStringUTFChars(logData, nullptr);
        std::string logString(logStr);
        env->ReleaseStringUTFChars(logData, logStr);
        
        bool parsed = g_logParser->parseLogFile(logString);
        if (parsed) {
            std::string summary = g_logParser->getSummary();
            
            std::ostringstream dataJson;
            dataJson << "\"summary\":\"" << summary << "\",";
            dataJson << "\"entry_count\":" << g_logParser->getEntryCount();
            
            return env->NewStringUTF(createJsonResponse(true, dataJson.str()).c_str());
        } else {
            return env->NewStringUTF(createJsonResponse(false, "", "Failed to parse log data").c_str());
        }
    } catch (const std::exception& e) {
        return env->NewStringUTF(createJsonResponse(false, "", std::string("Exception: ") + e.what()).c_str());
    }
}

} // extern "C"