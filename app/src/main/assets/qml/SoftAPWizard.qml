import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Dialog {
    id: softAPWizard
    title: "WiFi Setup Wizard"
    width: 400
    height: 500
    modal: true
    standardButtons: Dialog.Cancel

    property int currentStep: 0
    readonly property var stepTitles: [
        "Connect to Pi's Access Point",
        "Send WiFi Credentials",
        "Complete Setup"
    ]

    header: TabBar {
        id: stepIndicator
        currentIndex: currentStep
        
        Repeater {
            model: stepTitles
            TabButton {
                text: (index + 1) + ". " + modelData
                enabled: false
            }
        }
    }

    StackLayout {
        anchors.fill: parent
        currentIndex: currentStep

        // Step 1: Connect to Pi's AP
        Page {
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 16
                spacing: 16

                Text {
                    text: "Step 1: Connect to Raspberry Pi Access Point"
                    font.pointSize: 14
                    font.bold: true
                    Layout.fillWidth: true
                }

                Text {
                    text: "1. Go to your device's WiFi settings\n" +
                          "2. Look for a network named 'PixhawkPi-XXXX'\n" +
                          "3. Connect to this network\n" +
                          "4. Return to this app and click 'Next'"
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 100
                    color: "#f0f8ff"
                    border.color: "#4CAF50"
                    border.width: 1
                    radius: 4

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 8

                        Text {
                            text: "💡 Tip"
                            font.bold: true
                            color: "#4CAF50"
                        }
                        
                        Text {
                            text: "If you don't see the Pi's network, make sure the Pi is powered on and in setup mode."
                            Layout.fillWidth: true
                            wrapMode: Text.WordWrap
                            font.pointSize: 9
                        }
                    }
                }

                Item { Layout.fillHeight: true }

                RowLayout {
                    Layout.fillWidth: true

                    Item { Layout.fillWidth: true }

                    Button {
                        text: "Next"
                        onClicked: currentStep = 1
                    }
                }
            }
        }

        // Step 2: Send credentials
        Page {
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 16
                spacing: 16

                Text {
                    text: "Step 2: Configure WiFi Credentials"
                    font.pointSize: 14
                    font.bold: true
                    Layout.fillWidth: true
                }

                Text {
                    text: "Enter your home WiFi credentials to configure the Raspberry Pi:"
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                }

                GroupBox {
                    title: "WiFi Credentials"
                    Layout.fillWidth: true

                    ColumnLayout {
                        anchors.fill: parent
                        spacing: 8

                        TextField {
                            id: wizardSsidField
                            placeholderText: "WiFi Network Name (SSID)"
                            Layout.fillWidth: true
                        }

                        TextField {
                            id: wizardPasswordField
                            placeholderText: "WiFi Password"
                            echoMode: TextInput.Password
                            Layout.fillWidth: true
                        }

                        CheckBox {
                            id: showPasswordCheck
                            text: "Show password"
                            onCheckedChanged: {
                                wizardPasswordField.echoMode = checked ? 
                                    TextInput.Normal : TextInput.Password
                            }
                        }
                    }
                }

                Text {
                    id: statusText
                    text: wifiManager.setupStatus
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                    color: wifiManager.setupStatus.includes("Error") ? "#f44336" : "#4CAF50"
                }

                Item { Layout.fillHeight: true }

                RowLayout {
                    Layout.fillWidth: true

                    Button {
                        text: "Back"
                        onClicked: currentStep = 0
                    }

                    Item { Layout.fillWidth: true }

                    Button {
                        text: "Send Credentials"
                        enabled: wizardSsidField.text.length > 0 && 
                                wizardPasswordField.text.length > 0 &&
                                !wifiManager.isSending
                        onClicked: {
                            wifiManager.sendCredentials(wizardSsidField.text, wizardPasswordField.text)
                        }
                    }

                    Button {
                        text: "Next"
                        enabled: wifiManager.setupComplete
                        onClicked: currentStep = 2
                    }
                }
            }
        }

        // Step 3: Complete
        Page {
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 16
                spacing: 16

                Text {
                    text: "Step 3: Setup Complete"
                    font.pointSize: 14
                    font.bold: true
                    Layout.fillWidth: true
                }

                Text {
                    text: wifiManager.setupComplete ? 
                          "✅ WiFi credentials sent successfully!\n\n" +
                          "The Raspberry Pi will now:\n" +
                          "1. Disconnect from setup mode\n" +
                          "2. Connect to your home WiFi\n" +
                          "3. Become available at: " + settingsManager.piHost :
                          "⏳ Waiting for setup to complete..."
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 120
                    color: "#fff3e0"
                    border.color: "#ff9800"
                    border.width: 1
                    radius: 4

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 8

                        Text {
                            text: "📋 Next Steps"
                            font.bold: true
                            color: "#ff9800"
                        }
                        
                        Text {
                            text: "1. Reconnect your device to your home WiFi\n" +
                                  "2. Go to the Settings page to configure the Pi host\n" +
                                  "3. Test telemetry and camera connections"
                            Layout.fillWidth: true
                            wrapMode: Text.WordWrap
                            font.pointSize: 9
                        }
                    }
                }

                Item { Layout.fillHeight: true }

                RowLayout {
                    Layout.fillWidth: true

                    Button {
                        text: "Back"
                        onClicked: currentStep = 1
                    }

                    Item { Layout.fillWidth: true }

                    Button {
                        text: "Finish"
                        onClicked: {
                            currentStep = 0
                            softAPWizard.close()
                        }
                    }
                }
            }
        }
    }
}