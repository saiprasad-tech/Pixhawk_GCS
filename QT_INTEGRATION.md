# Qt/QML Integration Guide

This document explains how to integrate and use the Qt/QML components added to the Pixhawk GCS project.

## Overview

The project now includes Qt/QML components that provide:
- **Map integration** with Google Maps/OSM fallback
- **Bluetooth Low Energy** device management
- **WiFi provisioning** for Raspberry Pi
- **MJPEG camera streaming**
- **Telemetry visualization** and drone tracking
- **Person tracking controls**

## Architecture

```
MainActivity (Native Android)
    └── QtMainActivity (Qt/QML Interface)
        ├── ConnectPage.qml      (BLE + WiFi setup)
        ├── MapPage.qml          (Drone location + maps)
        ├── CameraPage.qml       (Pi camera streaming)
        ├── SettingsPage.qml     (Configuration)
        └── SoftAPWizard.qml     (WiFi setup wizard)

C++ Bridge Classes:
├── TelemetryClient      (UDP/HTTP telemetry)
├── BluetoothManager     (BLE GATT communication)
├── CameraManager        (MJPEG streaming)
├── SettingsManager      (Persistent configuration)
└── WiFiManager          (SoftAP credentials)
```

## Qt Dependencies Required

To fully enable Qt/QML functionality, add these dependencies to `app/build.gradle.kts`:

```kotlin
dependencies {
    // Qt for Android
    implementation("org.qtproject:qt5-android:5.15.2")
    
    // Qt modules needed
    implementation("org.qtproject:qt5-qtquick:5.15.2")
    implementation("org.qtproject:qt5-qtlocation:5.15.2")
    implementation("org.qtproject:qt5-qtbluetooth:5.15.2")
    implementation("org.qtproject:qt5-qtnetwork:5.15.2")
    
    // Existing dependencies...
}
```

## C++ Bridge Registration

In `QtMainActivity.kt`, register the C++ bridge objects:

```kotlin
// Register C++ objects with QML
qmlEngine.rootContext().setContextProperty("telemetryClient", TelemetryClient())
qmlEngine.rootContext().setContextProperty("bluetoothManager", BluetoothManager())
qmlEngine.rootContext().setContextProperty("cameraManager", CameraManager())
qmlEngine.rootContext().setContextProperty("settingsManager", SettingsManager())
qmlEngine.rootContext().setContextProperty("wifiManager", WiFiManager())
```

## Key Features

### 1. Map Provider Fallback
- **Default**: OpenStreetMap (no API key required)
- **Optional**: Google Maps (requires API key in Settings)
- Maps show drone position from telemetry data
- User location display when GPS available

### 2. Telemetry Integration
- **UDP Mode**: Listen on port 14555 for JSON telemetry
- **HTTP Mode**: Poll Pi API for telemetry data
- Expected JSON format:
```json
{
  "lat": 37.7749,
  "lon": -122.4194,
  "alt": 120.5,
  "fix": "3D",
  "sats": 12
}
```

### 3. Bluetooth LE Provisioning
Default UUIDs (configurable in Settings):
- **Service**: `0000abcd-0000-1000-8000-00805f9b34fb`
- **SSID**: `0000abce-0000-1000-8000-00805f9b34fb`
- **Password**: `0000abcf-0000-1000-8000-00805f9b34fb`
- **Command**: `0000abd0-0000-1000-8000-00805f9b34fb`

Commands:
- `TRACK_START` - Begin person tracking
- `TRACK_STOP` - Stop person tracking

### 4. WiFi SoftAP Setup
Sends credentials to Pi via HTTP POST:
```
POST http://192.168.4.1/api/wifi
Content-Type: application/json

{
  "ssid": "YourWiFiNetwork",
  "password": "YourPassword",
  "security": "WPA2"
}
```

### 5. Camera Streaming
- **MJPEG support** with anti-cache mechanisms
- **Default URL**: `http://pi.local:8080/stream.mjpeg`
- **FPS monitoring** and connection testing
- **Automatic refresh** to prevent browser caching

## Raspberry Pi API Requirements

For full functionality, the Raspberry Pi should provide these HTTP endpoints:

### Telemetry API
```
GET http://pi.local:8000/api/telemetry
Response: {"lat": 37.7749, "lon": -122.4194, "alt": 120.5, "fix": "3D", "sats": 12}
```

### WiFi Configuration
```
POST http://192.168.4.1/api/wifi
Request: {"ssid": "network", "password": "pass", "security": "WPA2"}
Response: {"success": true}
```

### Person Tracking Control
```
POST http://pi.local:8000/api/tracking/start
POST http://pi.local:8000/api/tracking/stop
Response: {"success": true, "status": "tracking_started"}
```

### Camera Stream
```
GET http://pi.local:8080/stream.mjpeg
Response: Multipart MJPEG stream
```

## Settings Configuration

All settings are persisted using Qt's QSettings:
- **Google Maps API Key** (optional)
- **Pi Host/IP address**
- **Telemetry mode** (UDP/HTTP) and ports
- **Camera stream URL**
- **BLE GATT UUIDs**
- **Update rates** and timeouts

## Usage Flow

1. **Launch Qt Interface** from main activity
2. **Configure Settings** (Pi host, API key, etc.)
3. **Connect via Bluetooth** to scan and pair with Pi
4. **Provision WiFi** using BLE or SoftAP wizard
5. **View Map** with real-time drone telemetry
6. **Monitor Camera** stream from Pi
7. **Control Tracking** via BLE commands

## Integration Notes

- The Qt interface is launched as a separate activity alongside the existing native UI
- All C++ bridge classes are designed to work with or without Qt
- Settings persist between app launches
- The implementation gracefully handles network failures and device disconnections
- Maps work offline with OSM, no billing required

## Build Integration

To complete the integration:

1. **Add Qt Android** to the build system
2. **Configure CMake** to build the C++ bridge classes
3. **Register QML types** in the application startup
4. **Handle Android permissions** for Location, Bluetooth, and WiFi
5. **Test on device** with actual Pi hardware

This architecture provides a complete ground control station with modern Qt/QML UI while preserving the existing native Android functionality.