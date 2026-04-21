import QtQuick 2.3

Rectangle {
    id: buttonSettings
    color: theme.button
    radius: 10

    property alias mouseArea: mouseArea
    property alias text: text
    anchors {
        leftMargin: 10
        rightMargin: 10
    }

    Text {
        anchors.fill: parent
        id: text
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
    }
}
