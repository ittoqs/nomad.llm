import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import ".."

Rectangle {
    id: sidebar
    color: Theme.bgPanel
    clip: true

    property int currentSessionId: 0
    property var sessionsModel
    property double ramGB: 0
    property string activeFilename: ""
    property string activeModelName: ""

    signal openSettingsRequested()
    signal newSessionRequested()
    signal sessionSelected(int sessionId)
    signal sessionDeleted(int sessionId)
    signal modelDownloadRequested(string repoId, string filename, string mmproj)
    signal modelLoadRequested(string name, string filename)

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12

        // Logo + Toggle
        RowLayout {
            Layout.fillWidth: true
            Text {
                text: "🌊 Nomad LLM"
                color: Theme.textMain
                font.pixelSize: 20
                font.bold: true
                Layout.fillWidth: true
            }
            // Settings gear
            Rectangle {
                width: 32; height: 32; radius: 8
                color: settingsArea.hovered ? Theme.bgItemHover : "transparent"
                property bool hovered: false
                Text { text: "⚙"; font.pixelSize: 16; anchors.centerIn: parent; color: Theme.textSecondary }
                MouseArea {
                    id: settingsArea
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    hoverEnabled: true
                    property bool hovered: false
                    onEntered: parent.hovered = true
                    onExited: parent.hovered = false
                    onClicked: sidebar.openSettingsRequested()
                }
            }
        }

        // New Chat button
        Rectangle {
            Layout.fillWidth: true
            height: 44
            radius: 12
            gradient: Gradient {
                GradientStop { position: 0; color: Theme.primary }
                GradientStop { position: 1; color: Theme.primaryHover }
            }

            RowLayout {
                anchors.centerIn: parent
                spacing: 8
                Text { text: "+"; color: "white"; font.pixelSize: 18; font.bold: true }
                Text { text: Settings.tr("new_chat"); color: "white"; font.pixelSize: 14; font.bold: true }
            }

            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: sidebar.newSessionRequested()
            }
        }

        // Chat History header
        Text {
            text: Settings.tr("chat_history")
            color: Theme.textMuted
            font.pixelSize: 12
            font.bold: true
            Layout.topMargin: 8
        }

        // Session list
        ListView {
            id: sessionList
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.maximumHeight: parent.height * 0.35
            clip: true
            spacing: 4
            model: sessionsModel

            delegate: Rectangle {
                width: ListView.view.width
                height: 48
                radius: 10
                color: sidebar.currentSessionId === model.id ? Theme.bgItemHover : (sessionHover.containsMouse ? Theme.bgItem : "transparent")
                border.color: sidebar.currentSessionId === model.id ? Theme.primary : "transparent"
                border.width: sidebar.currentSessionId === model.id ? 1 : 0

                Behavior on color { ColorAnimation { duration: 150 } }

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 10
                    spacing: 8

                    Text { text: "💬"; font.pixelSize: 14 }
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 0
                        Text {
                            text: model.title
                            color: Theme.textMain
                            font.pixelSize: 13
                            elide: Text.ElideRight
                            Layout.fillWidth: true
                        }
                        Text {
                            text: (model.message_count || 0) + " msgs"
                            color: Theme.textSecondary
                            font.pixelSize: 10
                        }
                    }

                    // Delete button
                    Rectangle {
                        width: 24; height: 24; radius: 6
                        color: delHover.containsMouse ? Theme.dangerBg : "transparent"
                        visible: sessionHover.containsMouse
                        Text { text: "×"; color: Theme.danger; font.pixelSize: 16; font.bold: true; anchors.centerIn: parent }
                        MouseArea {
                            id: delHover
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            hoverEnabled: true
                            onClicked: sidebar.sessionDeleted(model.id)
                        }
                    }
                }

                MouseArea {
                    id: sessionHover
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    hoverEnabled: true
                    acceptedButtons: Qt.LeftButton
                    onClicked: sidebar.sessionSelected(model.id)
                    z: -1
                }
            }

            // Empty state
            Text {
                visible: sessionsModel.count === 0
                text: Settings.tr("no_sessions")
                color: Theme.textSecondary
                font.pixelSize: 13
                anchors.centerIn: parent
            }
        }

        // Separator
        Rectangle { Layout.fillWidth: true; height: 1; color: Theme.bgItemHover }

        // Models section
        Text {
            text: Settings.tr("models")
            color: Theme.textMuted
            font.pixelSize: 12
            font.bold: true
        }

        ListView {
            id: modelListView
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            spacing: 6
            model: ModelManager.getModelCatalog(sidebar.ramGB)

            header: Component {
                ColumnLayout {
                    width: ListView.view ? ListView.view.width : 400
                    spacing: 8
                    // --- Custom Model Download ---
                    Rectangle {
                        width: ListView.view ? ListView.view.width : parent.width
                        height: 120
                        radius: 12
                        color: Theme.bgPanelElevated
                        border.color: Theme.border

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 16
                            spacing: 8

                            Text { text: "Custom HuggingFace Model"; color: Theme.textMain; font.pixelSize: 14; font.bold: true }

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 8

                                TextField {
                                    id: customRepoIdDrawer
                                    Layout.fillWidth: true
                                    placeholderText: "Repo ID (e.g. bartowski/Llama-3-8B-Instruct-GGUF)"
                                    color: Theme.textMain
                                    background: Rectangle { color: Theme.bgItem; radius: 6; border.color: Theme.border }
                                }

                                TextField {
                                    id: customFilenameDrawer
                                    Layout.fillWidth: true
                                    placeholderText: "Filename (e.g. model.gguf)"
                                    color: Theme.textMain
                                    background: Rectangle { color: Theme.bgItem; radius: 6; border.color: Theme.border }
                                }
                            }

                            Rectangle {
                                Layout.alignment: Qt.AlignRight
                                width: 120; height: 32; radius: 16
                                color: ModelManager.downloading ? Theme.textMuted : Theme.primary

                                Text { text: "Download"; color: "white"; font.pixelSize: 12; font.bold: true; anchors.centerIn: parent }

                                MouseArea {
                                    anchors.fill: parent
                                    cursorShape: Qt.PointingHandCursor
                                    enabled: !ModelManager.downloading && customRepoIdDrawer.text.length > 0 && customFilenameDrawer.text.length > 0
                                    onClicked: {
                                        sidebar.modelDownloadRequested(customRepoIdDrawer.text.trim(), customFilenameDrawer.text.trim(), "");
                                    }
                                }
                            }
                        }
                    }
                }
            }

            delegate: Rectangle {
                width: ListView.view.width
                height: 84
                radius: 12
                color: sidebar.activeFilename === modelData.filename ? Theme.bgActive : (modelData.compatible ? Theme.bgPanelElevated : Theme.bgMain)
                border.color: sidebar.activeFilename === modelData.filename ? Theme.primary : (modelData.downloaded ? Theme.success : Theme.bgItemHover)
                border.width: sidebar.activeFilename === modelData.filename ? 2 : 1
                opacity: modelData.compatible ? 1.0 : 0.45

                Behavior on border.color { ColorAnimation { duration: 200 } }

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 10
                    spacing: 4

                    RowLayout {
                        Layout.fillWidth: true
                        Text {
                            text: modelData.name
                            color: Theme.textMain
                            font.pixelSize: 13
                            font.bold: true
                            elide: Text.ElideRight
                            Layout.fillWidth: true
                        }
                        Rectangle {
                            width: tagText.width + 10; height: 18; radius: 9
                            color: Theme.bgActive
                            Text { id: tagText; text: modelData.type; color: Theme.textAccent; font.pixelSize: 9; font.bold: true; anchors.centerIn: parent }
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        Text {
                            text: Settings.tr("min_ram") + ": " + modelData.minRamGB + " GB"
                            color: Theme.textMuted
                            font.pixelSize: 10
                            Layout.fillWidth: true
                        }

                        // Status / Actions
                        Rectangle {
                            width: 64; height: 26; radius: 13
                            color: {
                                if (sidebar.activeFilename === modelData.filename) return Theme.primary;
                                if (modelData.downloaded) return Theme.successBg;
                                if (ModelManager.downloading && ModelManager.downloadingModel === modelData.filename) return Theme.warningBg;
                                return Theme.bgItemHover;
                            }
                            visible: modelData.compatible

                            Text {
                                text: {
                                    if (sidebar.activeFilename === modelData.filename) return "Active";
                                    if (ModelManager.downloading && ModelManager.downloadingModel === modelData.filename) return Math.round(ModelManager.downloadProgress * 100) + "%";
                                    if (modelData.downloaded) return "Load";
                                    return "↓";
                                }
                                color: "white"
                                font.pixelSize: 10
                                font.bold: true
                                anchors.centerIn: parent
                            }

                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                enabled: modelData.compatible && !ModelManager.downloading && sidebar.activeFilename !== modelData.filename
                                onClicked: {
                                    if (modelData.downloaded) {
                                        sidebar.modelLoadRequested(modelData.name, modelData.filename);
                                    } else {
                                        var mmproj = modelData.mmprojFilename || "";
                                        sidebar.modelDownloadRequested(modelData.repoId, modelData.filename, mmproj);
                                    }
                                }
                            }
                        }
                    }

                    // Download progress bar
                    Rectangle {
                        Layout.fillWidth: true
                        height: 3
                        radius: 2
                        color: Theme.bgItemHover
                        visible: ModelManager.downloading && ModelManager.downloadingModel === modelData.filename

                        Rectangle {
                            width: parent.width * ModelManager.downloadProgress
                            height: parent.height
                            radius: 2
                            color: Theme.warning
                            Behavior on width { NumberAnimation { duration: 200 } }
                        }
                    }
                }
            }
        }
    }
}
