import QtQuick
import QtQuick.Window
import QtQuick.Layouts

Window {
    visible: true
    width: Screen.width / 2
    height: Screen.height / 2
    minimumWidth: Screen.width / 3
    minimumHeight: chooseScreen.height + toolsLayout.height + 50
    title: "GjOBS"
    id: mainWindow

    property real scale: Screen.devicePixelRatio

    AppTheme {
        id: theme
    }

    Rectangle {
        anchors.fill: parent
        color: theme.background

        ColumnLayout {
            id: mainColumn
            spacing: 5
            anchors.fill: parent
            Rectangle {
                id: screenLayout
                color: theme.background
                Layout.fillWidth: true
                Layout.preferredHeight: (mainWindow.height - chooseScreen.height
                                         - toolsLayout.height) * scale
            }
            Rectangle {
                id: chooseScreen
                color: theme.chooseScreen
                Layout.fillWidth: true
                Layout.preferredHeight: 50 * scale
            }
            ToolsLayout {
                id: toolsLayout
                Layout.fillWidth: true
                Layout.preferredHeight: 250 * scale
            }
        }
    }
}
