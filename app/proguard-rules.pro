# Add any ProGuard configurations here
-keep class com.pixhawk.gcslab.SystemBridge { *; }
-keep class com.pixhawk.gcslab.** { native <methods>; }