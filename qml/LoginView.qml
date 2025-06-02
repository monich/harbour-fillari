import QtQuick 2.0
import Sailfish.Silica 1.0

import "harbour"

Item {
    id: thisView

    property int error
    property bool isLandscape
    property alias login: loginField.text

    signal signIn(var login, var password)

    Component.onCompleted: {
        if (!error) {
            if (loginField.text === "") {
                loginField.forceActiveFocus()
            } else {
                passwordField.requestFocus()
            }
        }
    }

    Item {
        id: hslGraphicsContainer

        anchors {
            top: parent.top
            left: parent.left
        }

        Image {
            id: hslGraphics

            anchors.centerIn: parent
            source: "images/hsl.svg"
            sourceSize.height: Math.min(Theme.itemSizeHuge, parent.height)
        }
    }

    Column {
        id: inputColumn

        readonly property int maxY: Math.floor(thisView.height - passwordField.y - passwordField.height)
        readonly property int screenHeight: isLandscape ? Screen.width : Screen.height

        y: Math.min((screenHeight - height)/2, maxY)
        anchors {
            right: parent.right
            left: parent.left
            rightMargin: Theme.horizontalPageMargin
            leftMargin: isLandscape ? parent.width / 2 : Theme.horizontalPageMargin
        }

        InfoLabel {
            id: loginLabel

            //: Login info label
            //% "Log in"
            text: qsTrId("fillari-login-info_label")
            font.bold: true
            opacity: inputColumn.y >= Theme.paddingLarge ? 1 : 0
            horizontalAlignment: Text.AlignLeft
        }

        Item {
            width: 1
            height: Theme.paddingLarge
        }

        TextField {
            id: loginField

            width: parent.width
            //: Login input field label
            //% "Email or phonenumber"
            label: qsTrId("fillari-login-field-label")
            EnterKey.enabled: text !== ""
            EnterKey.onClicked: passwordField.focus = true
            inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhNoPredictiveText
        }

        HarbourPasswordInputField {
            id: passwordField

            EnterKey.enabled: text !== ""
            EnterKey.onClicked: thisView.signIn(loginField.text, passwordField.text)
        }

        Button {
            x: Theme.horizontalPageMargin
            enabled: loginField.text !== "" && passwordField.text !== "" && opacity > 0
            opacity: (inputColumn.y + inputColumn.height + Theme.paddingLarge < thisView.height) ? 1 : 0
            //: Login form button
            //% "Sign in"
            text: qsTrId("fillari-login-button-text")
            onClicked: thisView.signIn(loginField.text, passwordField.text)
        }
    }

    states: [
        State {
            name: "portrait"
            when: !isLandscape
            changes: [
                AnchorChanges {
                    target: hslGraphicsContainer
                    anchors {
                        right: thisView.right
                        bottom: inputColumn.top
                    }
                }
            ]
        },
        State {
            name: "landscape"
            when: isLandscape
            changes: [
                AnchorChanges {
                    target: hslGraphicsContainer
                    anchors {
                        right: inputColumn.left
                        bottom: thisView.bottom
                    }
                }
            ]
        }
    ]
}
