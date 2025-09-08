import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

ApplicationWindow {
    id: mainWindow
    visible: true
    width: 400
    height: 600
    title: "Pixhawk GCS"

    property alias currentIndex: tabBar.currentIndex

    header: TabBar {
        id: tabBar
        TabButton {
            text: "Connect"
        }
        TabButton {
            text: "Map"
        }
        TabButton {
            text: "Camera"
        }
        TabButton {
            text: "Settings"
        }
    }

    StackLayout {
        anchors.fill: parent
        currentIndex: tabBar.currentIndex

        ConnectPage {
            id: connectPage
        }

        MapPage {
            id: mapPage
        }

        CameraPage {
            id: cameraPage
        }

        SettingsPage {
            id: settingsPage
        }
    }
}