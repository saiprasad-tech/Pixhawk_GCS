package com.pixhawk.gcslab

import android.os.Bundle
import android.os.Handler
import android.os.Looper
import android.widget.Button
import android.widget.TextView
import androidx.appcompat.app.AppCompatActivity
import org.json.JSONObject
import java.text.SimpleDateFormat
import java.util.*

class MainActivity : AppCompatActivity() {
    private lateinit var systemBridge: SystemBridge
    private lateinit var btnStart: Button
    private lateinit var btnStop: Button
    private lateinit var tvRateHz: TextView
    private lateinit var tvAvgAltitude: TextView
    private lateinit var tvAvgBattery: TextView
    private lateinit var tvMessages: TextView
    
    private val updateHandler = Handler(Looper.getMainLooper())
    private var updateRunnable: Runnable? = null
    private var isRunning = false
    
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        
        systemBridge = SystemBridge()
        
        // Initialize views
        btnStart = findViewById(R.id.btnStartTelemetry)
        btnStop = findViewById(R.id.btnStopTelemetry)
        tvRateHz = findViewById(R.id.tvRateHz)
        tvAvgAltitude = findViewById(R.id.tvAvgAltitude)
        tvAvgBattery = findViewById(R.id.tvAvgBattery)
        tvMessages = findViewById(R.id.tvMessages)
        
        // Setup button listeners
        btnStart.setOnClickListener { startTelemetry() }
        btnStop.setOnClickListener { stopTelemetry() }
        
        // Initialize the native system
        try {
            val result = systemBridge.initSystems(assets)
            val json = JSONObject(result)
            if (!json.getBoolean("ok")) {
                tvMessages.text = "Init failed: ${json.optString("error", "Unknown error")}"
            }
        } catch (e: Exception) {
            tvMessages.text = "Init exception: ${e.message}"
        }
    }
    
    private fun startTelemetry() {
        try {
            val result = systemBridge.startTelemetry()
            val json = JSONObject(result)
            
            if (json.getBoolean("ok")) {
                isRunning = true
                btnStart.isEnabled = false
                btnStop.isEnabled = true
                startUpdating()
                tvMessages.text = "Telemetry started successfully"
            } else {
                tvMessages.text = "Failed to start: ${json.optString("error", "Unknown error")}"
            }
        } catch (e: Exception) {
            tvMessages.text = "Start exception: ${e.message}"
        }
    }
    
    private fun stopTelemetry() {
        try {
            val result = systemBridge.stopTelemetry()
            val json = JSONObject(result)
            
            isRunning = false
            btnStart.isEnabled = true
            btnStop.isEnabled = false
            stopUpdating()
            
            if (json.getBoolean("ok")) {
                tvMessages.text = "Telemetry stopped successfully"
            } else {
                tvMessages.text = "Stop warning: ${json.optString("error", "Unknown error")}"
            }
        } catch (e: Exception) {
            tvMessages.text = "Stop exception: ${e.message}"
        }
    }
    
    private fun startUpdating() {
        updateRunnable = object : Runnable {
            override fun run() {
                if (isRunning) {
                    updateStats()
                    updateMessages()
                    updateHandler.postDelayed(this, 1000) // Update every second
                }
            }
        }
        updateHandler.post(updateRunnable!!)
    }
    
    private fun stopUpdating() {
        updateRunnable?.let { updateHandler.removeCallbacks(it) }
    }
    
    private fun updateStats() {
        try {
            val result = systemBridge.getTelemetryStats()
            val json = JSONObject(result)
            
            if (json.getBoolean("ok")) {
                val rateHz = json.getDouble("rate_hz")
                val avgAltitude = json.getDouble("avg_altitude")
                val avgBattV = json.getDouble("avg_batt_v")
                
                tvRateHz.text = getString(R.string.rate_hz, rateHz)
                tvAvgAltitude.text = getString(R.string.avg_altitude, avgAltitude)
                tvAvgBattery.text = getString(R.string.avg_battery, avgBattV)
            }
        } catch (e: Exception) {
            // Silently handle stats update errors to avoid UI spam
        }
    }
    
    private fun updateMessages() {
        try {
            val result = systemBridge.getTelemetryBatch(10)
            val json = JSONObject(result)
            
            if (json.getBoolean("ok")) {
                val messages = json.getJSONArray("messages")
                val sb = StringBuilder()
                
                for (i in 0 until messages.length()) {
                    val msg = messages.getJSONObject(i)
                    val type = msg.getString("type")
                    val seq = msg.getInt("seq")
                    val timestamp = msg.getLong("ts_ms")
                    
                    val time = SimpleDateFormat("HH:mm:ss", Locale.getDefault()).format(Date(timestamp))
                    sb.append("[$time] $type #$seq")
                    
                    when (type) {
                        "ATTITUDE" -> {
                            val yaw = msg.getDouble("yaw")
                            val pitch = msg.getDouble("pitch") 
                            val roll = msg.getDouble("roll")
                            sb.append(" Y:%.1f P:%.1f R:%.1f".format(yaw, pitch, roll))
                        }
                        "GPS" -> {
                            val lat = msg.getDouble("lat")
                            val lon = msg.getDouble("lon")
                            val alt = msg.getDouble("alt")
                            sb.append(" %.6f,%.6f @%.1fm".format(lat, lon, alt))
                        }
                        "BATTERY" -> {
                            val voltage = msg.getDouble("voltage")
                            val current = msg.getDouble("current")
                            val remaining = msg.getInt("remaining")
                            sb.append(" %.2fV %.2fA %d%%".format(voltage, current, remaining))
                        }
                        "HEARTBEAT" -> {
                            val mode = msg.getString("mode")
                            val armed = msg.getBoolean("armed")
                            sb.append(" $mode ${if (armed) "ARMED" else "DISARMED"}")
                        }
                    }
                    sb.append("\n")
                }
                
                if (sb.isNotEmpty()) {
                    tvMessages.text = sb.toString()
                } else {
                    tvMessages.text = getString(R.string.no_messages)
                }
            }
        } catch (e: Exception) {
            // Silently handle message update errors
        }
    }
    
    override fun onDestroy() {
        super.onDestroy()
        if (isRunning) {
            stopTelemetry()
        }
    }
}