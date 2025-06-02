import QtQuick 2.0
import Sailfish.Silica 1.0

import "harbour"

ListItem {
    property alias departureStation: departureStationLabel.text
    property date departureDate
    property alias distance: distanceLabel.text
    property alias duration: durationLabel.text
    property alias returnStation: returnStationLabel.text
    property date returnDate
    property alias bottomSeparator: bottom.visible
    property alias horizontalMargins: content.x
    property bool inProgress

    contentHeight: content.y + content.height

    readonly property real _opacityLow: 0.4
    readonly property color _separatorColor: Theme.rgba(Theme.primaryColor, _opacityLow)
    readonly property int _firstColumnWidth: departureIcon.width
    readonly property int _lastColumnWidth: Math.max(durationLabel.implicitWidth, distanceItem.implicitWidth, 1)
    readonly property int _centerColumnWidth: content.width - _firstColumnWidth - _lastColumnWidth - 2 * grid.columnSpacing

    function dateToString(date) {
        return date.toLocaleString(Qt.locale(), "dd.MM.yyyy hh:mm")
    }

    Column {
        id: content

        width: parent.width - 2 * x

        DummyItem { height: Theme.paddingMedium }

        Grid {
            id: grid

            columns: 3
            columnSpacing: Theme.paddingSmall

            Image {
                id: departureIcon

                source: "images/start.svg"
                sourceSize.height: departureStationLabel.height
            }

            Label {
                id: departureStationLabel

                width: _centerColumnWidth
                verticalAlignment: Text.AlignTop
                horizontalAlignment: Text.AlignLeft
                font.bold: true
            }

            Item {
                id: distanceItem

                implicitWidth: inProgress ? rideIcon.implicitWidth : distanceLabel.implicitWidth
                width: _lastColumnWidth
                height: distanceLabel.height

                Loader {
                    id: rideIcon

                    active: inProgress
                    anchors.right: parent.right
                    sourceComponent: Component {
                        HarbourHighlightIcon {
                            sourceSize.height: distanceLabel.height
                            source: "images/bike.svg"
                            highlightColor: Theme.primaryColor
                        }
                    }
                }

                Label {
                    id: distanceLabel

                    width: _lastColumnWidth
                    horizontalAlignment: Text.AlignRight
                    verticalAlignment: Text.AlignTop
                    font.bold: true
                    visible: !inProgress
                }
            }

            //===================================================================

            Item {
                width: _firstColumnWidth
                height: departureDateLabel.height

                Rectangle {
                    y: Theme.paddingSmall
                    width: Theme.paddingSmall
                    height: parent.height - 2 * y
                    radius: width/2
                    anchors.centerIn: parent
                    color: Theme.rgba(Theme.primaryColor, Theme.opacityLow)
                    opacity: inProgress ? 0 : 1
                }
            }

            Label {
                id: departureDateLabel

                width: _centerColumnWidth
                horizontalAlignment: Text.AlignLeft
                text: dateToString(departureDate)
            }

            Label {
                id: durationLabel

                width: _lastColumnWidth
                horizontalAlignment: Text.AlignRight
                color: inProgress ? Theme.highlightColor : Theme.primaryColor
            }

            //===================================================================

            Image {
                source: "images/finish.svg"
                sourceSize.height: departureIcon.sourceSize.height
                visible: !inProgress
            }

            Label {
                id: returnStationLabel

                width: _centerColumnWidth
                verticalAlignment: Text.AlignTop
                horizontalAlignment: Text.AlignLeft
                font.bold: true
                visible: !inProgress
            }

            DummyItem {
                visible: !inProgress
            }

            //===================================================================

            DummyItem {
                visible: !inProgress
            }

            Label {
                width: _centerColumnWidth
                text: isNaN(returnDate) ? "" : dateToString(returnDate)
                visible: !inProgress
            }

            Item {
                width: 1
                height: Theme.paddingMedium
                visible: !inProgress
            }
        }

        DummyItem { height: Theme.paddingMedium }

        Separator {
            id: bottom

            x: _firstColumnWidth + grid.columnSpacing
            width: parent.width - x
            color: _separatorColor
        }
    }
}
