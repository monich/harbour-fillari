import QtQuick 2.0
import Sailfish.Silica 1.0

Item {
    id: thisView

    property bool isLandscape
    property alias text: label.text

    Label {
        id: label

        width: parent.width - 2 * Theme.horizontalPageMargin
        height: implicitHeight
        anchors {
            bottom: busyIndicator.top
            bottomMargin: Theme.paddingLarge
            horizontalCenter: parent.horizontalCenter
        }
        horizontalAlignment: Text.AlignHCenter
        wrapMode: Text.Wrap
        color: Theme.highlightColor
    }

    BusyIndicator {
        id: busyIndicator

        // Always center on screen (not on the page)
        y: Math.round(((isLandscape ? Screen.width : Screen.height) - height) / 2)
        anchors.horizontalCenter: parent.horizontalCenter
        size: BusyIndicatorSize.Large
        running: true
    }
}
