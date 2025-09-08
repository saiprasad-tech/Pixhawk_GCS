import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Page {
    id: settingsPage
    title: "Settings"

    ScrollView {
        anchors.fill: parent
        anchors.margins: 16
        contentWidth: availableWidth

        ColumnLayout {
            width: parent.width
            spacing: 16

            GroupBox {
                title: "Map Settings"
                Layout.fillWidth: true

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 8

                    Text {
                        text: "Google Maps API Key (optional)"
                        font.pointSize: 10
                        color: "#666"
                    }

                    TextField {
                        id: googleMapsKeyField
                        Layout.fillWidth: true
                        placeholderText: "Leave empty to use OpenStreetMap"
                        text: settingsManager.googleMapsApiKey
                        echoMode: TextInput.Password
                        onEditingFinished: settingsManager.setGoogleMapsApiKey(text)
                    }

                    Text {
                        text: "Current provider: " + (googleMapsKeyField.text.length > 0 ? "Google Maps" : "OpenStreetMap")
                        font.pointSize: 9
                        color: "#666"
                    }
                }
            }

            GroupBox {
                title: "Raspberry Pi Connection"
                Layout.fillWidth: true

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 8

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        Text {
                            text: "Host/IP:"
                            Layout.preferredWidth: 60
                        }

                        TextField {
                            id: piHostField
                            Layout.fillWidth: true
                            text: settingsManager.piHost
                            placeholderText: "pi.local or 192.168.1.100"
                            onEditingFinished: settingsManager.setPiHost(text)
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        Text {
                            text: "Telemetry Port:"
                            Layout.preferredWidth: 100
                        }

                        TextField {
                            id: telemetryPortField
                            Layout.preferredWidth: 80
                            text: settingsManager.telemetryPort.toString()
                            validator: IntValidator { bottom: 1; top: 65535 }
                            onEditingFinished: settingsManager.setTelemetryPort(parseInt(text))
                        }

                        Item { Layout.fillWidth: true }

                        Text {
                            text: "API Port:"
                            Layout.preferredWidth: 60
                        }

                        TextField {
                            id: apiPortField
                            Layout.preferredWidth: 80
                            text: settingsManager.apiPort.toString()
                            validator: IntValidator { bottom: 1; top: 65535 }
                            onEditingFinished: settingsManager.setApiPort(parseInt(text))
                        }
                    }
                }
            }

            GroupBox {
                title: "Camera Settings"
                Layout.fillWidth: true

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 8

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        Text {
                            text: "Stream URL:"
                            Layout.preferredWidth: 80
                        }

                        TextField {
                            id: cameraUrlField
                            Layout.fillWidth: true
                            text: settingsManager.cameraUrl
                            placeholderText: "http://pi.local:8080/stream.mjpeg"
                            onEditingFinished: settingsManager.setCameraUrl(text)
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        Text {
                            text: "Refresh Rate:"
                            Layout.preferredWidth: 100
                        }

                        SpinBox {
                            id: refreshRateSpinBox
                            from: 5
                            to: 60
                            value: settingsManager.cameraRefreshRate
                            suffix: " sec"
                            onValueChanged: settingsManager.setCameraRefreshRate(value)
                        }

                        Item { Layout.fillWidth: true }
                    }
                }
            }

            GroupBox {
                title: "Telemetry Settings"
                Layout.fillWidth: true

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 8

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        Text {
                            text: "Mode:"
                            Layout.preferredWidth: 60
                        }

                        ComboBox {
                            id: telemetryModeCombo
                            Layout.preferredWidth: 150
                            model: ["UDP Listener", "HTTP Poll"]
                            currentIndex: settingsManager.telemetryMode === "udp" ? 0 : 1
                            onCurrentIndexChanged: {
                                settingsManager.setTelemetryMode(currentIndex === 0 ? "udp" : "http")
                            }
                        }

                        Item { Layout.fillWidth: true }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        Text {
                            text: "Update Rate:"
                            Layout.preferredWidth: 100
                        }

                        SpinBox {
                            id: updateRateSpinBox
                            from: 1
                            to: 10
                            value: settingsManager.telemetryUpdateRate
                            suffix: " Hz"
                            onValueChanged: settingsManager.setTelemetryUpdateRate(value)
                        }

                        Item { Layout.fillWidth: true }
                    }
                }
            }

            GroupBox {
                title: "Bluetooth Settings"
                Layout.fillWidth: true

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 8

                    Text {
                        text: "Service UUID"
                        font.pointSize: 10
                        color: "#666"
                    }

                    TextField {
                        id: serviceUuidField
                        Layout.fillWidth: true
                        text: settingsManager.bleServiceUuid
                        placeholderText: "0000abcd-0000-1000-8000-00805f9b34fb"
                        font.family: "monospace"
                        onEditingFinished: settingsManager.setBleServiceUuid(text)
                    }

                    Text {
                        text: "Characteristic UUIDs"
                        font.pointSize: 10
                        color: "#666"
                    }

                    GridLayout {
                        Layout.fillWidth: true
                        columns: 2
                        columnSpacing: 8
                        rowSpacing: 4

                        Text { text: "SSID:" }
                        TextField {
                            Layout.fillWidth: true
                            text: settingsManager.bleSsidUuid
                            font.family: "monospace"
                            font.pointSize: 9
                            onEditingFinished: settingsManager.setBleSsidUuid(text)
                        }

                        Text { text: "Password:" }
                        TextField {
                            Layout.fillWidth: true
                            text: settingsManager.blePasswordUuid
                            font.family: "monospace"
                            font.pointSize: 9
                            onEditingFinished: settingsManager.setBlePasswordUuid(text)
                        }

                        Text { text: "Command:" }
                        TextField {
                            Layout.fillWidth: true
                            text: settingsManager.bleCommandUuid
                            font.family: "monospace"
                            font.pointSize: 9
                            onEditingFinished: settingsManager.setBleCommandUuid(text)
                        }
                    }
                }
            }

            // Action buttons
            RowLayout {
                Layout.fillWidth: true
                spacing: 8

                Button {
                    text: "Reset to Defaults"
                    onClicked: settingsManager.resetToDefaults()
                }

                Button {
                    text: "Test Connection"
                    onClicked: settingsManager.testConnection()
                }

                Item { Layout.fillWidth: true }

                Button {
                    text: "Save Settings"
                    onClicked: settingsManager.saveSettings()
                }
            }

            Item {
                Layout.fillHeight: true
            }
        }
    }
}