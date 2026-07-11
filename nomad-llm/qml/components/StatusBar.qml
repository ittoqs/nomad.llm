import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import ".."

Rectangle {
    Layout.fillWidth: true
    height: 32
    color: Theme.bgMain

    property double ramGB: 0
    property double availRamGB: 0

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 20
        anchors.rightMargin: 20
        spacing: 16

        // Connection status
        Row {
            spacing: 6
            Rectangle {
                width: 8; height: 8; radius: 4
                color: InferenceEngine.modelLoaded ? Theme.success : Theme.danger
                anchors.verticalCenter: parent.verticalCenter
            }
            Text {
                text: InferenceEngine.modelLoaded ? Settings.tr("connected") : Settings.tr("disconnected")
                color: Theme.textMuted; font.pixelSize: 11
            }
        }

        // Active model
        Text {
            text: InferenceEngine.loadedModelName ? ("🤖 " + InferenceEngine.loadedModelName) : ""
            color: Theme.textSecondary; font.pixelSize: 11
            visible: text !== ""
        }

        Item { Layout.fillWidth: true }

        // Tokens/sec
        Text {
            text: InferenceEngine.generating ? (InferenceEngine.tokensPerSecond.toFixed(1) + " " + Settings.tr("tokens_per_sec")) : ""
            color: Theme.success; font.pixelSize: 11; font.bold: true
            visible: InferenceEngine.generating
        }

        // RAM
        Text {
            text: Settings.tr("ram") + ": " + availRamGB.toFixed(1) + "/" + ramGB.toFixed(1) + " GB"
            color: Theme.textSecondary; font.pixelSize: 11
        }

        // Download speed
        Text {
            text: ModelManager.downloading ? ("📥 " + ModelManager.downloadSpeedMBps.toFixed(1) + " MB/s") : ""
            color: Theme.warning; font.pixelSize: 11; font.bold: true
            visible: ModelManager.downloading
        }
    }
}
