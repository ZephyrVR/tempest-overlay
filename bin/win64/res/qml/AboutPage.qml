import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.3
import texasgamer.zephyr 1.0


MyStackViewPage {
    headerText: "About"

    content: ColumnLayout {
        spacing: 12

        RowLayout {
            Text {
                id: zephyr
                color: "#ffffff"
                font.pointSize: 28
                font.family: "Righteous"
                text: "Zephyr"
            }

            Text {
                id: version
                color: "#ffffff"
                font.pointSize: 12
                anchors.baseline: zephyr.baseline
            }
        }

        Text {
            color: "#ffffff"
            font.pointSize: 14
            font.bold: true
            text: "System Info"
        }

        ColumnLayout {
            Layout.leftMargin: 18

            RowLayout {
                Text {
                    color: "#ffffff"
                    font.pointSize: 12
                    text: "API:"
                }

                Text {
                    id: api
                    color: "#ffffff"
                    font.pointSize: 12
                }
            }
        }

        Text {
            color: "#ffffff"
            font.pointSize: 14
            font.bold: true
            text: "Directories"
            Layout.topMargin: 18
        }

        ColumnLayout {
            Layout.leftMargin: 18
            RowLayout {
                Text {
                    color: "#ffffff"
                    font.pointSize: 12
                    text: "Settings file:"
                }

                Text {
                    id: settingsFile
                    color: "#ffffff"
                    font.pointSize: 12
                }
            }

            RowLayout {
                Text {
                    color: "#ffffff"
                    font.pointSize: 12
                    text: "Log configuration:"
                }

                Text {
                    id: logConfig
                    color: "#ffffff"
                    font.pointSize: 12
                }
            }

            RowLayout {
                Text {
                    color: "#ffffff"
                    font.pointSize: 12
                    text: "VR runtime:"
                }

                Text {
                    id: vrRuntime
                    color: "#ffffff"
                    font.pointSize: 12
                }
            }
        }

        Text {
            color: "#ffffff"
            font.pointSize: 14
            font.bold: true
            text: "Licenses"
            Layout.topMargin: 18
        }

        ColumnLayout {
            Layout.leftMargin: 18
            RowLayout {
                Text {
                    color: "#ffffff"
                    font.pointSize: 12
                    font.family: "Righteous"
                    text: "Zephyr"
                }

                Text {
                    color: "#ffffff"
                    font.pointSize: 12
                    text: "\u00A9 2017 Thomas Gaubert. Released under GPL 3.0."
                }
            }

            Text {
                color: "#ffffff"
                font.pointSize: 12
                text: "Uses portions of OpenVR-AdvancedSettings by matzman666."
            }

            RowLayout {
                Layout.topMargin: 24
                MyPushButton {
                    Layout.preferredWidth: 300
                    text: "Zephyr on GitHub"
                    onClicked: {
                        AboutTabController.openUrl("https://github.com/ThomasGaubert/zephyr")
                        urlOpenedMessage.showMessage("External URL", "Website was opened on the desktop.")
                    }
                }

                MyPushButton {
                    Layout.preferredWidth: 500
                    text: "OpenVR-AdvancedSettings on GitHub"
                    onClicked: {
                        AboutTabController.openUrl("https://github.com/matzman666/OpenVR-AdvancedSettings")
                        urlOpenedMessage.showMessage("External URL", "Website was opened on the desktop.")
                    }
                }
            }
        }

        MyDialogOkPopup {
            id: urlOpenedMessage
            function showMessage(title, text) {
                dialogTitle = title
                dialogText = text
                open()
            }
        }

        Item {
            Layout.fillHeight: true
        }

        Component.onCompleted: {
            version.text = applicationVersion
            api.text = AboutTabController.apiVersion
            settingsFile.text = AboutTabController.settingsFile
            logConfig.text = AboutTabController.logConfiguration
            vrRuntime.text = AboutTabController.vrRuntime
        }
    }
}
