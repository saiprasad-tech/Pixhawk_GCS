import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Page {
    id: cameraPage
    title: "Camera"

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // Camera stream
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#000"
            border.color: "#333"
            border.width: 1

            Image {
                id: cameraStream
                anchors.fill: parent
                fillMode: Image.PreserveAspectFit
                cache: false
                asynchronous: true
                
                source: cameraManager.isStreaming ? cameraManager.streamUrl : ""
                
                Text {
                    anchors.centerIn: parent
                    text: cameraManager.isStreaming ? 
                          (cameraStream.status === Image.Loading ? "Loading..." : 
                           cameraStream.status === Image.Error ? "Stream Error" : "") :
                          "Camera stream not active"
                    color: "#fff"
                    font.pointSize: 14
                    visible: !cameraManager.isStreaming || cameraStream.status !== Image.Ready
                }

                BusyIndicator {
                    anchors.centerIn: parent
                    visible: cameraStream.status === Image.Loading
                    running: visible
                }
            }

            // Stream info overlay
            Rectangle {
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.margins: 8
                width: streamInfo.width + 16
                height: streamInfo.height + 8
                color: "#80000000"
                radius: 4
                visible: cameraManager.isStreaming

                Text {
                    id: streamInfo
                    anchors.centerIn: parent
                    text: "📹 " + cameraManager.streamUrl + "\n" +
                          "FPS: " + cameraManager.fps + " | " +
                          "Resolution: " + cameraStream.sourceSize.width + "x" + cameraStream.sourceSize.height
                    color: "#fff"
                    font.pointSize: 9
                }
            }
        }

        // Camera controls
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 80
            color: "#f8f8f8"
            border.color: "#d0d0d0"
            border.width: 1

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 8
                spacing: 4

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    Button {
                        text: cameraManager.isStreaming ? "Stop Stream" : "Start Stream"
                        onClicked: {
                            if (cameraManager.isStreaming) {
                                cameraManager.stopStream()
                            } else {
                                cameraManager.startStream()
                            }
                        }
                    }

                    Button {
                        text: "Refresh"
                        enabled: cameraManager.isStreaming
                        onClicked: cameraManager.refreshStream()
                    }

                    Item { Layout.fillWidth: true }

                    Text {
                        text: "Status: " + cameraManager.status
                        font.pointSize: 9
                        color: "#666"
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    Text {
                        text: "URL:"
                        font.pointSize: 9
                    }

                    TextField {
                        id: urlField
                        Layout.fillWidth: true
                        text: cameraManager.streamUrl
                        placeholderText: "http://pi.local:8080/stream.mjpeg"
                        font.pointSize: 9
                        onEditingFinished: cameraManager.setStreamUrl(text)
                    }

                    Button {
                        text: "Test"
                        onClicked: cameraManager.testConnection(urlField.text)
                    }
                }
            }
        }
    }

    // Auto-refresh timer for MJPEG stream
    Timer {
        id: refreshTimer
        interval: 30000 // 30 seconds
        running: cameraManager.isStreaming
        repeat: true
        onTriggered: {
            // Force cache refresh for MJPEG streams
            cameraStream.source = ""
            cameraStream.source = cameraManager.streamUrl + "?t=" + Date.now()
        }
    }
}