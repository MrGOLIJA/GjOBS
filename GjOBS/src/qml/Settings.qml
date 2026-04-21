import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Window {
    color: theme.background
    title: "Настройки"
    property int textWidth: 140
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        Rectangle {
            color: theme.subbackground
            border.width: 1
            border.color: "black"
            Layout.preferredWidth: 300
            Layout.preferredHeight: column.height + 20

            Column {
                id: column
                width: parent.width - 40
                anchors {
                    top: parent.top
                    left: parent.left
                    topMargin: 10
                    leftMargin: 10
                }
                anchors.margins: 10
                spacing: 10

                Row {
                    spacing: 10
                    width: parent.width

                    Text {
                        text: "Выходной формат"
                        width: textWidth
                        horizontalAlignment: Text.AlignRight
                        verticalAlignment: Text.AlignVCenter
                    }

                    ComboBox {
                        id: outputFormat
                        width: parent.width - 130
                        model: settings.getOutputFormatModel()
                        textRole: "text"
                        valueRole: "value"
                        currentIndex: settings.getFormat_index()
                    }
                }

                Row {
                    spacing: 10
                    width: parent.width

                    Text {
                        text: "Аудио кодек"
                        width: textWidth
                        horizontalAlignment: Text.AlignRight
                        verticalAlignment: Text.AlignVCenter
                    }

                    ComboBox {
                        id: audioCodec
                        width: parent.width - 130
                        model: settings.getAudioCodecModel()
                        textRole: "text"
                        valueRole: "value"
                        currentIndex: settings.getAudioCodec_index()
                    }
                }

                Row {
                    spacing: 10
                    width: parent.width

                    Text {
                        text: "Видео кодек"
                        width: textWidth
                        horizontalAlignment: Text.AlignRight
                        verticalAlignment: Text.AlignVCenter
                    }

                    ComboBox {
                        id: videoCodec
                        width: parent.width - 130
                        model: settings.getVideoCodecModel()
                        textRole: "text"
                        valueRole: "value"
                        currentIndex: settings.getVideoCodec_index()
                    }
                }

                Row {
                    spacing: 10
                    width: parent.width

                    Text {
                        text: "Рендер видео"
                        width: textWidth
                        horizontalAlignment: Text.AlignRight
                        verticalAlignment: Text.AlignVCenter
                    }

                    ComboBox {
                        id: render
                        width: parent.width - 130
                        model: settings.getRendModel()
                        textRole: "text"
                        valueRole: "value"
                        currentIndex: settings.getRend_index()
                    }
                }
            }
        }
        GjButton {
            id: buttonSave
            Layout.fillWidth: true
            Layout.preferredHeight: 30

            text.text: "Сохранить"

            mouseArea.onClicked: {
                var oldFormat = settings.getFormat()
                var oldAudioCodec = settings.getAudioCodec()
                var oldVideoCodec = settings.getVideoCodec()
                var oldRend = settings.getRend()

                if (settings.setFormat(outputFormat.currentValue)
                        && settings.setAudioCodec(audioCodec.currentValue)
                        && settings.setVideoCodec(videoCodec.currentValue)
                        && settings.setRend(render.currentValue)) {
                    savePopup.textItem = "Успешно сохранено"
                    savePopup.colorItem = "green"
                    savePopup.open()
                } else {
                    settings.setFormat(oldFormat)
                    settings.setAudioCodec(oldAudioCodec)
                    settings.setVideoCodec(oldVideoCodec)
                    settings.setRend(oldRend)
                    savePopup.textItem = "Выбраны несовместимые кодеки"
                    savePopup.colorItem = "red"
                    savePopup.open()
                }
            }
        }

        Popup {
            id: savePopup

            property alias textItem: textPopup.text
            property alias colorItem: rect.color

            x: buttonSave.width / 2 - rect.width / 2
            y: buttonSave.y - buttonSave.height - 20 - rect.height
            background: Rectangle {
                color: "transparent"
            }

            Rectangle {
                id: rect
                width: textPopup.implicitWidth + 20
                height: textPopup.implicitHeight + 20
                radius: 5
                color: theme.subbackground
                Text {
                    anchors.fill: parent
                    id: textPopup
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }
        }
    }
}
