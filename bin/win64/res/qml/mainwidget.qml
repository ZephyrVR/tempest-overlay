import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.0

Rectangle {
    id: root
    color: "#0d253a"
    width: 1200
    height: 800

    property RootPage rootPage: RootPage {
        stackView: mainView
    }

    property AboutPage aboutPage: AboutPage {
        stackView: mainView
        visible: false
    }

    StackView {
        id: mainView
        anchors.fill: parent

		pushEnter: Transition {
			PropertyAnimation {
				property: "x"
				from: mainView.width
				to: 0
				duration: 200
			}
		}
		pushExit: Transition {
			PropertyAnimation {
				property: "x"
				from: 0
				to: -mainView.width
				duration: 200
			}
		}
		popEnter: Transition {
			PropertyAnimation {
				property: "x"
				from: -mainView.width
				to: 0
				duration: 200
			}
		}
		popExit: Transition {
			PropertyAnimation {
				property: "x"
				from: 0
				to: mainView.width
				duration: 200
			}
		}

        initialItem: rootPage
    }
}
