import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import ".."

Popup {
    id: popup
    width: Math.min(600, parent.width * 0.9)
    height: Math.min(600, parent.height * 0.9)
    anchors.centerIn: parent
    modal: true
    focus: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    property string imagePath: ""
    signal imageEdited(string newImagePath)

    background: Rectangle {
        color: Theme.bgPanelElevated
        radius: 12
        border.color: Theme.border
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12

        Text {
            text: "Edit Image"
            font.pixelSize: 18
            font.bold: true
            color: Theme.textMain
        }

        // Image Viewport
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: Theme.bgItem
            clip: true
            border.color: Theme.border

            // A very simple crop bounding box simulation
            Flickable {
                id: flickable
                anchors.fill: parent
                contentWidth: img.width
                contentHeight: img.height
                clip: true

                Image {
                    id: img
                    source: popup.imagePath
                    fillMode: Image.PreserveAspectFit
                    width: flickable.width
                    height: flickable.height
                    sourceSize.width: 1024
                    sourceSize.height: 1024
                }
                
                // Crop overlay mask (simple visual representation)
                Rectangle {
                    id: cropBox
                    x: img.width * 0.1
                    y: img.height * 0.1
                    width: img.width * 0.8
                    height: img.height * 0.8
                    color: "transparent"
                    border.color: Theme.primary
                    border.width: 2
                    visible: true
                    
                    // Allow dragging crop box
                    Drag.active: dragArea.drag.active
                    
                    MouseArea {
                        id: dragArea
                        anchors.fill: parent
                        drag.target: cropBox
                        drag.axis: Drag.XAndYAxis
                    }
                }
            }
        }

        RowLayout {
            Layout.alignment: Qt.AlignRight
            spacing: 8

            Button {
                text: "Cancel"
                onClicked: popup.close()
            }

            Button {
                text: "Apply & Save"
                highlighted: true
                onClicked: {
                    // Simple crop implementation using QML grabToImage
                    cropBox.grabToImage(function(result) {
                        var tempPath = "file:///tmp/cropped_" + Date.now() + ".jpg";
                        result.saveToFile(tempPath.replace("file://", ""));
                        popup.imageEdited(tempPath);
                        popup.close();
                    }, Qt.size(cropBox.width, cropBox.height));
                }
            }
        }
    }
}
