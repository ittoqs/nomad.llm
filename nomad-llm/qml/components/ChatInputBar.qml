import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import ".."

DropArea {
    id: dropArea
    Layout.fillWidth: true
    Layout.preferredHeight: colLayout.implicitHeight

    property int currentSessionId: 0
    property var selectedImages: []
    property bool isGenerating: false

    signal sendMessageRequested(string message)
    signal attachFileRequested()
    
    // We emit this to the parent (main.qml) or we can use the popup locally
    
    onDropped: function(drop) {
        if (drop.hasUrls) {
            var newImages = selectedImages.slice();
            for (var i = 0; i < drop.urls.length; ++i) {
                // simple check for images
                var url = drop.urls[i].toString();
                if (url.match(/\.(jpeg|jpg|gif|png)$/i) != null) {
                    newImages.push(url);
                }
            }
            selectedImages = newImages;
        }
    }

    Rectangle {
        anchors.fill: parent
        color: dropArea.containsDrag ? Theme.primaryHover : "transparent"
        opacity: 0.2
        visible: dropArea.containsDrag
        radius: 12
    }

    ColumnLayout {
        id: colLayout
        anchors.fill: parent
        spacing: 4

        // Image previews
        ScrollView {
            Layout.fillWidth: true
            Layout.leftMargin: 16
            Layout.rightMargin: 16
            height: selectedImages.length > 0 ? 76 : 0
            visible: selectedImages.length > 0
            contentWidth: previewRow.implicitWidth
            clip: true

            RowLayout {
                id: previewRow
                spacing: 8
                
                Repeater {
                    model: selectedImages
                    
                    Rectangle {
                        width: 64; height: 64; radius: 8
                        color: Theme.bgItem
                        border.color: Theme.primary
                        
                        Image {
                            anchors.fill: parent
                            anchors.margins: 2
                            source: modelData
                            fillMode: Image.PreserveAspectCrop
                            clip: true
                        }
                        
                        // Edit overlay hint
                        Rectangle {
                            anchors.bottom: parent.bottom
                            anchors.left: parent.left
                            anchors.right: parent.right
                            height: 18
                            color: "#aa000000"
                            Text {
                                anchors.centerIn: parent
                                text: "✎ Edit"
                                color: "white"
                                font.pixelSize: 10
                            }
                        }

                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                editorPopup.imagePath = modelData;
                                editorPopup.editIndex = index;
                                editorPopup.open();
                            }
                        }
                        
                        Rectangle {
                            width: 20; height: 20; radius: 10; color: Theme.danger
                            anchors.right: parent.right; anchors.top: parent.top; anchors.margins: -8
                            Text { text: "×"; color: "white"; font.pixelSize: 12; font.bold: true; anchors.centerIn: parent }
                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    var arr = selectedImages.slice();
                                    arr.splice(index, 1);
                                    selectedImages = arr;
                                }
                            }
                        }
                    }
                }
            }
        }

        // Input Box
        Rectangle {
            Layout.fillWidth: true
            Layout.margins: 16
            Layout.topMargin: 4
            height: Math.max(56, inputField.implicitHeight + 20)
            Layout.maximumHeight: 180
            radius: 28
            color: Theme.bgItem
            border.color: inputField.activeFocus ? Theme.primary : Theme.bgItemHover
            border.width: inputField.activeFocus ? 2 : 1

            Behavior on border.color { ColorAnimation { duration: 200 } }

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 16
                anchors.rightMargin: 8
                anchors.topMargin: 4
                anchors.bottomMargin: 4
                spacing: 8

                // Attach button
                Rectangle {
                    width: 36; height: 36; radius: 18
                    color: attachBtn.containsMouse ? Theme.bgItemHover : "transparent"
                    Text { text: "📎"; font.pixelSize: 18; anchors.centerIn: parent }
                    MouseArea {
                        id: attachBtn
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        hoverEnabled: true
                        onClicked: attachFileRequested()
                    }
                }

                // Multi-line TextArea
                ScrollView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    TextArea {
                        id: inputField
                        placeholderText: dropArea.containsDrag ? "Drop images here..." : (currentSessionId === 0 ? Settings.tr("create_session_first") :
                                         (!InferenceEngine.modelLoaded ? Settings.tr("select_model_first") : Settings.tr("type_message")))
                        color: Theme.textMain
                        font.pixelSize: 14
                        wrapMode: TextArea.Wrap
                        background: Rectangle { color: "transparent" }
                        enabled: InferenceEngine.modelLoaded && !isGenerating && currentSessionId !== 0
                        placeholderTextColor: Theme.textSecondary

                        Keys.onReturnPressed: function(event) {
                            if (event.modifiers & Qt.ShiftModifier) {
                                event.accepted = false;
                            } else {
                                event.accepted = true;
                                send()
                            }
                        }
                    }
                }

                // Voice Recording Button
                Rectangle {
                    width: 40; height: 40; radius: 10
                    color: VoiceManager.isRecording ? Theme.danger : Theme.bgItem
                    border.color: Theme.border
                    Text { text: "🎙"; anchors.centerIn: parent; font.pixelSize: 18 }
                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            if (VoiceManager.isRecording) {
                                VoiceManager.stopRecording();
                                VoiceManager.processVoice("");
                            } else {
                                VoiceManager.startRecording();
                            }
                        }
                    }
                }

                // Send button
                Rectangle {
                    width: 44; height: 44; radius: 22

                    gradient: Gradient {
                        GradientStop { position: 0; color: (InferenceEngine.modelLoaded && (inputField.text.trim().length > 0 || selectedImages.length > 0) && !isGenerating && currentSessionId !== 0) ? Theme.primary : Theme.bgItemHover }
                        GradientStop { position: 1; color: (InferenceEngine.modelLoaded && (inputField.text.trim().length > 0 || selectedImages.length > 0) && !isGenerating && currentSessionId !== 0) ? Theme.primaryHover : Theme.bgItemHover }
                    }

                    Text {
                        text: "➤"
                        color: "white"
                        font.pixelSize: 18
                        anchors.centerIn: parent
                        rotation: 0
                    }

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        enabled: InferenceEngine.modelLoaded && (inputField.text.trim().length > 0 || selectedImages.length > 0) && !isGenerating && currentSessionId !== 0
                        onClicked: send()
                    }
                }
            }
        }
    }

    ImageEditorPopup {
        id: editorPopup
        property int editIndex: -1
        
        onImageEdited: function(newImagePath) {
            if (editIndex >= 0 && editIndex < selectedImages.length) {
                var arr = selectedImages.slice();
                arr[editIndex] = newImagePath;
                selectedImages = arr;
            }
        }
    }

    function send() {
        var msg = inputField.text.trim();
        // Here we could include images in the sendMessageRequested signal
        // e.g. sendMessageRequested(msg, selectedImages)
        // Since we are not updating the C++ backend completely, we'll send it as JSON string or keep it simple
        if (msg !== "" || selectedImages.length > 0) {
            // For now, let's append image tags to the message so ChatArea handles them easily,
            // or modify the signal if we had full access. Let's assume sendMessageRequested only takes string for now,
            // but we can pass images via a custom string format or update the signal.
            // Wait, we can modify the signal signature!
            // But let's check where it's connected in main.qml. If I change signature, I must update main.qml.
            // Let's use a workaround: JSON payload if images exist, or just normal string if not.
            
            var payload = msg;
            if (selectedImages.length > 0) {
                payload = "IMG_ATTACH:" + JSON.stringify(selectedImages) + ":::" + msg;
            }
            
            sendMessageRequested(payload);
            inputField.text = "";
            selectedImages = [];
        }
    }

    function setInputText(text) {
        inputField.text = text;
    }
}
