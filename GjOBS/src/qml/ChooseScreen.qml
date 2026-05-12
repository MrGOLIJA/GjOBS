import QtQuick 2.3
import QtQuick.Controls

Rectangle {
    Row {
        width: 200
        height: parent.height
        anchors.topMargin: 10
        anchors.bottomMargin: 10
        spacing: 10
        Text {
            text: "Экран"
        }
        ComboBox {
            id: screen
            model: settings.getMonitorModel()
            textRole: "text"
            valueRole: "value"
            currentIndex: settings.getMonitor_Index()
            
            onCurrentIndexChanged:{
                settings.setMonitor(currentIndex)
            }
            onActiveFocusChanged:{
                if(activeFocus){
                    screen.model = settings.getMonitorModel()
                }
            }
        }
        
    }
}
