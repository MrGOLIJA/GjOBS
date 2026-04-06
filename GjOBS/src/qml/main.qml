import QtQuick
import QtQuick.Window

Window {
    visible: true
    width: 200
    height: 100
    title: "GjOBS"

    Rectangle {
        anchors.fill: parent
        color: "red"

        Text {
            anchors.centerIn: parent
            text: "Hello, World!"
        }
    }
}