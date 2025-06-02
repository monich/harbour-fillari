import QtQuick 2.0
import Sailfish.Silica 1.0
import QtGraphicalEffects 1.0

MouseArea {
    property int error

    readonly property color _backgroundColor: "#ebebeb"
    readonly property color _borderColor: "#333333"
    readonly property color _textColor: "#333333"
    readonly property real _opacityFaint: 0.2
    readonly property real _opacityLow: 0.4
    readonly property int _lineThickness: Math.ceil(Theme.paddingSmall/10)

    anchors.fill: parent

    Rectangle {
        anchors.fill: parent
        color: _backgroundColor
        opacity: _opacityLow
    }

    Rectangle {
        id: shadow

        x: panel.x + Theme.paddingLarge
        y: panel.y + Theme.paddingLarge
        width: panel.width
        height: panel.height
        radius: panel.radius
        color: _borderColor
        visible: false
    }

    FastBlur {
        source: shadow
        anchors.fill: panel
        radius: Theme.paddingLarge
        transparentBorder: true
    }

    Rectangle {
        id: panel

        anchors.centerIn: parent
        radius: Theme.paddingMedium
        color: _backgroundColor
        height: content.height + 2 * Theme.paddingLarge
        width: content.width + 2 * Theme.horizontalPageMargin
        border {
            width: _lineThickness
            color: _borderColor
        }

        Row {
            id: content

            spacing: Theme.paddingLarge
            anchors.centerIn: parent

            Image {
                id: image

                anchors.verticalCenter: parent.verticalCenter
                source: "images/forbidden.svg"
                sourceSize.height: Theme.itemSizeHuge
            }

            Column {
                anchors.verticalCenter: parent.verticalCenter

                Label {
                    anchors.horizontalCenter: parent.horizontalCenter
                    font.bold: true
                    color: _textColor
                    text: "HTTP"
                }

                Label {
                    anchors.horizontalCenter: parent.horizontalCenter
                    font {
                        pixelSize: Theme.itemSizeLarge
                        bold: true
                    }
                    color: _textColor
                    text: error
                }
            }
        }
    }
}
