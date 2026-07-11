import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import ".."

ColumnLayout {
    spacing: 16
    anchors.centerIn: parent

    property double ramGB: 0
    property double diskGB: 0

    Text {
        text: "🔍 " + t("hardware_detected")
        color: Theme.textMain
        font.pixelSize: 22
        font.bold: true
        Layout.alignment: Qt.AlignHCenter
    }

    // Hardware cards
    Repeater {
        model: [
            { icon: "🧠", label: t("ram"), value: ramGB.toFixed(1) + " GB" },
            { icon: "⚡", label: t("cpu"), value: HardwareDetector.cpuCores + " " + t("cores") },
            { icon: "🎮", label: t("gpu"), value: HardwareDetector.gpuName },
            { icon: "💾", label: t("free_space"), value: diskGB.toFixed(1) + " GB" }
        ]

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 52
            radius: 12
            color: Theme.bgPanelElevated
            border.color: Theme.border

            RowLayout {
                anchors.fill: parent
                anchors.margins: 14
                Text { text: modelData.icon; font.pixelSize: 20 }
                Text { text: modelData.label; color: Theme.textSecondary; font.pixelSize: 14; Layout.fillWidth: true }
                Text { text: modelData.value; color: Theme.textMain; font.pixelSize: 14; font.bold: true }
            }
        }
    }

    Rectangle {
        Layout.fillWidth: true
        height: 48
        radius: 12
        color: Theme.bgPanel
        border.color: Theme.success

        RowLayout {
            anchors.fill: parent
            anchors.margins: 14
            Text { text: "✨"; font.pixelSize: 18 }
            Text { text: t("recommended") + ": " + HardwareDetector.recommendedModel; color: Theme.success; font.pixelSize: 13; font.bold: true }
        }
    }
}
