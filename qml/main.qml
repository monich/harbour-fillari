import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.fillari 1.0

ApplicationWindow {
    id: appWindow

    allowedOrientations: Orientation.Portrait | Orientation.LandscapeMask
    initialPage: loginPageComponent

    cover: Component {
        CoverPage {
            session: bikeSession
        }
    }

    Component {
        id: loginPageComponent

        LoginPage {
            allowedOrientations: appWindow.allowedOrientations
            session: bikeSession
        }
    }

    Component {
        id: mainPageComponent

        MainPage {
            allowedOrientations: appWindow.allowedOrientations
            session: bikeSession
        }
    }

    BikeUser {
        id: user

        userId: "0000"
    }

    BikeSession {
        id: bikeSession

        property var _lastPage: user.hasFile("Cookies") ? mainPageComponent : loginPageComponent

        dataDir: user.dataDir
        onSessionStateChanged: {
            var page = _lastPage

            switch (sessionState) {
            case BikeSession.LoginCheck:
                // Don't replace the page
                break;
            case BikeSession.UserInfoQuery:
            case BikeSession.HistoryQuery:
            case BikeSession.Ready:
            case BikeSession.NetworkError:
                page = mainPageComponent
                break
            default:
                page = loginPageComponent
                break
            }

            if (!pageStack.currentPage) {
                initialPage = _lastPage = page
            } else if (_lastPage !== page) {
                _lastPage = page
                pageStack.replaceAbove(null, page, {}, PageStackAction.Animated)
            }
        }
    }
}
