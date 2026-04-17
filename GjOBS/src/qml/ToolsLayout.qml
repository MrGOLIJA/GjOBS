import QtQuick 2.3
import QtQuick.Layouts

Rectangle {
    id: toolsLayout
    color: "green"
    Layout.fillWidth: true
    Layout.preferredHeight: 250 * scale
    Rectangle {
        anchors {
            top: parent.top
            horizontalCenter: parent.horizontalCenter
        }

        color: "purple"
        width: 300 * scale
        height: parent.height
    }
}
