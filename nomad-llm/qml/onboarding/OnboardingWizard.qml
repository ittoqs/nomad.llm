import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import ".."

Rectangle {
    id: rootWizard
    color: Theme.bgMain

    property int onboardingStep: 0
    property double ramGB: 0
    property double diskGB: 0
    property string activeFilename: ""

    signal wizardFinished()

    ColumnLayout {
        anchors.centerIn: parent
        width: Math.min(parent.width * 0.85, 560)
        spacing: 32

        // Step indicators
        Row {
            Layout.alignment: Qt.AlignHCenter
            spacing: 12
            Repeater {
                model: 4
                Rectangle {
                    width: onboardingStep >= index ? 32 : 10
                    height: 10
                    radius: 5
                    color: onboardingStep >= index ? Theme.primary : Theme.bgItemHover
                    Behavior on width { NumberAnimation { duration: 250; easing.type: Easing.OutQuad } }
                    Behavior on color { ColorAnimation { duration: 250 } }
                }
            }
        }

        // Step content
        StackLayout {
            Layout.fillWidth: true
            Layout.preferredHeight: 400
            currentIndex: onboardingStep

            WelcomeStep { }
            HardwareScanStep { ramGB: rootWizard.ramGB; diskGB: rootWizard.diskGB }
            ModelSelectStep { ramGB: rootWizard.ramGB; activeFilename: rootWizard.activeFilename }
            ReadyStep { }
        }

        // Navigation
        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            Text {
                text: t("skip")
                color: Theme.textMuted
                font.pixelSize: 14
                visible: onboardingStep < 3
                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: rootWizard.wizardFinished()
                }
            }

            Item { Layout.fillWidth: true }

            Rectangle {
                width: 120
                height: 44
                radius: 22
                color: Theme.primary
                visible: onboardingStep > 0

                Text {
                    text: t("back")
                    color: "white"
                    font.pixelSize: 14
                    font.bold: true
                    anchors.centerIn: parent
                }
                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: onboardingStep--
                }

                Behavior on opacity { NumberAnimation { duration: 200 } }
            }

            Rectangle {
                width: 140
                height: 44
                radius: 22
                gradient: Gradient {
                    GradientStop { position: 0; color: Theme.primary }
                    GradientStop { position: 1; color: Theme.primaryHover }
                }

                Text {
                    text: onboardingStep < 3 ? t("next") : t("finish")
                    color: "white"
                    font.pixelSize: 14
                    font.bold: true
                    anchors.centerIn: parent
                }
                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        if (onboardingStep < 3) {
                            onboardingStep++;
                        } else {
                            rootWizard.wizardFinished()
                        }
                    }
                }
            }
        }
    }
}
