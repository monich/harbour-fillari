import QtQuick 2.0
import Sailfish.Silica 1.0

Item {
    property alias text: label.text

    readonly property real _sqrt2: Math.sqrt(2)
    readonly property int _lineThickness: Math.ceil(Theme.paddingSmall/10)
    readonly property color _backgroundColor: "#ebebeb"
    readonly property color _foregroundColor: "#333333"

    width: label.width + 2 * Theme.paddingMedium
    height: label.height + 2 * Theme.paddingMedium

    Rectangle {
        id: contents

        y: Theme.paddingSmall
        width: parent.width
        height: parent.height - Theme.paddingLarge
        color: _backgroundColor
        radius: Theme.paddingSmall
        border {
            width: _lineThickness
            color: _foregroundColor
        }
    }

    Item {
        id : triangle

        anchors {
            horizontalCenter: parent.horizontalCenter
            top: contents.bottom
            topMargin: - _lineThickness
        }

        width: Theme.paddingLarge
        height: width
        clip: true

        Rectangle {
            width: parent.width / _sqrt2
            height: width
            color: _backgroundColor
            border {
                width: _lineThickness
                color: _foregroundColor
            }
            transform: [
                Rotation {
                    axis.z: 1
                    angle: 45
                },
                Translate {
                    x: triangle.width / 2
                    y: - triangle.height /2
                }
            ]
            antialiasing: true
        }
    }

    Item {
        anchors.fill: triangle

        // Repair clipping/antialiasing artifacts
        Rectangle {
            x: _lineThickness
            y: _lineThickness
            width: (parent.width - 2 * _lineThickness) / _sqrt2
            height: width
            color: _backgroundColor
            transform: [
                Rotation {
                    axis.z: 1
                    angle: 45
                },
                Translate {
                    x: triangle.width / 2
                    y: - triangle.height /2
                }
            ]
        }
    }

    Label {
        id: label

        color: _foregroundColor
        anchors.centerIn: contents
        font {
            bold: true
            pixelSize: Theme.fontSizeSmall
        }
    }
}
