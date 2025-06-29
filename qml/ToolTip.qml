import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.fillari 1.0

Item {
    property alias text: label.text

    implicitWidth: label.width + 2 * label.x
    implicitHeight: label.height + 2 * label.y + toolTip.bottomMargin

    ToolTipItem {
        id: toolTip

        width: Math.floor(parent.width)
        height: Math.floor(parent.height)
        backgroundColor: "#ebebeb"
        borderColor: "#333333"
        borderWidth: Math.ceil(Theme.paddingSmall/10)
        bottomMargin: Theme.paddingSmall * 2
        radius: Theme.paddingSmall
    }

    Label {
        id: label

        x: Theme.paddingMedium
        color: toolTip.borderColor
        font {
            bold: true
            pixelSize: Theme.fontSizeSmall
        }
    }
}
