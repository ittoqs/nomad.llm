import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import ".."

ColumnLayout {
    spacing: 12
    anchors.fill: parent

    property double ramGB: 0
    property string activeFilename: ""

    Text {
        text: "📦 " + t("select_model")
        color: Theme.textMain
        font.pixelSize: 22
        font.bold: true
    }

    ListView {
        Layout.fillWidth: true
        Layout.fillHeight: true
        clip: true
        spacing: 8
        model: ModelManager.getModelCatalog(ramGB)

        delegate: Rectangle {
            width: ListView.view.width
            height: 72
            radius: 12
            color: modelData.compatible ? Theme.bgPanelElevated : Theme.bgPanel
            border.color: modelData.downloaded ? Theme.success : (modelData.compatible ? Theme.bgItemHover : Theme.bgItem)
            opacity: modelData.compatible ? 1.0 : 0.4

            RowLayout {
                anchors.fill: parent
                anchors.margins: 14
                spacing: 12

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 2
                    Text { text: modelData.name; color: Theme.textMain; font.pixelSize: 14; font.bold: true }
                    Text {
                        text: modelData.type + " · " + t("min_ram") + ": " + modelData.minRamGB + "GB"
                        color: Theme.textMuted; font.pixelSize: 11
                    }
                }

                Rectangle {
                    width: 80; height: 32; radius: 16
                    color: modelData.downloaded ? Theme.successBg : Theme.primary
                    visible: modelData.compatible

                    Text {
                        text: modelData.downloaded ? "✓" : t("download")
                        color: "white"
                        font.pixelSize: 12
                        font.bold: true
                        anchors.centerIn: parent
                    }
                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        enabled: !modelData.downloaded && !ModelManager.downloading
                        onClicked: {
                            var mmproj = modelData.mmprojFilename || "";
                            ModelManager.downloadModel(modelData.repoId, modelData.filename, mmproj);
                        }
                    }
                }
            }
        }
    }
}
