import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.fillari 1.0

Rectangle {
    id: thisItem

    property int mode

    readonly property int  _minimumPressHighlightTime: ('minimumPressHighlightTime' in Theme) ? Theme.minimumPressHighlightTime : 64
    readonly property real _opacityFaint: ('opacityFaint' in Theme) ? Theme.opacityFaint : 0.2
    readonly property real _opacityLow: ('opacityLow' in Theme) ? Theme.opacityLow : 0.4
    readonly property var _modes: [BikeHistoryStats.Distance, BikeHistoryStats.Rides, BikeHistoryStats.Duration]
    readonly property var _labels: [
        //: Button label
        //% "Kilometers"
        qsTrId("fillari-history-distance_button"),
        //: Button label
        //% "Journeys"
        qsTrId("fillari-history-rides_button"),
        //: Button label
        //% "Time"
        qsTrId("fillari-history-duration_button")
    ]

    width: parent.width
    height: Theme.itemSizeExtraSmall
    radius: Theme.paddingSmall
    color: "transparent"
    border {
        color: Theme.rgba(Theme.primaryColor, _opacityLow)
        width: Math.ceil(Theme.paddingSmall/10)
    }

    Row {
        id: buttonsRow

        anchors.fill: parent

        Repeater {
            id: buttons
            model: _modes.length
            delegate: MouseArea {
                id: button

                readonly property bool selected: thisItem.mode === _modes[model.index]
                readonly property bool down: pressed && containsMouse
                readonly property bool showPress: down || pressTimer.running

                height: buttonsRow.height
                width: buttonsRow.width / buttons.count
                onPressedChanged: {
                    if (pressed) {
                        pressTimer.start()
                    }
                }
                onCanceled: pressTimer.stop()
                onClicked: thisItem.mode = _modes[model.index]

                Rectangle {
                    anchors {
                        fill: parent
                        margins: thisItem.border.width
                    }
                    radius: Theme.paddingSmall
                    color: selected ? Theme.rgba(Theme.primaryColor, _opacityFaint) : "transparent"

                    Label {
                        anchors.centerIn: parent
                        color: showPress ? Theme.highlightColor : Theme.primaryColor
                        text: _labels[model.index]
                        font.bold: selected
                    }
                }

                Timer {
                    id: pressTimer

                    interval: _minimumPressHighlightTime
                }
            }
        }
    }
}
