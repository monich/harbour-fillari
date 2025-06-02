import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.fillari 1.0

ApplicationWindow {
    id: appWindow

    allowedOrientations: Orientation.All

    initialPage: Component {
        MainPage {
            allowedOrientations: appWindow.allowedOrientations
            session: bikeSession
        }
    }

    cover: Component {
        CoverPage {
            session: bikeSession
        }
    }

    BikeUser {
        id: user

        userId: "0000"
    }

    BikeSession {
        id: bikeSession

        dataDir: user.dataDir
    }
}
