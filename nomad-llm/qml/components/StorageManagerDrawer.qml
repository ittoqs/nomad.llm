import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import ".."

Drawer {
    id: storageDrawer
    edge: Qt.RightEdge
    width: Math.min(parent.width * 0.85, 450)
    height: parent.height

    background: Rectangle {
        color: Theme.bgPanel
        border.color: Theme.border
    }

    signal closedDrawer()
    
    property var modelsList: []
    
    function refreshModels() {
        modelsList = ModelManager.getDownloadedModels()
        storageListModel.clear()
        for (var i = 0; i < modelsList.length; i++) {
            storageListModel.append(modelsList[i])
        }
    }
    
    onOpened: {
        refreshModels()
    }
    
    Connections {
        target: ModelManager
        function onModelDeleted() { refreshModels(); }
        function onQuantizeFinished() { refreshModels(); }
        function onDownloadFinished() { refreshModels(); }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 24
        spacing: 20

        // Header
        RowLayout {
            Layout.fillWidth: true
            Text {
                text: "💾 Storage Manager"
                color: Theme.textMain
                font.pixelSize: 20
                font.bold: true
                Layout.fillWidth: true
            }
            Rectangle {
                width: 32; height: 32; radius: 8
                color: closeBtn.containsMouse ? Theme.bgItemHover : "transparent"
                Text { text: "×"; color: Theme.textSecondary; font.pixelSize: 20; anchors.centerIn: parent }
                MouseArea {
                    id: closeBtn
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    hoverEnabled: true
                    onClicked: storageDrawer.close()
                }
            }
        }
        
        Rectangle { Layout.fillWidth: true; height: 1; color: Theme.bgItemHover }
        
        // List of models
        ListModel { id: storageListModel }
        
        ListView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            spacing: 12
            model: storageListModel
            
            delegate: Rectangle {
                width: ListView.view.width
                height: 120
                radius: 12
                color: Theme.bgPanelElevated
                border.color: Theme.border
                
                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 12
                    spacing: 8
                    
                    RowLayout {
                        Layout.fillWidth: true
                        Text {
                            text: model.filename
                            color: Theme.textMain
                            font.pixelSize: 14
                            font.bold: true
                            elide: Text.ElideRight
                            Layout.fillWidth: true
                        }
                    }
                    
                    Text {
                        text: "Size: " + model.sizeMB.toFixed(2) + " MB"
                        color: Theme.textMuted
                        font.pixelSize: 12
                    }
                    
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8
                        
                        // Delete Button
                        Rectangle {
                            width: 80; height: 32; radius: 8
                            color: Theme.dangerBg
                            Text { text: "Delete"; color: Theme.danger; font.bold: true; anchors.centerIn: parent; font.pixelSize: 12 }
                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: {
                                    ModelManager.deleteModel(model.filename)
                                }
                            }
                        }
                        
                        // Quantize ComboBox
                        ComboBox {
                            id: formatCombo
                            Layout.fillWidth: true
                            height: 32
                            model: ["Q4_K_M", "Q5_K_M", "Q4_0", "Q8_0"]
                            background: Rectangle { color: Theme.bgItem; radius: 8; border.color: Theme.border }
                            contentItem: Text {
                                text: formatCombo.currentText
                                color: Theme.textMain
                                verticalAlignment: Text.AlignVCenter
                                leftPadding: 8
                            }
                        }
                        
                        // Quantize Button
                        Rectangle {
                            width: 80; height: 32; radius: 8
                            color: (ModelManager.quantizing && ModelManager.quantizingModel === model.filename) ? Theme.warningBg : Theme.primary
                            Text {
                                text: (ModelManager.quantizing && ModelManager.quantizingModel === model.filename) ? "Working" : "Quantize"
                                color: "white"
                                font.bold: true
                                anchors.centerIn: parent
                                font.pixelSize: 12
                            }
                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                enabled: !ModelManager.quantizing
                                onClicked: {
                                    ModelManager.quantizeModel(model.filename, formatCombo.currentText)
                                }
                            }
                        }
                    }
                    
                    // Progress Bar
                    Rectangle {
                        Layout.fillWidth: true
                        height: 4
                        radius: 2
                        color: Theme.bgItemHover
                        visible: ModelManager.quantizing && ModelManager.quantizingModel === model.filename
                        
                        Rectangle {
                            width: parent.width * ModelManager.quantizeProgress
                            height: parent.height
                            radius: 2
                            color: Theme.warning
                            Behavior on width { NumberAnimation { duration: 200 } }
                        }
                    }
                }
            }
            
            // Empty state
            Text {
                visible: storageListModel.count === 0
                text: "No models downloaded."
                color: Theme.textSecondary
                font.pixelSize: 14
                anchors.centerIn: parent
            }
        }
    }
}
