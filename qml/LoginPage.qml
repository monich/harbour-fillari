import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.fillari 1.0

Page {
    id: thisPage

    property var session

    Loader {
        id: loginViewLoader

        readonly property bool _active: session.sessionState === BikeSession.Unauthorized ||
                                        session.sessionState === BikeSession.LoginFailed

        anchors.fill: parent
        active: opacity > 0
        opacity: _active ? 1 : 0
        sourceComponent: Component {
            LoginView {
                isLandscape: thisPage.isLandscape
                error: session.httpError
                login: session.login
                onSignIn: session.signIn(login, password)
            }
        }
        Behavior on opacity { FadeAnimation { } }
    }

    Loader {
        id: loginNetworkErrorViewLoader

        readonly property bool _active: session.sessionState === BikeSession.LoginNetworkError

        anchors.fill: parent
        active: opacity > 0
        opacity: _active ? 1 : 0
        sourceComponent: Component {
            LoginNetworkErrorView {
                isLandscape: thisPage.isLandscape
                onRetry: session.restart()
            }
        }
        Behavior on opacity { FadeAnimation { } }
    }

    Loader {
        width: parent.width
        height: thisPage.isLandscape ? Screen.width : Screen.height
        active: opacity > 0
        opacity: loginViewLoader._active || loginNetworkErrorViewLoader._active ? 0 : 1
        sourceComponent: Component { WaitView { } }
        Behavior on opacity { FadeAnimation { } }
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

    Connections {
        target: session
        onSessionStateChanged: {
            if (session.sessionState === BikeSession.LoginFailed) {
                //: Default login error message
                //% "Login failed!"
                errorPopup.text = session.errorText ? session.errorText : qsTrId("fillari-login-failed")
                errorPopup.show(false)
            }
        }
    }

    ErrorPopup {
        id: errorPopup

        isLandscape: thisPage.isLandscape
        autoHide: false
    }
}
