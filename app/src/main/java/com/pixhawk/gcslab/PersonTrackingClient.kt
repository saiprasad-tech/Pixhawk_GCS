package com.pixhawk.gcslab

import kotlinx.coroutines.*
import okhttp3.*
import okhttp3.MediaType.Companion.toMediaType
import okhttp3.RequestBody.Companion.toRequestBody
import org.json.JSONObject
import java.io.IOException

/**
 * HTTP client for person tracking and Pi control APIs
 * Complements the BLE functionality with HTTP-based commands
 */
class PersonTrackingClient {
    private val client = OkHttpClient.Builder()
        .connectTimeout(5, java.util.concurrent.TimeUnit.SECONDS)
        .readTimeout(10, java.util.concurrent.TimeUnit.SECONDS)
        .writeTimeout(10, java.util.concurrent.TimeUnit.SECONDS)
        .build()

    private val mediaTypeJson = "application/json; charset=utf-8".toMediaType()
    private val scope = CoroutineScope(Dispatchers.IO + SupervisorJob())

    var piHost: String = "pi.local"
    var apiPort: Int = 8000

    fun startTracking(callback: (Boolean, String) -> Unit) {
        scope.launch {
            try {
                val url = "http://$piHost:$apiPort/api/tracking/start"
                val request = Request.Builder()
                    .url(url)
                    .post("{}".toRequestBody(mediaTypeJson))
                    .addHeader("User-Agent", "PixhawkGCS/1.0")
                    .build()

                client.newCall(request).execute().use { response ->
                    val responseBody = response.body?.string()
                    
                    if (response.isSuccessful && responseBody != null) {
                        try {
                            val json = JSONObject(responseBody)
                            val success = json.optBoolean("success", false)
                            val status = json.optString("status", "Unknown")
                            
                            withContext(Dispatchers.Main) {
                                callback(success, if (success) "Tracking started: $status" else "Failed: $status")
                            }
                        } catch (e: Exception) {
                            withContext(Dispatchers.Main) {
                                callback(true, "Tracking started (response parse error)")
                            }
                        }
                    } else {
                        withContext(Dispatchers.Main) {
                            callback(false, "HTTP ${response.code}: ${response.message}")
                        }
                    }
                }
            } catch (e: IOException) {
                withContext(Dispatchers.Main) {
                    callback(false, "Network error: ${e.message}")
                }
            } catch (e: Exception) {
                withContext(Dispatchers.Main) {
                    callback(false, "Error: ${e.message}")
                }
            }
        }
    }

    fun stopTracking(callback: (Boolean, String) -> Unit) {
        scope.launch {
            try {
                val url = "http://$piHost:$apiPort/api/tracking/stop"
                val request = Request.Builder()
                    .url(url)
                    .post("{}".toRequestBody(mediaTypeJson))
                    .addHeader("User-Agent", "PixhawkGCS/1.0")
                    .build()

                client.newCall(request).execute().use { response ->
                    val responseBody = response.body?.string()
                    
                    if (response.isSuccessful && responseBody != null) {
                        try {
                            val json = JSONObject(responseBody)
                            val success = json.optBoolean("success", false)
                            val status = json.optString("status", "Unknown")
                            
                            withContext(Dispatchers.Main) {
                                callback(success, if (success) "Tracking stopped: $status" else "Failed: $status")
                            }
                        } catch (e: Exception) {
                            withContext(Dispatchers.Main) {
                                callback(true, "Tracking stopped (response parse error)")
                            }
                        }
                    } else {
                        withContext(Dispatchers.Main) {
                            callback(false, "HTTP ${response.code}: ${response.message}")
                        }
                    }
                }
            } catch (e: IOException) {
                withContext(Dispatchers.Main) {
                    callback(false, "Network error: ${e.message}")
                }
            } catch (e: Exception) {
                withContext(Dispatchers.Main) {
                    callback(false, "Error: ${e.message}")
                }
            }
        }
    }

    fun getTrackingStatus(callback: (Boolean, String, JSONObject?) -> Unit) {
        scope.launch {
            try {
                val url = "http://$piHost:$apiPort/api/tracking/status"
                val request = Request.Builder()
                    .url(url)
                    .get()
                    .addHeader("User-Agent", "PixhawkGCS/1.0")
                    .build()

                client.newCall(request).execute().use { response ->
                    val responseBody = response.body?.string()
                    
                    if (response.isSuccessful && responseBody != null) {
                        try {
                            val json = JSONObject(responseBody)
                            val success = json.optBoolean("success", false)
                            val status = json.optString("status", "Unknown")
                            
                            withContext(Dispatchers.Main) {
                                callback(success, status, if (success) json else null)
                            }
                        } catch (e: Exception) {
                            withContext(Dispatchers.Main) {
                                callback(false, "Response parse error: ${e.message}", null)
                            }
                        }
                    } else {
                        withContext(Dispatchers.Main) {
                            callback(false, "HTTP ${response.code}: ${response.message}", null)
                        }
                    }
                }
            } catch (e: IOException) {
                withContext(Dispatchers.Main) {
                    callback(false, "Network error: ${e.message}", null)
                }
            } catch (e: Exception) {
                withContext(Dispatchers.Main) {
                    callback(false, "Error: ${e.message}", null)
                }
            }
        }
    }

    fun testConnection(callback: (Boolean, String) -> Unit) {
        scope.launch {
            try {
                val url = "http://$piHost:$apiPort/api/health"
                val request = Request.Builder()
                    .url(url)
                    .get()
                    .addHeader("User-Agent", "PixhawkGCS/1.0")
                    .build()

                client.newCall(request).execute().use { response ->
                    if (response.isSuccessful) {
                        withContext(Dispatchers.Main) {
                            callback(true, "Connection successful")
                        }
                    } else {
                        withContext(Dispatchers.Main) {
                            callback(false, "HTTP ${response.code}: ${response.message}")
                        }
                    }
                }
            } catch (e: IOException) {
                withContext(Dispatchers.Main) {
                    callback(false, "Connection failed: ${e.message}")
                }
            } catch (e: Exception) {
                withContext(Dispatchers.Main) {
                    callback(false, "Error: ${e.message}")
                }
            }
        }
    }

    fun sendCustomCommand(command: String, params: JSONObject = JSONObject(), callback: (Boolean, String, JSONObject?) -> Unit) {
        scope.launch {
            try {
                val url = "http://$piHost:$apiPort/api/command"
                val payload = JSONObject().apply {
                    put("command", command)
                    put("params", params)
                }

                val request = Request.Builder()
                    .url(url)
                    .post(payload.toString().toRequestBody(mediaTypeJson))
                    .addHeader("User-Agent", "PixhawkGCS/1.0")
                    .build()

                client.newCall(request).execute().use { response ->
                    val responseBody = response.body?.string()
                    
                    if (response.isSuccessful && responseBody != null) {
                        try {
                            val json = JSONObject(responseBody)
                            val success = json.optBoolean("success", false)
                            val message = json.optString("message", "Command executed")
                            
                            withContext(Dispatchers.Main) {
                                callback(success, message, if (success) json else null)
                            }
                        } catch (e: Exception) {
                            withContext(Dispatchers.Main) {
                                callback(false, "Response parse error: ${e.message}", null)
                            }
                        }
                    } else {
                        withContext(Dispatchers.Main) {
                            callback(false, "HTTP ${response.code}: ${response.message}", null)
                        }
                    }
                }
            } catch (e: IOException) {
                withContext(Dispatchers.Main) {
                    callback(false, "Network error: ${e.message}", null)
                }
            } catch (e: Exception) {
                withContext(Dispatchers.Main) {
                    callback(false, "Error: ${e.message}", null)
                }
            }
        }
    }

    fun cleanup() {
        scope.cancel()
    }
}