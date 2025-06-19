import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.fillari 1.0

Page {
    id: thisPage

    property var session

    Connections {
        target: session
        onHttpErrorChanged: {
            if (session.httpError) {
                httpErrorPanel.opacity = 1
            }
        }
    }

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: height

        Loader {
            anchors.fill: parent
            active: opacity > 0
            opacity: (session.sessionState === BikeSession.LoginCheck ||
                      session.sessionState === BikeSession.LoggingIn ||
                      session.sessionState === BikeSession.LoggingOut) ? 1 : 0
            sourceComponent: Component { WaitView { } }
            Behavior on opacity { FadeAnimation { } }
        }

        Loader {
            anchors.fill: parent
            active: opacity > 0
            opacity: (session.sessionState === BikeSession.LoginNetworkError) ? 1 : 0
            sourceComponent: Component {
                LoginNetworkErrorView {
                    isLandscape: thisPage.isLandscape
                    onRetry: session.restart()
                }
            }
            Behavior on opacity { FadeAnimation { } }
        }

        Loader {
            anchors.fill: parent
            active: opacity > 0
            opacity: (session.sessionState === BikeSession.Unauthorized ||
                      session.sessionState === BikeSession.LoginFailed) ? 1 : 0
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
            anchors.fill: parent
            active: opacity > 0
            opacity: (session.sessionState === BikeSession.UserInfoQuery ||
                      session.sessionState === BikeSession.HistoryQuery ||
                      session.sessionState === BikeSession.Ready ||
                      session.sessionState === BikeSession.NetworkError) ? 1 : 0
            sourceComponent: Component {
                MainView {
                    isLandscape: thisPage.isLandscape
                    allowedOrientations: thisPage.allowedOrientations
                    session: thisPage.session
                }
            }
            Behavior on opacity { FadeAnimation { } }
        }
    }

    HttpError {
        id: httpErrorPanel

        anchors.fill: parent

        error: session.httpError
        visible: opacity > 0
        opacity: session.httpError && (session.sessionState === BikeSession.NetworkError ||
                                       session.sessionState === BikeSession.LoginNetworkError) ? 1 : 0
        onClicked: opacity = 0
        Behavior on opacity { FadeAnimation { } }
    }
}
