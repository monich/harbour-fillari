import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.fillari 1.0

import "harbour"

CoverBackground {
    property var session

    readonly property real _opacityLow: 0.4
    readonly property color _hslYellow: "#fcb919"
    readonly property bool _darkOnLight: ('colorScheme' in Theme) && Theme.colorScheme === 1

    readonly property bool _busy: session.sessionState === BikeSession.UserInfoQuery ||
                                  session.sessionState === BikeSession.HistoryQuery
    readonly property bool _canRefresh: session.sessionState === BikeSession.Ready ||
                                        session.sessionState === BikeSession.NetworkError ||
                                        session.sessionState === BikeSession.LoginNetworkError

    BikeHistoryStats {
        id: stats

        // Make sure that the year is set first, to avoid unnecessary
        // recalculation of the statistics for the entire history
        year: session.thisYear
        history: year ? session.history : {}
        mode: BikeHistoryStats.Distance
    }

    Item {
        width: parent.width
        anchors {
            top: parent.top
            bottom: actionArea.top
        }

        Item {
            width: parent.width
            anchors {
                top: parent.top
                bottom: icon.top
            }

            Rectangle {
                width: fillariImage.width + 2 * Theme.paddingMedium
                height: fillariImage.height + 2 * Theme.paddingMedium
                radius: Theme.paddingSmall
                anchors.centerIn: parent
                color: _hslYellow

                Image {
                    id: fillariImage

                    anchors.centerIn: parent
                    source: "images/fillari.svg"
                    sourceSize.width: icon.width
                }
            }
        }

        HarbourHighlightIcon {
            id: icon

            readonly property int bottomY: Math.ceil(Math.min((parent.height + height + coverActionArea.height)/2,
                                                               parent.height - label.paintedHeight - 2 * Theme.paddingLarge))

            y: bottomY - height
            anchors.horizontalCenter: parent.horizontalCenter
            sourceSize.width: parent.width/2
            source: "images/bike.svg"
            highlightColor: _hslYellow
        }

        Label {
            id: label

            width: parent.width
            anchors {
                top: icon.bottom
                bottom: parent.bottom
            }
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
            visible: session.rideInProgress || (session.lastYear && session.thisYear === session.lastYear)
            text: session.rideInProgress ?
                Fillari.format(session.rideDuration, BikeHistoryStats.Duration) :
                Fillari.format(stats.total, stats.mode)
            color: session.rideInProgress ? Theme.highlightColor : Theme.primaryColor
            font {
                pixelSize: Theme.fontSizeLarge
                bold: true
            }
        }
    }

    BusyIndicator {
        id: busyIndicator

        anchors.centerIn: coverActionArea
        size: BusyIndicatorSize.Small
        color: Theme.primaryColor
        running: _busy
        Component.onCompleted: {
            // _forceAnimation appeared in Sailfish OS 4.0
            if ('_forceAnimation' in busyIndicator) {
                // Without this, BusyIndicator won't rotate because
                // Qt.application.active becomes false when the main
                // application window becomes invisible
                _forceAnimation = true
            }
        }
    }

    Item {
        id: actionArea

        height: (_busy || _canRefresh) ? coverActionArea.height : 0
        anchors {
            left: coverActionArea.left
            right: coverActionArea.right
            bottom: coverActionArea.bottom
        }
    }

    CoverActionList {
        enabled: _canRefresh
        CoverAction {
            iconSource: "image://theme/icon-cover-refresh"
            onTriggered: session.sessionState === BikeSession.LoginNetworkError ?
                session.restart() : session.refresh()
        }
    }
}
