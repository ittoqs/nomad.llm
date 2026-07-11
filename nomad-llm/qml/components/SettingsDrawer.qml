import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import ".."

Drawer {
    id: settingsDrawer
    edge: Qt.RightEdge

    signal clearDataRequested()
    signal storageManagerRequested()

    background: Rectangle {
        color: Theme.bgPanel
        border.color: Theme.border
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 24
        spacing: 20

        // Header
        RowLayout {
            Layout.fillWidth: true
            Text {
                text: "⚙ " + Settings.tr("settings")
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
                    onClicked: settingsDrawer.close()
                }
            }
        }

        Rectangle { Layout.fillWidth: true; height: 1; color: Theme.bgItemHover }

        // Font size
        ColumnLayout {
            spacing: 8
            Text { text: "🔤 " + Settings.tr("font_size") + ": " + Settings.fontSize + "px"; color: Theme.textSecondary; font.pixelSize: 13; font.bold: true }
            Slider {
                Layout.fillWidth: true
                from: 10; to: 24; stepSize: 1
                value: Settings.fontSize
                onMoved: Settings.fontSize = value

                background: Rectangle {
                    x: parent.leftPadding; y: parent.topPadding + parent.availableHeight / 2 - height / 2
                    width: parent.availableWidth; height: 4; radius: 2; color: Theme.bgItemHover
                    Rectangle { width: parent.parent.visualPosition * parent.width; height: parent.height; radius: 2; color: Theme.primary }
                }
                handle: Rectangle {
                    x: parent.leftPadding + parent.visualPosition * (parent.availableWidth - width)
                    y: parent.topPadding + parent.availableHeight / 2 - height / 2
                    width: 20; height: 20; radius: 10; color: Theme.primary; border.color: Theme.textAccent
                }
            }
        }

        // Temperature
        ColumnLayout {
            spacing: 8
            Text { text: "🌡 " + Settings.tr("temperature_label") + ": " + Settings.temperature.toFixed(1); color: Theme.textSecondary; font.pixelSize: 13; font.bold: true }
            Slider {
                Layout.fillWidth: true
                from: 0.0; to: 2.0; stepSize: 0.1
                value: Settings.temperature
                onMoved: Settings.temperature = value

                background: Rectangle {
                    x: parent.leftPadding; y: parent.topPadding + parent.availableHeight / 2 - height / 2
                    width: parent.availableWidth; height: 4; radius: 2; color: Theme.bgItemHover
                    Rectangle { width: parent.parent.visualPosition * parent.width; height: parent.height; radius: 2; color: Theme.warning }
                }
                handle: Rectangle {
                    x: parent.leftPadding + parent.visualPosition * (parent.availableWidth - width)
                    y: parent.topPadding + parent.availableHeight / 2 - height / 2
                    width: 20; height: 20; radius: 10; color: Theme.warning; border.color: Theme.warning
                }
            }
        }

        // Max Response Tokens
        ColumnLayout {
            spacing: 8
            Text { text: "📝 " + Settings.tr("max_tokens") + ": " + Settings.maxResponseTokens; color: Theme.textSecondary; font.pixelSize: 13; font.bold: true }
            Slider {
                Layout.fillWidth: true
                from: 128; to: 8192; stepSize: 128
                value: Settings.maxResponseTokens
                onMoved: Settings.maxResponseTokens = value

                background: Rectangle {
                    x: parent.leftPadding; y: parent.topPadding + parent.availableHeight / 2 - height / 2
                    width: parent.availableWidth; height: 4; radius: 2; color: Theme.bgItemHover
                    Rectangle { width: parent.parent.visualPosition * parent.width; height: parent.height; radius: 2; color: Theme.success }
                }
                handle: Rectangle {
                    x: parent.leftPadding + parent.visualPosition * (parent.availableWidth - width)
                    y: parent.topPadding + parent.availableHeight / 2 - height / 2
                    width: 20; height: 20; radius: 10; color: Theme.success; border.color: Theme.success
                }
            }
        }

        Item { Layout.fillHeight: true }

        // Manage Storage
        Rectangle {
            Layout.fillWidth: true
            height: 40
            radius: 8
            color: storageBtn.containsMouse ? Theme.primaryHover : Theme.primary
            
            Text {
                text: "Manage Storage"
                color: "white"
                font.pixelSize: 13
                font.bold: true
                anchors.centerIn: parent
            }

            MouseArea {
                id: storageBtn
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: {
                    settingsDrawer.storageManagerRequested()
                }
            }
        }

        // Clear Data
        Rectangle {
            Layout.fillWidth: true
            height: 40
            radius: 8
            color: clearDataBtn.containsMouse ? Theme.dangerHover : Theme.dangerBg
            
            Text {
                text: "Clear All Data"
                color: "white"
                font.pixelSize: 13
                font.bold: true
                anchors.centerIn: parent
            }

            MouseArea {
                id: clearDataBtn
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: {
                    Database.clearAllData();
                    settingsDrawer.clearDataRequested()
                }
            }
        }

        // About
        Rectangle {
            Layout.fillWidth: true
            height: 60
            radius: 12
            color: Theme.bgPanelElevated

            ColumnLayout {
                anchors.centerIn: parent
                spacing: 2
                Text { text: "🌊 Nomad LLM v" + AppVersion; color: Theme.textSecondary; font.pixelSize: 13; Layout.alignment: Qt.AlignHCenter }
                Text { text: "Fully Embedded C++ · Offline-First · Privacy-First"; color: Theme.textSecondary; font.pixelSize: 10; Layout.alignment: Qt.AlignHCenter }
            }
        }
    }
}
