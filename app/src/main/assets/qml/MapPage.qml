import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtLocation 5.15
import QtPositioning 5.15

Page {
    id: mapPage
    title: "Map"

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // Status bar
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 40
            color: "#f0f0f0"
            border.color: "#d0d0d0"
            border.width: 1

            RowLayout {
                anchors.fill: parent
                anchors.margins: 8
                spacing: 16

                Text {
                    text: "Drone: " + (telemetryClient.hasValidPosition ? 
                          telemetryClient.coordinate.latitude.toFixed(6) + ", " + 
                          telemetryClient.coordinate.longitude.toFixed(6) : "No GPS")
                    font.pointSize: 10
                }

                Text {
                    text: "Alt: " + telemetryClient.altitude.toFixed(1) + "m"
                    font.pointSize: 10
                }

                Text {
                    text: "Sats: " + telemetryClient.satellites
                    font.pointSize: 10
                }

                Text {
                    text: "Fix: " + telemetryClient.fixType
                    font.pointSize: 10
                }

                Item { Layout.fillWidth: true }

                Text {
                    text: "Updated: " + telemetryClient.lastUpdateTime
                    font.pointSize: 8
                    color: "#666"
                }
            }
        }

        // Map
        Map {
            id: map
            Layout.fillWidth: true
            Layout.fillHeight: true
            
            // Plugin selection with fallback
            plugin: Plugin {
                name: settingsManager.googleMapsApiKey.length > 0 ? "googlemaps" : "osm"
                
                PluginParameter {
                    name: "googlemaps.api_key"
                    value: settingsManager.googleMapsApiKey
                }
            }

            center: telemetryClient.hasValidPosition ? 
                    telemetryClient.coordinate : 
                    QtPositioning.coordinate(37.7749, -122.4194) // Default to San Francisco

            zoomLevel: 15

            // Drone marker
            MapQuickItem {
                id: droneMarker
                visible: telemetryClient.hasValidPosition
                coordinate: telemetryClient.coordinate
                anchorPoint.x: droneIcon.width / 2
                anchorPoint.y: droneIcon.height / 2

                sourceItem: Rectangle {
                    id: droneIcon
                    width: 20
                    height: 20
                    radius: 10
                    color: telemetryClient.fixType === "3D" ? "#00ff00" : "#ffaa00"
                    border.color: "#333"
                    border.width: 2

                    Text {
                        anchors.centerIn: parent
                        text: "🚁"
                        font.pointSize: 8
                    }
                }
            }

            // User location marker (if available)
            MapQuickItem {
                id: userMarker
                visible: positionSource.position.coordinate.isValid
                coordinate: positionSource.position.coordinate
                anchorPoint.x: userIcon.width / 2
                anchorPoint.y: userIcon.height / 2

                sourceItem: Rectangle {
                    id: userIcon
                    width: 16
                    height: 16
                    radius: 8
                    color: "#0066ff"
                    border.color: "#fff"
                    border.width: 2
                }
            }

            // Mouse area for map interaction
            MouseArea {
                anchors.fill: parent
                onDoubleClicked: {
                    if (telemetryClient.hasValidPosition) {
                        map.center = telemetryClient.coordinate
                    }
                }
            }
        }

        // Map controls
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 50
            color: "#f8f8f8"
            border.color: "#d0d0d0"
            border.width: 1

            RowLayout {
                anchors.fill: parent
                anchors.margins: 8
                spacing: 8

                Button {
                    text: "Center on Drone"
                    enabled: telemetryClient.hasValidPosition
                    onClicked: {
                        map.center = telemetryClient.coordinate
                        map.zoomLevel = 16
                    }
                }

                Button {
                    text: "Center on User"
                    enabled: positionSource.position.coordinate.isValid
                    onClicked: {
                        map.center = positionSource.position.coordinate
                        map.zoomLevel = 16
                    }
                }

                Item { Layout.fillWidth: true }

                Text {
                    text: "Provider: " + (settingsManager.googleMapsApiKey.length > 0 ? "Google Maps" : "OpenStreetMap")
                    font.pointSize: 9
                    color: "#666"
                }
            }
        }
    }

    // Position source for user location
    PositionSource {
        id: positionSource
        active: true
        updateInterval: 5000
    }
}