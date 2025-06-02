import QtQuick 2.0
import Sailfish.Silica 1.0

Item {
    id: thisView

    property bool isLandscape

    signal retry()

    Item {
        id: content

        width: parent.width
        anchors {
            top: parent.top
            bottom: retryButton.top
        }

        InfoLabel {
            id: infoLabel

            anchors {
                left: parent.left
                leftMargin: isLandscape ? parent.width/2 : Theme.horizontalPageMargin
                right: parent.right
                rightMargin: Theme.horizontalPageMargin
                verticalCenter: parent.verticalCenter
                verticalCenterOffset: isLandscape ? 0 : Math.round(-parent.height/6)
            }
            //: Full screen error message
            //% "Sorry, cannot connect to the HSL service. Please try again later."
            text: qsTrId("fillari-login_error-message")
        }

        Item {
            id: hslGraphicsContainer

            anchors {
                bottom: parent.bottom
                left: parent.left
            }

            Image {
                id: hslGraphics

                anchors.centerIn: parent
                source: "images/hsl.svg"
                sourceSize.height: Math.min(Theme.itemSizeHuge, parent.height)
            }
        }
    }

    Button {
        id: retryButton

        anchors {
            horizontalCenter: parent.horizontalCenter
            bottom: parent.bottom
            bottomMargin: Theme.paddingLarge
        }
        //: Button for retrying to connect to the service
        //% "Retry"
        text: qsTrId("fillari-login_error-retry_button")
        onClicked: thisView.retry()
    }

    states: [
        State {
            name: "portrait"
            when: !isLandscape
            changes: [
                AnchorChanges {
                    target: hslGraphicsContainer
                    anchors {
                        top: infoLabel.bottom
                        right: content.right
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
                        top: content.top
                        right: infoLabel.left
                    }
                }
            ]
        }
    ]
}
