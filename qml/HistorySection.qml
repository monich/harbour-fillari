import QtQuick 2.0
import Sailfish.Silica 1.0

Item {
    id: thisItem

    property alias month: monthLabel.text
    property alias detail1: label1.text
    property string detail2
    property color color: Theme.highlightColor
    property int leftPadding
    property int rightPadding

    width: parent ? parent.width : Screen.width
    implicitHeight: Math.max(monthLabel.height, infoBox.height)

    Label {
        id: monthLabel

        anchors {
            left: parent.left
            leftMargin: thisItem.leftPadding
            verticalCenter: parent.verticalCenter
        }
        width: parent.width - infoBox.width
        topPadding: Theme.paddingMedium
        bottomPadding: Theme.paddingMedium
        font.capitalization: Font.Capitalize
        color: thisItem.color
    }

    Row {
        id: infoBox

        anchors {
            right: parent.right
            rightMargin: thisItem.rightPadding
            verticalCenter: parent.verticalCenter
        }

        Label {
            id: label1

            anchors.verticalCenter: parent.verticalCenter
            color: thisItem.color
            font.bold: true
        }

        Label {
            id: label2

            anchors.verticalCenter: parent.verticalCenter
            color: thisItem.color
            text: detail1 === "" ? detail2 :
                  detail2 !== "" ? " (" + detail2 + ")" :  ""
        }
    }
}
