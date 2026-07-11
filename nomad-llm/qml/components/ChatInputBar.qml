import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import ".."

ColumnLayout {
    spacing: 4

    property int currentSessionId: 0
    property string selectedImagePath: ""
    property bool isGenerating: false

    signal sendMessageRequested(string message)
    signal attachFileRequested()
    signal clearAttachmentRequested()

    // Image preview
    Rectangle {
        Layout.fillWidth: true
        Layout.leftMargin: 16
        Layout.rightMargin: 16
        height: 64
        color: "transparent"
        visible: selectedImagePath !== ""

        Rectangle {
            width: 56; height: 56; radius: 12
            color: Theme.bgItem
            border.color: Theme.primary

            Image {
                anchors.fill: parent; anchors.margins: 3
                source: selectedImagePath
                fillMode: Image.PreserveAspectCrop; clip: true
            }

            Rectangle {
                width: 18; height: 18; radius: 9; color: Theme.danger
                anchors.right: parent.right; anchors.top: parent.top; anchors.margins: -6
                Text { text: "×"; color: "white"; font.pixelSize: 12; font.bold: true; anchors.centerIn: parent }
                MouseArea {
                    anchors.fill: parent
                    onClicked: clearAttachmentRequested()
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
                    placeholderText: currentSessionId === 0 ? Settings.tr("create_session_first") :
                                     (!InferenceEngine.modelLoaded ? Settings.tr("select_model_first") : Settings.tr("type_message"))
                    color: Theme.textMain
                    font.pixelSize: 14
                    wrapMode: TextArea.Wrap
                    background: Rectangle { color: "transparent" }
                    enabled: InferenceEngine.modelLoaded && !isGenerating && currentSessionId !== 0
                    placeholderTextColor: Theme.textSecondary

                    Keys.onReturnPressed: function(event) {
                        if (event.modifiers & Qt.ShiftModifier) {
                            // Shift+Enter: new line
                            event.accepted = false;
                        } else {
                            event.accepted = true;
                            send()
                        }
                    }
                }
            }

            // --- Voice Recording Button ---
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
                    GradientStop { position: 0; color: (InferenceEngine.modelLoaded && inputField.text.trim().length > 0 && !isGenerating && currentSessionId !== 0) ? Theme.primary : Theme.bgItemHover }
                    GradientStop { position: 1; color: (InferenceEngine.modelLoaded && inputField.text.trim().length > 0 && !isGenerating && currentSessionId !== 0) ? Theme.primaryHover : Theme.bgItemHover }
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
                    enabled: InferenceEngine.modelLoaded && inputField.text.trim().length > 0 && !isGenerating && currentSessionId !== 0
                    onClicked: send()
                }
            }
        }
    }

    function send() {
        var msg = inputField.text.trim();
        if (msg !== "") {
            sendMessageRequested(msg);
            inputField.text = "";
        }
    }

    function setInputText(text) {
        inputField.text = text;
    }
}
