import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.fillari 1.0

Page {
    id: thisPage

    property var session

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: height

        Loader {
            id: mainViewLoader

            readonly property bool _active: session.sessionState === BikeSession.UserInfoQuery ||
                                            session.sessionState === BikeSession.HistoryQuery ||
                                            session.sessionState === BikeSession.Ready ||
                                            session.sessionState === BikeSession.NetworkError ||
                                            session.sessionState === BikeSession.LoggingOut

            anchors.fill: parent
            active: opacity > 0
            opacity: _active ? 1 : 0
            sourceComponent: Component {
                MainView {
                    isLandscape: thisPage.isLandscape
                    allowedOrientations: thisPage.allowedOrientations
                    session: thisPage.session
                }
            }
            Behavior on opacity { FadeAnimation { } }
        }

        Loader {
            width: parent.width
            height: thisPage.isLandscape ? Screen.width : Screen.height
            active: opacity > 0
            opacity: mainViewLoader._active ? 0 : 1
            sourceComponent: Component { WaitView { } }
            Behavior on opacity { FadeAnimation { } }
        }
    }

    HttpError {
        id: httpErrorPanel

        anchors.fill: parent

        error: session.httpError
        visible: opacity > 0
        opacity: 0

        onClicked: opacity = 0
        onErrorChanged: {
            if (error) {
                opacity = 1
            }
        }
        Behavior on opacity { FadeAnimation { } }
    }
}
