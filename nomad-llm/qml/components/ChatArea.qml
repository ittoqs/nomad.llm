import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import ".."

ListView {
    id: chatList
    clip: true
    spacing: 12
    cacheBuffer: 2000

    property var chatModel
    property bool isThinking: false

    model: chatModel

    // Empty state
    Text {
        visible: chatModel.count === 0
        text: "🌊\n\n" + Settings.tr("welcome_title") + "\n" + Settings.tr("welcome_subtitle")
        color: Theme.textMuted
        font.pixelSize: 16
        horizontalAlignment: Text.AlignHCenter
        wrapMode: Text.WordWrap
        anchors.centerIn: parent
        width: parent.width * 0.6
    }

    delegate: Item {
        width: chatList.width
        height: bubble.height + 8
        property bool isUser: model.sender === "User"

        Rectangle {
            id: bubble
            width: Math.min(Math.max(msgContent.implicitWidth + 32, 100), chatList.width * 0.82)
            height: msgContent.implicitHeight + 28
            radius: 16
            anchors.right: isUser ? parent.right : undefined
            anchors.left: isUser ? undefined : parent.left

            gradient: Gradient {
                GradientStop { position: 0; color: isUser ? Theme.primaryHover : Theme.bgItem }
                GradientStop { position: 1; color: isUser ? Theme.primaryHover : Theme.bgPanelElevated }
            }

            border.color: isUser ? Theme.primary : Theme.bgItemHover
            border.width: 1

            ColumnLayout {
                id: msgContent
                anchors.fill: parent
                anchors.margins: 14
                spacing: 4

                // Sender label
                Text {
                    text: isUser ? "You" : "Nomad"
                    color: isUser ? Theme.textAccent : Theme.textAccent
                    font.pixelSize: 11
                    font.bold: true
                }

                Text {
                    text: model.text
                    color: Theme.textMain
                    wrapMode: Text.WordWrap
                    font.pixelSize: Settings.fontSize
                    textFormat: Text.MarkdownText
                    Layout.fillWidth: true
                    onLinkActivated: function(link) { Qt.openUrlExternally(link) }
                }
            }

            // Action buttons on hover
            Row {
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.margins: 6
                spacing: 4
                visible: !isUser && bubbleHover.containsMouse

                // Copy
                Rectangle {
                    width: 28; height: 28; radius: 6
                    color: copyBtn.containsMouse ? Theme.textMuted : Theme.bgItemHover
                    Text { text: "📋"; font.pixelSize: 12; anchors.centerIn: parent }
                    MouseArea {
                        id: copyBtn
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        hoverEnabled: true
                        onClicked: {
                            Clipboard.setText(model.text)
                            // Optional: show a toast here
                        }
                    }
                }

                // Delete
                Rectangle {
                    width: 28; height: 28; radius: 6
                    color: delBtn.containsMouse ? Theme.textMuted : Theme.bgItemHover
                    Text { text: "🗑️"; font.pixelSize: 12; anchors.centerIn: parent }
                    MouseArea {
                        id: delBtn
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        hoverEnabled: true
                        onClicked: {
                            chatModel.remove(index)
                        }
                    }
                }
            }

            MouseArea {
                id: bubbleHover
                anchors.fill: parent
                hoverEnabled: true
                acceptedButtons: Qt.NoButton
                z: -1
            }
        }
    }

    // Scroll to bottom button
    Rectangle {
        width: 40; height: 40; radius: 20
        color: Theme.bgItemHover
        border.color: Theme.textMuted
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 8
        visible: !chatList.atYEnd && chatModel.count > 5

        Text { text: "↓"; color: Theme.textSecondary; font.pixelSize: 18; anchors.centerIn: parent }
        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: chatList.positionViewAtEnd()
        }
    }

    // Thinking indicator as a footer
    footer: Item {
        width: chatList.width
        height: isThinking ? 40 : 0
        visible: isThinking

        RowLayout {
            anchors.left: parent.left
            anchors.leftMargin: 32
            anchors.verticalCenter: parent.verticalCenter
            spacing: 6

            Repeater {
                model: 3
                Rectangle {
                    width: 8; height: 8; radius: 4
                    color: Theme.primary

                    SequentialAnimation on opacity {
                        loops: Animation.Infinite
                        NumberAnimation { to: 0.3; duration: 300; easing.type: Easing.InOutQuad }
                        PauseAnimation { duration: index * 150 }
                        NumberAnimation { to: 1.0; duration: 300; easing.type: Easing.InOutQuad }
                        PauseAnimation { duration: (2 - index) * 150 }
                    }
                }
            }

            Text { text: Settings.tr("thinking"); color: Theme.textMuted; font.pixelSize: 12; font.italic: true }
        }
    }
}
