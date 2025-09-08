# Raspberry Pi API Specification

This document specifies the HTTP API endpoints that should be implemented on the Raspberry Pi to support the Pixhawk GCS Android application.

## Base Configuration

- **Primary API Port**: 8000 (configurable in app settings)
- **Camera Stream Port**: 8080 (configurable in app settings)
- **SoftAP Setup IP**: 192.168.4.1 (when Pi is in setup mode)

## API Endpoints

### 1. Health Check
```
GET /api/health
```

**Response:**
```json
{
  "success": true,
  "status": "running",
  "version": "1.0.0",
  "timestamp": "2024-01-15T10:30:00Z"
}
```

### 2. Telemetry Data
```
GET /api/telemetry
```

**Response:**
```json
{
  "success": true,
  "data": {
    "lat": 37.7749,
    "lon": -122.4194,
    "alt": 120.5,
    "fix": "3D",
    "sats": 12,
    "hdop": 1.2,
    "speed": 0.0,
    "heading": 0.0,
    "timestamp": "2024-01-15T10:30:00Z"
  }
}
```

**Fix Types:**
- `"No Fix"` - No GPS signal
- `"2D"` - 2D position fix (lat/lon only)
- `"3D"` - 3D position fix (lat/lon/alt)

### 3. Person Tracking Control

#### Start Tracking
```
POST /api/tracking/start
```

**Request Body:** (optional parameters)
```json
{
  "mode": "follow",
  "distance": 5.0,
  "height": 3.0
}
```

**Response:**
```json
{
  "success": true,
  "status": "tracking_started",
  "mode": "follow",
  "target_distance": 5.0,
  "timestamp": "2024-01-15T10:30:00Z"
}
```

#### Stop Tracking
```
POST /api/tracking/stop
```

**Response:**
```json
{
  "success": true,
  "status": "tracking_stopped",
  "reason": "user_requested",
  "timestamp": "2024-01-15T10:30:00Z"
}
```

#### Get Tracking Status
```
GET /api/tracking/status
```

**Response:**
```json
{
  "success": true,
  "status": "tracking_active",
  "mode": "follow",
  "target_detected": true,
  "target_distance": 4.8,
  "confidence": 0.95,
  "tracking_duration": 45.2,
  "timestamp": "2024-01-15T10:30:00Z"
}
```

**Status Values:**
- `"idle"` - Not tracking
- `"tracking_active"` - Actively tracking person
- `"target_lost"` - Person lost, searching
- `"error"` - Tracking error occurred

### 4. WiFi Configuration (SoftAP Mode)

#### Configure WiFi Credentials
```
POST /api/wifi
Host: 192.168.4.1  # When connected to Pi's AP
```

**Request Body:**
```json
{
  "ssid": "YourHomeNetwork",
  "password": "YourPassword",
  "security": "WPA2",
  "country": "US"
}
```

**Response:**
```json
{
  "success": true,
  "message": "WiFi credentials received. Restarting network...",
  "ssid": "YourHomeNetwork",
  "estimated_restart_time": 30
}
```

**Note:** The Pi should restart its network stack after receiving credentials, which may cause the HTTP connection to timeout. This is normal behavior.

### 5. Custom Commands
```
POST /api/command
```

**Request Body:**
```json
{
  "command": "arm_motors",
  "params": {
    "force": false,
    "checks": ["battery", "gps", "compass"]
  }
}
```

**Response:**
```json
{
  "success": true,
  "message": "Motors armed successfully",
  "command": "arm_motors",
  "result": {
    "battery_level": 85,
    "gps_fix": "3D",
    "compass_calibrated": true
  },
  "timestamp": "2024-01-15T10:30:00Z"
}
```

### 6. Camera Stream
```
GET /stream.mjpeg
Port: 8080 (default camera stream port)
```

**Response:** MJPEG multipart stream
```
Content-Type: multipart/x-mixed-replace; boundary=frame

--frame
Content-Type: image/jpeg
Content-Length: [size]

[JPEG data]
--frame
Content-Type: image/jpeg
Content-Length: [size]

[JPEG data]
...
```

## Error Responses

All endpoints should return appropriate HTTP status codes and error details:

```json
{
  "success": false,
  "error": "invalid_request",
  "message": "Missing required parameter: ssid",
  "code": 400,
  "timestamp": "2024-01-15T10:30:00Z"
}
```

**Common Error Codes:**
- `400` - Bad Request (invalid parameters)
- `401` - Unauthorized (authentication required)
- `404` - Not Found (endpoint doesn't exist)
- `405` - Method Not Allowed (wrong HTTP method)
- `500` - Internal Server Error (server error)
- `503` - Service Unavailable (system not ready)

## Implementation Notes

### MAVLink Integration
The Pi should bridge MAVLink telemetry to the JSON format expected by the API:

```python
# Example MAVLink to JSON conversion
def mavlink_to_json(mavlink_msg):
    if mavlink_msg.get_type() == 'GLOBAL_POSITION_INT':
        return {
            "lat": mavlink_msg.lat / 1e7,
            "lon": mavlink_msg.lon / 1e7,
            "alt": mavlink_msg.alt / 1000.0,
            "fix": "3D" if mavlink_msg.hdop < 2.0 else "2D",
            "sats": mavlink_msg.satellites_visible
        }
```

### Person Tracking
The tracking system should:
1. Use computer vision to detect and track persons
2. Calculate appropriate drone positioning (distance, height)
3. Send movement commands to the flight controller
4. Handle target loss gracefully
5. Provide real-time status updates

### Security Considerations
- Implement rate limiting on API endpoints
- Validate all input parameters
- Use HTTPS when possible (though HTTP is acceptable for local network)
- Consider implementing API key authentication for production use

### Performance
- Cache telemetry data to avoid overwhelming the flight controller
- Implement WebSocket endpoints for real-time data if needed
- Use efficient image processing for person tracking
- Monitor system resources (CPU, memory, temperature)

This API specification ensures seamless integration between the Android GCS app and the Raspberry Pi companion computer, providing a complete drone control and monitoring solution.