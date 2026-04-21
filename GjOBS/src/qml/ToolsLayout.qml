import QtQuick
import QtQuick.Layouts
import QtQuick.Window
import QtQuick.Controls

Rectangle {
    id: toolsLayout
    color: theme.toolsLayout
    Rectangle {
        anchors {
            top: parent.top
            horizontalCenter: parent.horizontalCenter
        }
        color: theme.buttonsLayout
        width: 300 * scale
        height: parent.height
        ColumnLayout {
            spacing: 10
            anchors.fill: parent
            GjButton {
                id: buttonStart

                property bool started: false

                Layout.fillWidth: true
                Layout.preferredHeight: 30
                text.text: "Начать запись"
                mouseArea.onClicked: {
                    if (!buttonStart.started) {
                        recorder.start()
                        buttonStart.started = true
                        text.text = "Остановить запись"
                    } else {
                        recorder.stop()
                        buttonStart.started = false
                        text.text = "Начать запись"
                    }
                }
            }
            GjButton {
                id: buttonSettings
                Layout.fillWidth: true
                Layout.preferredHeight: 30
                text.text: "Настройки"
                mouseArea.onClicked: {
                    settingsWindow.visible = true
                }
                Settings {
                    id: settingsWindow
                    width: parent.width
                    height: parent.height
                }
            }
            GjButton {
                id: buttonExit
                Layout.fillWidth: true
                Layout.preferredHeight: 30
                text.text: "Настройки"
                mouseArea.onClicked: {
                    Qt.quit()
                }
            }
        }
    }
}
