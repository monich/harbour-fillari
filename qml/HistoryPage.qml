import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.fillari 1.0

Page {
    id: thisPage

    property var session

    readonly property var _years: session.years

    BikeHistoryStats {
        id: stats

        // Make sure that the year is set first, to avoid unnecessary
        // recalculation of the statistics for the entire history
        year: _years.length > 0 ? _years[_years.length - 1] : 0
        history: year ? session.history : undefined
    }

    SilicaListView {
        id: list

        anchors.fill: parent
        model: BikeHistoryModel {
            year: stats.year
            history: stats.history
        }

        header: Component {
            Column {
                id: headerColumn

                property real lastHeight: height

                width: list.width

                // For whatever reason the initial height of the header
                // is smaller (ComboBox height is zero) and then grows.
                // The intention here is to fix the initial list view
                // scroll position.
                onHeightChanged: {
                    if (list.contentY === -lastHeight) {
                        list.contentY = -height
                    }
                    lastHeight = height
                }

                PageHeader {
                    id: header

                    //: Page title
                    //% "Ride history"
                    title: qsTrId("fillari-history-header")
                }

                ComboBox {
                    id: yearSelector

                    //: Combo box label
                    //% "Year"
                    label: qsTrId("fillari-history-year-label")
                    menu: ContextMenu {
                        Repeater {
                            model: _years.length
                            MenuItem {
                                text: _years[modelData]
                            }
                        }
                    }

                    onCurrentIndexChanged: stats.year = _years[currentIndex]
                    Component.onCompleted: {
                        if (_years.length > 0) {
                            yearSelector.currentIndex = _years.length - 1
                        }
                    }
                }

                HistoryGraph {
                    id: graph

                    // Make it fit nicely into the screen in landscape
                    readonly property int maxHeight: thisPage.height - header.height - yearSelector.height - 2 * headerColumn .spacing - Theme.paddingLarge

                    x: Theme.horizontalPageMargin
                    width: parent.width - 2 * x
                    model: stats
                    height: Math.min(width * 3 / 4, maxHeight)
                }

                DummyItem { height: Theme.paddingLarge }

                Row {
                    x: graph.x

                    Label {
                        width: graph.width / 2
                        anchors.verticalCenter: parent.verticalCenter
                        //: Label
                        //% "Season total"
                        text: qsTrId("fillari-main-label-season_total")
                        font.bold: true
                        verticalAlignment: Text.AlignVCenter
                    }

                    Label {
                        width: graph.width / 2
                        anchors.verticalCenter: parent.verticalCenter
                        horizontalAlignment: Text.AlignRight
                        text: Fillari.format(stats.total, stats.mode)
                        font.bold: true
                        verticalAlignment: Text.AlignVCenter
                    }
                }

                DummyItem { height: Theme.paddingLarge }
            }
        }

        section {
            property: "month"
            criteria: ViewSection.FullString
            delegate: Component {
                HistorySection {
                    month: list.model.monthName(section)
                    detail1: stats.mode ===  BikeHistoryStats.Distance ? stats.formatMonthTotal(section, BikeHistoryStats.Distance) :
                             stats.mode ===  BikeHistoryStats.Rides ? stats.formatMonthTotal(section, BikeHistoryStats.Rides) : ""
                    detail2: stats.mode ===  BikeHistoryStats.Duration ? stats.formatMonthTotal(section, BikeHistoryStats.Duration) : ""
                    leftPadding: Theme.horizontalPageMargin
                    rightPadding: Theme.horizontalPageMargin
                    color: Theme.highlightColor
                }
            }
        }

        delegate: Component {
            HistoryItem {
                horizontalMargins: Theme.horizontalPageMargin
                inProgress: model.inProgress
                departureStation: model.departureStation
                departureDate: model.departureDate
                distance: inProgress ? "" : Fillari.format(model.distance, BikeHistoryStats.Distance)
                duration: Fillari.format(model.duration, BikeHistoryStats.Duration)
                returnStation: model.returnStation
                returnDate: model.returnDate
                bottomSeparator: model.index + 1 < list.count
            }
        }

        VerticalScrollDecorator { }
    }
}
