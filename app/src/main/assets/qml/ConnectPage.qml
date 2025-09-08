import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Page {
    id: connectPage
    title: "Connect"

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 16

        GroupBox {
            title: "Bluetooth Connection"
            Layout.fillWidth: true

            ColumnLayout {
                anchors.fill: parent
                spacing: 8

                Button {
                    text: "Scan for Devices"
                    Layout.fillWidth: true
                    onClicked: bluetoothManager.startScan()
                }

                ListView {
                    id: deviceList
                    Layout.fillWidth: true
                    Layout.preferredHeight: 120
                    model: bluetoothManager.deviceModel
                    
                    delegate: ItemDelegate {
                        width: deviceList.width
                        text: model.name + " (" + model.address + ")"
                        onClicked: bluetoothManager.connectToDevice(model.index)
                    }
                }

                Text {
                    text: "Status: " + bluetoothManager.connectionStatus
                    Layout.fillWidth: true
                }
            }
        }

        GroupBox {
            title: "WiFi Provisioning"
            Layout.fillWidth: true

            ColumnLayout {
                anchors.fill: parent
                spacing: 8

                TextField {
                    id: ssidField
                    placeholderText: "WiFi SSID"
                    Layout.fillWidth: true
                }

                TextField {
                    id: passwordField
                    placeholderText: "WiFi Password"
                    echoMode: TextInput.Password
                    Layout.fillWidth: true
                }

                Button {
                    text: "Send WiFi Credentials"
                    Layout.fillWidth: true
                    enabled: bluetoothManager.isConnected && ssidField.text.length > 0
                    onClicked: bluetoothManager.sendWifiCredentials(ssidField.text, passwordField.text)
                }

                Button {
                    text: "SoftAP Setup Wizard"
                    Layout.fillWidth: true
                    onClicked: softAPDialog.open()
                }
            }
        }

        GroupBox {
            title: "Person Tracking"
            Layout.fillWidth: true

            RowLayout {
                anchors.fill: parent
                spacing: 8

                Button {
                    text: "Start Tracking"
                    Layout.fillWidth: true
                    enabled: bluetoothManager.isConnected
                    onClicked: bluetoothManager.startTracking()
                }

                Button {
                    text: "Stop Tracking"
                    Layout.fillWidth: true
                    enabled: bluetoothManager.isConnected
                    onClicked: bluetoothManager.stopTracking()
                }
            }
        }

        Item {
            Layout.fillHeight: true
        }
    }

    SoftAPWizard {
        id: softAPDialog
    }
}