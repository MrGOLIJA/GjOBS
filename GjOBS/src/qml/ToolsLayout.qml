import QtQuick
import QtQuick.Layouts
import QtQuick.Window
import QtQuick.Controls

Rectangle {
    id: toolsLayout
    color: "green"
    Rectangle {
        anchors {
            top: parent.top
            horizontalCenter: parent.horizontalCenter
        }
        color: "purple"
        width: 300 * scale
        height: parent.height
        ColumnLayout{
            spacing: 10
            anchors.fill: parent
            Rectangle{
                id: buttonStart

                property bool started: false

                color: "blue"
                radius: 10
                anchors{
                    leftMargin: 10
                    rightMargin: 10
                }

                Layout.fillWidth: true
                Layout.preferredHeight: 30

                Text{
                    anchors.fill: parent
                    id: startText
                    text: "Начать запись"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter

                }

                MouseArea{
                    anchors.fill: parent
                    onClicked:{
                        if (!buttonStart.started){
                            recorder.start()
                            buttonStart.started = true
                            startText.text = "Остановить запись"
                        }
                        else{
                            recorder.stop()
                            buttonStart.started = false;
                            startText.text = "Начать запись"
                        }
                    }
                }
            }
        }
    }
}
