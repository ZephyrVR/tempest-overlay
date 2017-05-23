import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.3
import texasgamer.zephyr 1.0
import "." // QTBUG-34418, singletons require explicit import to load qmldir file

RootStackViewPage {
    id: rootPage
    width: 1200
    height: 800
    headerText: "Zephyr"
    stackView: mainView
    content: Item {
        ColumnLayout {
            anchors.fill: parent
            RowLayout {
                Layout.fillHeight: true
                Layout.fillWidth: true

                ColumnLayout {
                    Layout.preferredWidth: 250
                    Layout.maximumWidth: 250
                    Layout.fillHeight: true
                    spacing: 18
                    MyPushButton {
                        id: loginButton
                        activationSoundEnabled: false
                        text: "Login"
                        Layout.fillWidth: true
                        onClicked: {
                            MyResources.playFocusChangedSound()
                            OverlayController.promptLogin()
                            loginOpenedMessage.showMessage("Login on Desktop", "See window on desktop to login.")
                        }
                    }

                    MyPushButton {
                        id: logoutButton
                        activationSoundEnabled: false
                        text: "Logout"
                        Layout.fillWidth: true
                        onClicked: {
                            MyResources.playFocusChangedSound()
                            OverlayController.logout()
                        }
                    }

                    Item {
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                    }

                    MyPushButton {
                        id: aboutButton
                        activationSoundEnabled: false
                        text: "About"
                        Layout.fillWidth: true
                        onClicked: {
                            MyResources.playFocusChangedSound()
                            mainView.push(aboutPage)
                        }
                    }
                }

                ColumnLayout {
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    Layout.leftMargin: 80
                    anchors.top: parent.top
                    spacing: 18

                    MyText {
                        text: "Welcome!"
                        font.pointSize: 28
                    }

                    MyText {
                        id: introText
                    }

                    MyDialogOkPopup {
                        id: loginOpenedMessage
                        function showMessage(title, text) {
                            dialogTitle = title
                            dialogText = text
                            open()
                        }
                    }
                }
            }
        }
    }

Component.onCompleted: {
    updateLoggedInState()

    updateSummary()
}

Connections {
    target: OverlayController
    onLoggedInChanged : {
        console.log("logged in changed")
        updateLoggedInState()
    }
}

function updateLoggedInState() {
    var loggedIn = OverlayController.loggedIn
    loginButton.visible = !loggedIn
    logoutButton.visible = loggedIn
    introText.text = !loggedIn ? "To get started, select \"Login\" and login using your Steam account." : "Notifications will appear as they arrive."
}

function updateSummary() {
    // TODO: idk
}

Timer {
    id: summaryUpdateTimer
    repeat: true
    interval: 100
    onTriggered: {
        parent.updateSummary()
    }
}

onVisibleChanged: {
    if (visible) {
        updateSummary()
        summaryUpdateTimer.start()
    } else {
        summaryUpdateTimer.stop()
    }
}
}
