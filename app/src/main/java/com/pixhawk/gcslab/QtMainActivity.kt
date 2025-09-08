package com.pixhawk.gcslab

import android.Manifest
import android.content.pm.PackageManager
import android.os.Bundle
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import androidx.core.app.ActivityCompat
import androidx.core.content.ContextCompat

class QtMainActivity : AppCompatActivity() {
    private val PERMISSION_REQUEST_CODE = 1001
    
    private val REQUIRED_PERMISSIONS = arrayOf(
        Manifest.permission.ACCESS_FINE_LOCATION,
        Manifest.permission.ACCESS_COARSE_LOCATION,
        Manifest.permission.BLUETOOTH,
        Manifest.permission.BLUETOOTH_ADMIN,
        Manifest.permission.ACCESS_WIFI_STATE,
        Manifest.permission.CHANGE_WIFI_STATE
    )
    
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        
        // Check and request permissions
        if (!hasAllPermissions()) {
            requestPermissions()
        } else {
            initializeQt()
        }
    }
    
    private fun hasAllPermissions(): Boolean {
        return REQUIRED_PERMISSIONS.all { permission ->
            ContextCompat.checkSelfPermission(this, permission) == PackageManager.PERMISSION_GRANTED
        }
    }
    
    private fun requestPermissions() {
        ActivityCompat.requestPermissions(this, REQUIRED_PERMISSIONS, PERMISSION_REQUEST_CODE)
    }
    
    override fun onRequestPermissionsResult(
        requestCode: Int,
        permissions: Array<out String>,
        grantResults: IntArray
    ) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults)
        
        if (requestCode == PERMISSION_REQUEST_CODE) {
            if (grantResults.all { it == PackageManager.PERMISSION_GRANTED }) {
                initializeQt()
            } else {
                Toast.makeText(this, "Permissions required for full functionality", Toast.LENGTH_LONG).show()
                initializeQt() // Continue anyway with limited functionality
            }
        }
    }
    
    private fun initializeQt() {
        // For now, show a simple message that Qt integration is ready
        // In a full implementation, this would initialize Qt and load QML
        setContentView(R.layout.qt_placeholder)
        
        // Setup back button
        findViewById<android.widget.Button>(R.id.btnBackToNative).setOnClickListener {
            finish()
        }
        
        Toast.makeText(this, "Qt/QML interface ready (placeholder)", Toast.LENGTH_LONG).show()
    }
}