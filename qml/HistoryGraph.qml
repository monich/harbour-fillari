import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.fillari 1.0

Item {
    id: thisItem

    property var model
    property bool busy
    readonly property int mode: model.mode

    readonly property real _opacityLow: 0.4
    readonly property int _margin: Theme.paddingLarge
    readonly property int _leftGraphMargin: _margin + Theme.itemSizeLarge
    readonly property int _rightGraphMargin: _margin
    readonly property int _lineThickness: Math.ceil(Theme.paddingSmall/10)
    readonly property color _lineColor: Theme.rgba(Theme.primaryColor, _opacityLow)
    readonly property color _hslYellow: "#fcb919"
    readonly property int _maxValue: model.maxValue

    ModeSwitch {
        id: buttons

        width: parent.width
        mode: thisItem.mode
        onModeChanged: model.mode = mode
    }

    Rectangle {
        id: graphArea

        width: parent.width
        anchors {
            top: buttons.bottom
            topMargin: Theme.paddingLarge
            bottom: parent.bottom
        }
        color: "transparent"
        radius: Theme.paddingSmall
        border {
            color: _lineColor
            width: _lineThickness
        }

        Label {
            id: header

            x: _margin
            y: Theme.paddingMedium
            width: parent.width - 2 * x

            //: Graph header
            //% "Per month"
            text: qsTrId("fillari-graph-per_month_header")
            font.bold: true
        }

        BusyIndicator {
            id: busyIndicator

            y: (horizontalAxis.y - height) / 2
            anchors.horizontalCenter: parent.horizontalCenter
            size: BusyIndicatorSize.Medium
            running: busy
        }

        Repeater {
            id: grid

            readonly property int _step: Fillari.step(_maxValue, Math.min((histogramRow.height - header.height) / Theme.itemSizeSmall, 5), thisItem.mode)
            readonly property int _ystep: _maxValue ? histogramRow.height * _step / _maxValue : 0

            model:  _step ? Math.floor(_maxValue / _step) : 0

            Column {
                x: _margin
                y: horizontalAxis.y - (model.modelData + 1) * grid._ystep - height
                width: thisItem.width - 2 * x
                visible: opacity > 0
                opacity: busy ? 0 : 1

                Behavior on opacity { FadeAnimation { } }

                Label {
                    font.pixelSize: Theme.fontSizeSmall
                    color: _lineColor
                    text: Fillari.format((model.modelData + 1) * grid._step, thisItem.mode)
                    opacity: parent.y > (header.y + header.height + Theme.paddingMedium) ? 1 : 0
                }

                Rectangle {
                    width: parent.width
                    height: _lineThickness
                    color: _lineColor
                }
            }
        }

        Row {
            id: histogramRow

            readonly property int count: histogramBars.count
            readonly property int delegateWidth: count ? ((thisItem.width - x - _rightGraphMargin + spacing) / count - spacing) : 0
            readonly property int barWidth: Math.ceil(Math.min(thisItem.width / 40, delegateWidth))

            x: _leftGraphMargin
            spacing: Theme.paddingLarge
            opacity: 1 - busyIndicator.opacity
            visible: opacity > 0
            anchors {
                top: header.bottom
                topMargin: Theme.paddingMedium
                bottom: horizontalAxis.top
            }

            Repeater {
                id: histogramBars

                model: thisItem.model
                delegate: Component {
                    MouseArea {
                        width: histogramRow.delegateWidth
                        height: histogramRow.height

                        ShaderEffectSource {
                            width: histogramRow.delegateWidth
                            height: histogramRow.height + histogramRow.y
                            anchors.bottom: parent.bottom
                            visible: model.value && _maxValue > 0

                            sourceItem: Item {
                                width: histogramRow.delegateWidth
                                height: histogramRow.height + histogramRow.y

                                Item {
                                    id: cap

                                    x: bar.x
                                    y: bar.y - height/2
                                    width: bar.width
                                    height: width
                                    clip: true

                                    Rectangle {
                                        width: parent.width
                                        height: width
                                        radius: width/2
                                        color: bar.color
                                    }
                                }

                                Rectangle {
                                    id: bar

                                    color: _hslYellow
                                    width: histogramRow.barWidth
                                    height: Math.max(histogramRow.height * model.value / _maxValue - cap.height / 2, 0)
                                    anchors {
                                        horizontalCenter: parent.horizontalCenter
                                        bottom: parent.bottom
                                    }

                                    Behavior on height { SmoothedAnimation { duration: 250 } }
                                }
                            }
                        }

                        onPressedChanged: toolTips.itemAt(model.index).shouldBeVisible = pressed && model.value > 0 && _maxValue
                    }
                }
            }
        }

        Row {
            spacing: histogramRow.spacing
            opacity: histogramRow.opacity
            visible: opacity > 0
            anchors {
                left: histogramRow.left
                top: histogramRow.top
                bottom: histogramRow.bottom
            }

            Repeater {
                id: toolTips

                model: thisItem.model
                delegate: Component {
                    Item {
                        width: histogramRow.delegateWidth
                        height: histogramRow.height

                        property bool shouldBeVisible

                        onShouldBeVisibleChanged: {
                            if (shouldBeVisible) {
                                visibilityTimer.restart()
                            }
                        }

                        Timer {
                            id: visibilityTimer

                            interval: 1000
                        }

                        Loader {
                            y: parent.height * (1 - model.value / _maxValue) - Theme.paddingSmall - height
                            anchors.horizontalCenter: parent.horizontalCenter
                            opacity: parent.shouldBeVisible || visibilityTimer.running ? 1 : 0
                            active: opacity > 0

                            sourceComponent: Component {
                                ToolTip {
                                    text: Fillari.format(model.value, thisItem.mode)
                                }
                            }

                            Behavior on opacity { FadeAnimation { } }
                        }
                    }
                }
            }
        }

        Rectangle {
            id: horizontalAxis

            x: _margin
            width: parent.width - 2 * x
            height: _lineThickness
            color: _lineColor
            anchors {
                bottom: monthLabelsRow.top
                bottomMargin: Theme.paddingMedium
            }
        }

        Row {
            id: monthLabelsRow

            readonly property int count: monthLabels.count
            readonly property int labelWidth: count ? ((thisItem.width - x - _rightGraphMargin + spacing) / count - spacing) : 0

            x: _leftGraphMargin
            spacing: Theme.paddingLarge
            anchors {
                bottom: parent.bottom
                bottomMargin: Theme.paddingMedium
            }

            Repeater {
                id: monthLabels

                model: thisItem.model
                delegate: Component {
                    Label {
                        width: monthLabelsRow.labelWidth
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        minimumPixelSize: Theme.fontSizeTiny
                        fontSizeMode: Text.Fit
                        font {
                            capitalization: Font.AllUppercase
                            pixelSize: Theme.fontSizeSmall
                        }
                        color: _lineColor
                        text: model.month
                    }
                }
            }
        }
    }
}
