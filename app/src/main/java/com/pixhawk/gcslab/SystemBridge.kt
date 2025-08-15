package com.pixhawk.gcslab

/**
 * Bridge class for native C++ telemetry system integration
 */
class SystemBridge {
    companion object {
        init {
            System.loadLibrary("pixhawkcore")
        }
    }

    // Native method declarations
    external fun initSystems(assetManager: android.content.res.AssetManager): String
    external fun startTelemetry(): String
    external fun stopTelemetry(): String
    external fun getTelemetryBatch(maxCount: Int): String
    external fun getTelemetryStats(): String
    
    // Additional methods from existing CMakeLists.txt structure
    external fun getAttitude(): String
    external fun getPath(): String  
    external fun getDeclination(lat: Double, lon: Double): String
    external fun getGeoidSeparation(lat: Double, lon: Double): String
    external fun getLogSummary(logData: String): String
}