import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import ".." // To access Theme if needed, though singletons usually don't need this if they are registered properly, but it's safe.

ColumnLayout {
    spacing: 20
    anchors.centerIn: parent

    Text {
        text: "🌊"
        font.pixelSize: 72
        Layout.alignment: Qt.AlignHCenter
    }
    Text {
        text: t("welcome_title")
        color: Theme.textMain
        font.pixelSize: 28
        font.bold: true
        Layout.alignment: Qt.AlignHCenter
    }
    Text {
        text: t("welcome_subtitle")
        color: Theme.textSecondary
        font.pixelSize: 16
        Layout.alignment: Qt.AlignHCenter
        horizontalAlignment: Text.AlignHCenter
        wrapMode: Text.WordWrap
        Layout.maximumWidth: 400
    }
}
