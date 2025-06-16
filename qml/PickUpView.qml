import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.fillari 1.0

import "harbour"

MouseArea {
    id: thisItem

    property alias nfcid: params.laNfcid1

    signal done()

    readonly property bool _active: visible && Qt.application.active
    readonly property color _bikeBackground: "#fcbc19"
    readonly property color _bikeColor: "#333333"

    NfcParam {
        id: params

        active: _active
    }

    NfcMode {
        enableModes: NfcSystem.CardEmulation
        disableModes: NfcSystem.ReaderWriter + NfcSystem.P2PInitiator + NfcSystem.P2PTarget
        active: _active
    }

    NfcTech {
        allowTechs: NfcSystem.NfcA
        disallowTechs: NfcSystem.NfcB + NfcSystem.NfcF
        active: _active
    }

    Connections {
        target: Qt.application
        onActiveChanged: if (opacity > 0 && !Qt.application.active) thisItem.done()
    }

    Rectangle {
        anchors.fill: parent
        color: Theme.highlightDimmerColor
    }

    Column {
        width: parent.width
        anchors.centerIn: parent

        spacing: Theme.paddingLarge

        Rectangle {
            anchors.horizontalCenter: parent.horizontalCenter
            width: Math.max(bike.width, bike.height) + 2 * Theme.paddingLarge
            height: width
            radius: Theme.paddingLarge
            color: _bikeBackground

            HarbourHighlightIcon {
                id: bike

                anchors.centerIn: parent
                sourceSize.width: Theme.itemSizeHuge
                source: "images/bike.svg"
                highlightColor: _bikeColor
            }
        }

        ProgressBar {
            id: progress

            width: parent.width
            indeterminate: true
            opacity: (NfcAdapter.mode == NfcSystem.CardEmulation && NfcAdapter.laNfcid1 == nfcid) ? 1 : 0
            Behavior on opacity { FadeAnimation { } }
        }

        InfoLabel {
            //: Info label
            //% "Show your phone to the card reader instead of your HSL card."
            text: qsTrId("fillari-pick_up-info_label")
            opacity: progress.opacity
        }

        Button {
            //: Button label
            //% "Done"
            text: qsTrId("fillari-pick_up-button-done")
            anchors.horizontalCenter: parent.horizontalCenter
            onClicked: thisItem.done()
        }
    }
}
