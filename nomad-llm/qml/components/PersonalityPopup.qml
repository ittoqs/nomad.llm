import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import ".."

Popup {
    id: personalityPopup
    width: 320
    height: 400
    modal: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    property string currentPersonality: "general"
    property string currentSystemPrompt: ""

    signal personalitySelected(string personalityId, string prompt)
    signal systemPromptChanged(string prompt)

    background: Rectangle {
        color: Theme.bgPanelElevated
        radius: 16
        border.color: Theme.border
        border.width: 1
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12

        Text {
            text: "🎭 " + Settings.tr("personality")
            color: Theme.textMain
            font.pixelSize: 16
            font.bold: true
        }

        ListView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            spacing: 6
            model: Settings.getPersonalityPresets()

            delegate: Rectangle {
                width: ListView.view.width
                height: 56
                radius: 10
                color: currentPersonality === modelData.id ? Theme.bgActive : Theme.bgPanel
                border.color: currentPersonality === modelData.id ? Theme.primary : Theme.bgItemHover

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 12
                    spacing: 10

                    Text { text: modelData.icon; font.pixelSize: 20 }
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 0
                        Text { text: modelData.name; color: Theme.textMain; font.pixelSize: 13; font.bold: true }
                        Text {
                            text: modelData.prompt.substring(0, 50) + "..."
                            color: Theme.textMuted; font.pixelSize: 10
                            elide: Text.ElideRight
                            Layout.fillWidth: true
                        }
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        personalityPopup.personalitySelected(modelData.id, modelData.prompt)
                        personalityPopup.close()
                    }
                }
            }
        }

        // Custom prompt
        Text {
            text: Settings.tr("system_prompt")
            color: Theme.textMuted
            font.pixelSize: 12
            font.bold: true
        }

        Rectangle {
            Layout.fillWidth: true
            height: 60
            radius: 8
            color: Theme.bgPanel
            border.color: Theme.border

            TextArea {
                anchors.fill: parent
                anchors.margins: 8
                text: currentSystemPrompt
                color: Theme.textMain
                font.pixelSize: 11
                wrapMode: TextArea.Wrap
                background: Rectangle { color: "transparent" }
                placeholderText: "Custom system prompt..."
                placeholderTextColor: Theme.textMuted
                onTextChanged: {
                    if (currentSystemPrompt !== text) {
                        personalityPopup.systemPromptChanged(text)
                    }
                }
            }
        }
    }
}
