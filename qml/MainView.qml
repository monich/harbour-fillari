import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.fillari 1.0

Item {
    id: thisView

    property int allowedOrientations
    property bool isLandscape
    property var session

    property var _remorsePopup
    readonly property real _opacityLow: 0.4
    readonly property bool _remorsePopupVisible: _remorsePopup ? _remorsePopup.visible : false
    readonly property color _hslYellow: "#fcb919"
    readonly property bool _busy: session.sessionState === BikeSession.UserInfoQuery ||
                                  session.sessionState === BikeSession.HistoryQuery


    BikeHistoryStats {
        id: stats

        year: session.thisYear
        history: session.history
    }

    BikeHistoryModel {
        id: lastTrips

        history: stats.history
        maxCount: 3
    }

    Component {
        id: remorsePopupComponent

        RemorsePopup { }
    }

    SilicaFlickable {
        width: parent.width
        contentHeight: header.height + content.height + content.spacing
        anchors.fill: parent

        PullDownMenu {
            id: pulleyMenu

            property bool logoutRequested
            readonly property bool shouldBeVisible: !thisView._busy && !_remorsePopupVisible

            function updateVisibility() {
                if (shouldBeVisible || !active) {
                    visible = shouldBeVisible
                }
            }

            Component.onCompleted: updateVisibility()
            onShouldBeVisibleChanged: updateVisibility()
            onActiveChanged: {
                updateVisibility()
                // It doesn't look nice when remorse popup appears before the menu
                // flick animation has finished
                if (logoutRequested) {
                    logoutRequested = false
                    if (!_remorsePopup) _remorsePopup = remorsePopupComponent.createObject(thisView)
                    //: Remorse popup text
                    //% "Logging out"
                    _remorsePopup.execute(qsTrId("fillari-remorse-logout"),
                        function() { session.logOut() })
                }
            }

            MenuItem {
                //: Menu item
                //% "Log out"
                text: qsTrId("fillari-menu-log_out")
                // The remorse popup will appear when the menu has flicked back
                onClicked: pulleyMenu.logoutRequested = true
            }

            MenuItem {
                //: Menu item
                //% "Pick up"
                text: qsTrId("fillari-menu-pick_up")
                visible: session.nfcid1 !== "" && NfcSystem.valid && NfcSystem.present && NfcSystem.enabled && NfcSystem.version >= NfcSystem.Version_1_2_2
                onClicked: pickUp.opacity = 1
            }

            MenuItem {
                //: Menu item
                //% "Refresh"
                text: qsTrId("fillari-menu-refresh")
                onClicked: session.refresh()
            }
        }

        PageHeader {
            id: header

            title: session.fullName
            description: session.sessionState === BikeSession.UserInfoQuery ?
                //: Main page status text
                //% "Loading account information..."
                qsTrId("fillari-main-status-loading_user_info") :
                session.sessionState === BikeSession.HistoryQuery ?
                //: Main page status text
                //% "Loading history..."
                qsTrId("fillari-main-status-loading_history") :
                session.sessionState === BikeSession.NetworkError ?
                //: Main page status text
                //% "Network error"
                qsTrId("fillari-main-status-loading_network_error") :
                //: Main page status text
                //% "Last update %1"
                qsTrId("fillari-main-status-last_update_time").
                    arg(session.lastUpdate.toLocaleString(Qt.locale(),
                    "dd.MM.yyyy HH:mm"))

            Rectangle {
                width: fillariImage.width + 2 * Theme.paddingMedium
                height: fillariImage.height + 2 * Theme.paddingMedium
                radius: Theme.paddingSmall
                color: _hslYellow
                anchors {
                    verticalCenter: header.extraContent.verticalCenter
                    left: header.extraContent.left
                }

                Image {
                    id: fillariImage

                    anchors.centerIn: parent
                    source: "images/fillari.svg"
                    sourceSize.width: Theme.itemSizeLarge
                }
            }
        }

        Column  {
            id: content

            x: Theme.horizontalPageMargin
            width: parent.width - 2 * x
            anchors.top: header.bottom
            spacing: Theme.paddingLarge

            SectionTitle {
                //: Section header
                //% "My rides %1"
                text: qsTrId("fillari-main-section-my_rides").arg(session.thisYear)
                font.pixelSize: Theme.fontSizeLarge
            }

            HistoryGraph {
                // Make it fit nicely into the screen in landscape
                readonly property int maxHeight: thisView.height - content.y - y - 2 * content.spacing - seasonTotal.height

                model: stats
                busy: thisView._busy
                width: parent.width
                height: Math.min(width * 3 / 4, maxHeight)
            }

            Row {
                id: seasonTotal

                Label {
                    width: content.width / 2
                    anchors.verticalCenter: parent.verticalCenter
                    //: Label
                    //% "Season total"
                    text: qsTrId("fillari-main-label-season_total")
                    font.bold: true
                    verticalAlignment: Text.AlignVCenter
                }

                Label {
                    width: content.width / 2
                    anchors.verticalCenter: parent.verticalCenter
                    horizontalAlignment: Text.AlignRight
                    text: thisView._busy ? "" : Fillari.format(stats.total, stats.mode)
                    font.bold: true
                    verticalAlignment: Text.AlignVCenter
                }
            }

            Button {
                id: historyButton

                //: Button label
                //% "Ride history"
                text: qsTrId("fillari-main-button-ride_history")
                anchors.right: parent.right
                enabled: !_busy
                onClicked: pageStack.push(Qt.resolvedUrl("HistoryPage.qml"), {
                    allowedOrientations: thisView.allowedOrientations,
                    session: thisView.session})
                Component.onCompleted: {
                    // icon and layoutDirection appeared in Sailfish OS 4.0
                    if ('layoutDirection' in historyButton) {
                        icon.source = "image://theme/icon-m-right"
                        layoutDirection = Qt.RightToLeft
                    }
                }
            }

            SectionTitle {
                //: Label
                //% "Latest rides"
                text: qsTrId("fillari-main-section-latest_rides")
                visible: latestRides.visible
            }

            SilicaListView {
                id: latestRides

                visible: count > 0
                x: -content.x // Fill the entire flickable (horizontally)
                width: parent.width + 2 * content.x
                height: contentHeight
                model: lastTrips
                delegate: HistoryItem {
                    horizontalMargins: content.x
                    inProgress: model.inProgress
                    departureStation: model.departureStation
                    departureDate: model.departureDate
                    distance: inProgress ? "" : Fillari.format(model.distance, BikeHistoryStats.Distance)
                    duration: Fillari.format(model.duration, BikeHistoryStats.Duration)
                    returnStation: model.returnStation
                    returnDate: model.returnDate
                    bottomSeparator: model.index + 1 < latestRides.count
                }
            }

            Label {
                width: parent.width
                //: Label
                //% "Your pass is valid until %1"
                text: qsTrId("fillari-main-pass-valid-to").arg(session.passEndDate.toLocaleDateString(Qt.locale(), "dd.MM.yyyy"))
                font.pixelSize: Theme.fontSizeSmall
                visible: (!_busy || latestRides.count) && session.passActive
                horizontalAlignment: Text.AlignRight
                opacity: _opacityLow
            }
        }

        VerticalScrollDecorator {}
    }

    PickUpView {
        id: pickUp

        nfcid: session.nfcid1
        anchors.fill: parent
        visible: opacity > 0
        opacity: 0
        onDone: opacity = 0
        Behavior on opacity { FadeAnimation { } }
    }
}
