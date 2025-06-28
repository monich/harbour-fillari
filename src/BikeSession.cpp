/*
 * Copyright (C) 2025 Slava Monich <slava@monich.com>
 *
 * You may use this file under the terms of the BSD license as follows:
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer
 *     in the documentation and/or other materials provided with the
 *     distribution.
 *
 *  3. Neither the names of the copyright holders nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation
 * are those of the authors and should not be interpreted as representing
 * any official policies, either expressed or implied.
 */

#include "BikeSession.h"

#include "BikeHistoryModel.h"
#include "BikeHistoryQuery.h"
#include "BikeLogin.h"
#include "BikeLogout.h"
#include "BikeObjectQuery.h"

#include <QtCore/QDate>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QJsonObject>
#include <QtCore/QListIterator>
#include <QtCore/QScopedPointer>
#include <QtCore/QTextStream>
#include <QtCore/QTimer>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkCookieJar>
#include <QtNetwork/QNetworkCookie>

#include "HarbourDebug.h"

// s(SignalName,signalName)
#define QUEUED_SIGNALS(s) \
    s(DataDir,dataDir) \
    s(Login,login) \
    s(ErrorText,errorText) \
    s(HttpError,httpError) \
    s(SessionState,sessionState) \
    s(LastUpdate,lastUpdate) \
    s(LastNetworkError,lastNetworkError) \
    s(FirstNames,firstNames) \
    s(LastName,lastName) \
    s(FullName,fullName) \
    s(HslCard,hslCard) \
    s(Nfcid1,nfcid1) \
    s(History,history) \
    s(RideInProgress,rideInProgress) \
    s(RideDuration,rideDuration) \
    s(Years,years) \
    s(LastYear,lastYear) \
    s(ThisYear,thisYear)

// ==========================================================================
// BikeSession::CookieJar
// Unprotects setAllCookies(const QList<QNetworkCookie>&) and
// QNetworkCookieJar::allCookies()
// ==========================================================================

class BikeSession::CookieJar :
    public QNetworkCookieJar
{
    Q_OBJECT
    friend class BikeSession::Private;

public:
    CookieJar(QObject* aParent) : QNetworkCookieJar(aParent) {}
};

// ==========================================================================
// BikeSession::Private
// ==========================================================================

class BikeSession::Private :
    public QObject
{
    Q_OBJECT

    typedef void (BikeSession::*SignalEmitter)();
    typedef uint SignalMask;
    enum Signal {
        #define SIGNAL_ENUM_(Name,name) Signal##Name##Changed,
        QUEUED_SIGNALS(SIGNAL_ENUM_)
        #undef  SIGNAL_ENUM_
        SignalCount
    };

    static const QString COOKIES_FILE;
    static const QString LOGIN_FILE;

    #if HARBOUR_DEBUG
    static const char* sessionName(State);
    #endif

public:
    Private(BikeSession*);

    static int last(const QList<int>&);

    BikeSession* parentObject();
    void queueSignal(Signal);
    void emitQueuedSignals();

    void setDataDir(QString);
    void setLogin(QString);
    void setErrorText(QString);
    void setHttpStatus(int);
    void setState(State);
    void setFirstName(QString);
    void setLastName(QString);
    void setIdent(QString, QString);
    bool rideInProgress() const;
    int rideDuration() const;
    void start();
    void signIn(QString, QString);
    void logOut();
    void refreshHistory();
    void updated();

    void saveCookies(CookieJar*) const;
    void saveCookies() const;
    QNetworkCookieJar* loadCookies();
    QString loadTextFile(const QString&);
    void saveTextFile(const QString&, const QString&);
    void userInfoReceived(const QJsonObject&);
    void startRequest(BikeRequest*, State, const char*, const char*);
    void startObjectQuery(BikeObjectQuery*, State, const char*,
        const char* aHttpErrorSlot = SLOT(onHttpError(int)),
        const char* aNetworkErrorSlot = SLOT(onNetworkError()));

private Q_SLOTS:
    void onUserQueryFinished(const QJsonObject&);
    void onServiceQueryFinished(const QJsonObject&);
    void onHistoryQueryFinished(const QJsonArray&);
    void onLoginSuccess(const QJsonObject&);
    void onLoginFailure(QString);
    void onLoginNetworkError();
    void onLoginHttpError(int);
    void onLogoutDone();
    void onNetworkError();
    void onHttpError(int);

public:
    SignalMask iQueuedSignals;
    Signal iFirstQueuedSignal;
    QNetworkAccessManager iNetworkAccessManager;
    BikeRequest::Ptr iRequest;
    QString iDataDir;
    int iHttpError;
    State iState;
    QString iLogin;
    QString iErrorText;
    QDateTime iLastUpdate;
    QDateTime iLastNetworkError;
    QString iFirstNames;
    QString iLastName;
    QString iHslCard;
    QString iNfcid1;
    QJsonArray iHistory;
    QTimer* iRideDurationTimer;
    QList<int> iYears;
    int iThisYear;
};

const QString BikeSession::Private::COOKIES_FILE("Cookies");
const QString BikeSession::Private::LOGIN_FILE("Login");

BikeSession::Private::Private(
    BikeSession* aParent) :
    QObject(aParent),
    iQueuedSignals(0),
    iFirstQueuedSignal(SignalCount),
    iHttpError(0),
    iState(None),
    iRideDurationTimer(Q_NULLPTR),
    iThisYear(QDate::currentDate().year())
{}

// static
inline
int
BikeSession::Private::last(
    const QList<int>& aList)
{
    return aList.isEmpty() ? 0 : aList.last();
}

inline
BikeSession*
BikeSession::Private::parentObject()
{
    return qobject_cast<BikeSession*>(parent());
}

void
BikeSession::Private::queueSignal(
    Signal aSignal)
{
    if (aSignal >= 0 && aSignal < SignalCount) {
        const SignalMask signalBit = (SignalMask(1) << aSignal);

        if (iQueuedSignals) {
            iQueuedSignals |= signalBit;
            if (iFirstQueuedSignal > aSignal) {
                iFirstQueuedSignal = aSignal;
            }
        } else {
            iQueuedSignals = signalBit;
            iFirstQueuedSignal = aSignal;
        }
    }
}

void
BikeSession::Private::emitQueuedSignals()
{
    static const SignalEmitter emitSignal [] = {
        #define SIGNAL_EMITTER_(Name,name) &BikeSession::name##Changed,
        QUEUED_SIGNALS(SIGNAL_EMITTER_)
        #undef SIGNAL_EMITTER_
    };

    if (iQueuedSignals) {
        uint i = iFirstQueuedSignal;
        BikeSession* obj = parentObject();

        // Reset first queued signal before emitting the signals.
        // Signal handlers may emit new signals.
        iFirstQueuedSignal = SignalCount;
        for (; i < SignalCount && iQueuedSignals; i++) {
            const SignalMask signalBit = (SignalMask(1) << i);
            if (iQueuedSignals & signalBit) {
                iQueuedSignals &= ~signalBit;
                Q_EMIT (obj->*(emitSignal[i]))();
            }
        }
    }
}

#if HARBOUR_DEBUG
//static
const char*
BikeSession::Private::sessionName(
    State aState)
{
    switch (aState) {
    case None: return "None";
    case LoginCheck: return "LoginCheck";
    case UserInfoQuery: return "UserInfoQuery";
    case HistoryQuery: return "HistoryQuery";
    case Unauthorized: return "Unauthorized";
    case LoggingIn: return "LoggingIn";
    case LoggingOut: return "LoggingOut";
    case LoginFailed: return "LoginFailed";
    case LoginNetworkError: return "LoginNetworkError";
    case NetworkError: return "NetworkError";
    case Ready: return "Ready";
    }
    return "?";
}
#endif

void
BikeSession::Private::updated()
{
    iLastUpdate = QDateTime::currentDateTime();
    queueSignal(SignalLastUpdateChanged);
}

void
BikeSession::Private::setState(
    State aState)
{
    if (iState != aState) {
        HDEBUG(sessionName(iState) << "=>" << sessionName(aState));
        iState = aState;
        queueSignal(SignalSessionStateChanged);
    }
}

void
BikeSession::Private::setLogin(
    QString aLogin)
{
    if (iLogin != aLogin) {
        iLogin = aLogin;
        HDEBUG(iLogin);
        queueSignal(SignalLoginChanged);
    }
}

void
BikeSession::Private::setErrorText(
    QString aErrorText)
{
    if (iErrorText != aErrorText) {
        iErrorText = aErrorText;
        queueSignal(SignalErrorTextChanged);
    }
}

void
BikeSession::Private::setHttpStatus(
    int aStatus)
{
    int httpError = (aStatus == BikeRequest::OK) ? 0 : aStatus;

    if (iHttpError != httpError) {
        iHttpError = httpError;
#if HARBOUR_DEBUG
        if (httpError) {
            HDEBUG("HTTP error" << httpError);
        }
#endif
        queueSignal(SignalHttpErrorChanged);
    }
}

void
BikeSession::Private::setFirstName(
    QString aFirstNames)
{
    if (iFirstNames != aFirstNames) {
        iFirstNames = aFirstNames;
        HDEBUG(iFirstNames);
        queueSignal(SignalFirstNamesChanged);
        queueSignal(SignalFullNameChanged);
    }
}

void
BikeSession::Private::setLastName(
    QString aLastName)
{
    if (iLastName != aLastName) {
        iLastName = aLastName;
        HDEBUG(iLastName);
        queueSignal(SignalLastNameChanged);
        queueSignal(SignalFullNameChanged);
    }
}

void
BikeSession::Private::setIdent(
    QString aIdentType,
    QString aIdentData)
{
    QString hslCard, nfcid1;

    if (aIdentType == QStringLiteral("card") &&
        aIdentData.length() == 16 &&
        aIdentData.at(0) == '0' &&
        aIdentData.at(1) == '0') {
        hslCard = aIdentData;
        nfcid1 = hslCard.right(14).toLower();
    }

    if (iHslCard != hslCard) {
        iHslCard = hslCard;
        HDEBUG("card" << hslCard);
        queueSignal(SignalHslCardChanged);
    }

    if (iNfcid1 != nfcid1) {
        iNfcid1 = nfcid1;
        HDEBUG("NFCID1" << qPrintable(nfcid1));
        queueSignal(SignalNfcid1Changed);
    }
}

bool
BikeSession::Private::rideInProgress() const
{
    return !iHistory.isEmpty() &&
        BikeHistoryModel::rideInProgress(iHistory.first().toObject());
}

int
BikeSession::Private::rideDuration() const
{
    if (!iHistory.isEmpty()) {
        const QJsonObject ride(iHistory.first().toObject());

        if (BikeHistoryModel::rideInProgress(ride)) {
            const QDateTime departureDate(QDateTime::fromString(ride.
                value(QStringLiteral("departureDate")).toString(),
                Qt::ISODate));

            if (departureDate.isValid()) {
                const QDateTime now(QDateTime::currentDateTime());
                const qint64 secs = departureDate.secsTo(now);

                if (secs > 0) {
                    HDEBUG(secs);
                    return int(secs);
                }
            }
        }
    }
    return 0;
}

void
BikeSession::Private::start()
{
    // Query user information
    startObjectQuery(new BikeUserQuery(&iNetworkAccessManager), LoginCheck,
        SLOT(onUserQueryFinished(QJsonObject)),
        SLOT(onLoginHttpError(int)),
        SLOT(onLoginNetworkError()));
}

void
BikeSession::Private::setDataDir(
    QString aDataDir)
{
    if (iDataDir != aDataDir) {
        iDataDir = aDataDir;
        HDEBUG(iDataDir);
        queueSignal(SignalDataDirChanged);
        setErrorText(QString());
        setFirstName(QString());
        setLastName(QString());
        saveCookies();
        iNetworkAccessManager.setCookieJar(loadCookies());
        setLogin(loadTextFile(LOGIN_FILE));
        if (iDataDir.isEmpty()) {
            setState(None);
        } else {
            start();
        }
    }
}

void
BikeSession::Private::signIn(
    QString aLogin,
    QString aPassword)
{
    HDEBUG("Signing in as" << aLogin);
    saveTextFile(LOGIN_FILE, aLogin);
    setLogin(aLogin);

    BikeLogin* login = new BikeLogin(&iNetworkAccessManager, aLogin, aPassword);

    connect(login, SIGNAL(failure(QString)), SLOT(onLoginFailure(QString)));
    connect(login, SIGNAL(success(QJsonObject)), SLOT(onLoginSuccess(QJsonObject)));
    startRequest(login, LoggingIn,
        SLOT(onLoginHttpError(int)),
        SLOT(onLoginNetworkError()));
}

void
BikeSession::Private::logOut()
{
    HDEBUG("Logging out");

    BikeLogout* logout = new BikeLogout(&iNetworkAccessManager);

    connect(logout, SIGNAL(finished()), SLOT(onLogoutDone()));
    iRequest.reset(logout);
    setState(LoggingOut);
    setHttpStatus(0);
}

void
BikeSession::Private::saveCookies() const
{
    saveCookies(qobject_cast<CookieJar*>(iNetworkAccessManager.cookieJar()));
}

void
BikeSession::Private::saveCookies(
    CookieJar* aJar) const
{
    if (aJar && !iDataDir.isEmpty()) {
        QDir dir(iDataDir);

        if (dir.mkpath(".")) {
            QFile file(dir.filePath(COOKIES_FILE));

            if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                QListIterator<QNetworkCookie> it(aJar->allCookies());
                QTextStream out(&file);

                while (it.hasNext()) {
                    out << QString::fromLatin1(it.next().toRawForm())
                        << QString("\n");
                }
            }
        }
    }
}

QNetworkCookieJar*
BikeSession::Private::loadCookies()
{
    CookieJar* jar = new CookieJar(&iNetworkAccessManager);

    if (!iDataDir.isEmpty()) {
        QDir dir(iDataDir);
        QFile file(dir.filePath(COOKIES_FILE));

        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QList<QNetworkCookie> cookies;
            QTextStream in(&file);

            while (!in.atEnd()) {
                const QByteArray line(in.readLine().toLatin1());
                cookies.append(QNetworkCookie::parseCookies(line));
            }
            HDEBUG("Loaded" << cookies.count() << qPrintable(file.fileName()));
            jar->setAllCookies(cookies);
        }
    }
    return jar;
}

QString
BikeSession::Private::loadTextFile(
    const QString& aFileName)
{
    // Load a single line of text from a file
    if (!iDataDir.isEmpty()) {
        QDir dir(iDataDir);
        QFile file(dir.filePath(aFileName));

        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);

            if (!in.atEnd()) {
                QString text(in.readLine());
                if (!text.isEmpty()) {
                    HDEBUG(aFileName << "=" << text);
                    return text;
                }
            }
        }
    }
    return QString();
}

void
BikeSession::Private::saveTextFile(
    const QString& aFileName,
    const QString& aText)
{
    if (!iDataDir.isEmpty()) {
        QDir dir(iDataDir);
        QFile file(dir.filePath(aFileName));

        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream(&file) << aText;
        }
    }
}

void
BikeSession::Private::startObjectQuery(
    BikeObjectQuery* aQuery,
    State aState,
    const char* aFinishSlot,
    const char* aHttpErrorSlot,
    const char* aNetworkErrorSlot)
{
    connect(aQuery, SIGNAL(finished(QJsonObject)), aFinishSlot);
    startRequest(aQuery, aState, aHttpErrorSlot, aNetworkErrorSlot);
}

void
BikeSession::Private::startRequest(
    BikeRequest* aRequest,
    State aState,
    const char* aHttpErrorSlot,
    const char* aNetworkErrorSlot)
{
    connect(aRequest, SIGNAL(httpError(int)), aHttpErrorSlot);
    connect(aRequest, SIGNAL(networkError()), aNetworkErrorSlot);
    iRequest.reset(aRequest);
    setState(aState);
    setHttpStatus(0);
}

void
BikeSession::Private::userInfoReceived(
    const QJsonObject& aUserInfo)
{
    const QJsonObject user(aUserInfo.value("user").toObject());
    const QJsonObject userName(user.value("name").toObject());
    QString firstNames(userName.value("givenName").toString());
    QString lastName(userName.value("familyName").toString());

    if (firstNames.isEmpty() && lastName.isEmpty()) {
        // Try legal information if the user-friendly name is missing
        const QJsonObject legal(user.value("legalInformation").toObject());

        firstNames = legal.value("firstNames").toString();
        lastName = legal.value("lastName").toString();
    }

    setFirstName(firstNames);
    setLastName(lastName);
    updated();

    // Query service info
    startObjectQuery(new BikeServiceQuery(&iNetworkAccessManager),
        UserInfoQuery, SLOT(onServiceQueryFinished(QJsonObject)));
}

void
BikeSession::Private::refreshHistory()
{
    // Query the history
    BikeHistoryQuery* query = new BikeHistoryQuery(&iNetworkAccessManager);

    connect(query, SIGNAL(finished(QJsonArray)),
        SLOT(onHistoryQueryFinished(QJsonArray)));
    startRequest(query, HistoryQuery,
        SLOT(onHttpError(int)),
        SLOT(onNetworkError()));
}

void
BikeSession::Private::onHistoryQueryFinished(
    const QJsonArray& aHistory)
{
    const bool wasInProgress = rideInProgress();
    QList<int> years;

    HDEBUG("Loaded" << aHistory.size() << "trips");
    iRequest.reset();
    iHistory = aHistory;
    queueSignal(SignalHistoryChanged);

#if 0
    // Simulation of a ride in progress
    QJsonArray history(aHistory);
    if (!history.isEmpty()) {
        HDEBUG("Simulating ride in progress");
        QJsonObject entry(history.first().toObject());
        entry.insert("distance", 0);
        entry.insert("duration", 0);
        entry.insert("departureDate", QDateTime::currentDateTime().addSecs(-10).toString(Qt::ISODate));
        entry.insert("returnDate", QJsonValue());
        entry.insert("returnStation", "");
        history.replace(0, entry);
    }
    iHistory = history;
#elif 0
    // Simulation of history change
    QJsonArray history(aHistory);
    if (!history.isEmpty()) {
        static int count = 1;
        HDEBUG("Adding" << count << "trip(s)");
        for (int i = 0; i < count; i++) {
            history.prepend(history.first());
        }
        count++;
    }
    iHistory = history;
#endif

    // Update the years
    const uint n = aHistory.size();
    for (uint i = 0; i < n; i++) {
        const QJsonObject entry(aHistory.at(i).toObject());
        const QString isoDate(entry.value(QStringLiteral("departureDate")).toString());
        const QDate date(QDateTime::fromString(isoDate, Qt::ISODate).date());

        if (date.isValid()) {
            const int year = date.year();

            if (!years.contains(year)) {
                years.insert(0, year);
            }
        }
    }
    qSort(years);
    if (iYears != years) {
        if (last(iYears) != last(years)) {
            queueSignal(SignalLastYearChanged);
        }
        iYears = years;
        queueSignal(SignalYearsChanged);
    }

    if (rideInProgress() != wasInProgress) {
        queueSignal(SignalRideInProgressChanged);
        queueSignal(SignalRideDurationChanged);
        if (wasInProgress) {
            delete iRideDurationTimer;
            iRideDurationTimer = Q_NULLPTR;
        } else if (!iRideDurationTimer) {
            iRideDurationTimer = new QTimer(this);
            iRideDurationTimer->setInterval(1000);
            iRideDurationTimer->start();
            parentObject()->connect(iRideDurationTimer, SIGNAL(timeout()),
                SIGNAL(rideDurationChanged()));
        }
    }
    updated();
    setState(Ready);
    emitQueuedSignals();
}

void
BikeSession::Private::onServiceQueryFinished(
    const QJsonObject& aServiceInfo)
{
    setIdent(aServiceInfo.value(QStringLiteral("ident_type")).toString(),
        aServiceInfo.value(QStringLiteral("ident_data")).toString());
    refreshHistory();
    updated();
    emitQueuedSignals();
}

void
BikeSession::Private::onUserQueryFinished(
    const QJsonObject& aUserInfo)
{
    bool authenticated = aUserInfo.value(QStringLiteral("authenticated")).toBool();

    saveCookies();
    HDEBUG("authenticated:" << authenticated);
    if (authenticated) {
        userInfoReceived(aUserInfo);
    } else {
        iRequest.reset();
        setFirstName(QString());
        setLastName(QString());
        setState(Unauthorized);
    }
    emitQueuedSignals();
}

void
BikeSession::Private::onLoginSuccess(
    const QJsonObject& aUserInfo)
{
    saveCookies();
    setErrorText(QString());
    userInfoReceived(aUserInfo);
    emitQueuedSignals();
}

void
BikeSession::Private::onLoginFailure(
    QString aErrorMessage)
{
    HDEBUG(aErrorMessage);
    iRequest.reset();
    saveCookies();
    setErrorText(aErrorMessage);
    setState(LoginFailed);
    emitQueuedSignals();
}

void
BikeSession::Private::onLoginNetworkError()
{
    onLoginHttpError(0);
}

void
BikeSession::Private::onLoginHttpError(
    int aHttpError)
{
    iRequest.reset();
    iLastNetworkError = QDateTime::currentDateTime();
    queueSignal(SignalLastNetworkErrorChanged);
    setErrorText(QString());
    setHttpStatus(aHttpError);
    setState(LoginNetworkError);
    emitQueuedSignals();
}

void
BikeSession::Private::onLogoutDone()
{
    const bool rideWasInProgress = rideInProgress();

    iRequest.reset();
    iNetworkAccessManager.setCookieJar(new CookieJar(&iNetworkAccessManager));

    if (!iDataDir.isEmpty()) {
        QDir dir(iDataDir);

        if (dir.remove(COOKIES_FILE)) {
            HDEBUG("Removed" << qPrintable(dir.filePath(COOKIES_FILE)));
        }
    }

    if (!iHistory.isEmpty()) {
        iHistory = QJsonArray();
        queueSignal(SignalHistoryChanged);
    }

    if (!iYears.isEmpty()) {
        iYears.clear();
        queueSignal(SignalLastYearChanged);
        queueSignal(SignalYearsChanged);
    }

    if (rideWasInProgress) {
        queueSignal(SignalRideInProgressChanged);
        queueSignal(SignalRideDurationChanged);
        delete iRideDurationTimer;
        iRideDurationTimer = Q_NULLPTR;
    }

    setHttpStatus(BikeRequest::OK);
    setErrorText(QString());
    setFirstName(QString());
    setLastName(QString());
    setIdent(QString(), QString());
    setState(Unauthorized);

    start();
    emitQueuedSignals();
}

void
BikeSession::Private::onNetworkError()
{
    onHttpError(0);
}

void
BikeSession::Private::onHttpError(
    int aHttpError)
{
    iRequest.reset();
    iLastNetworkError = QDateTime::currentDateTime();
    queueSignal(SignalLastNetworkErrorChanged);
    setErrorText(QString());
    setHttpStatus(aHttpError);
    setState(NetworkError);
    emitQueuedSignals();
}

// ==========================================================================
// BikeSession
// ==========================================================================

BikeSession::BikeSession(
    QObject* aParent) :
    QObject(aParent),
    iPrivate(new Private(this))
{}

QString
BikeSession::dataDir() const
{
    return iPrivate->iDataDir;
}

void
BikeSession::setDataDir(
    QString aDataDir)
{
    iPrivate->setDataDir(aDataDir);
    iPrivate->emitQueuedSignals();
}

QString
BikeSession::login() const
{
    return iPrivate->iLogin;
}

QString
BikeSession::errorText() const
{
    return iPrivate->iErrorText;
}

int
BikeSession::httpError() const
{
    return iPrivate->iHttpError;
}

BikeSession::State
BikeSession::sessionState() const
{
    return iPrivate->iState;
}

QDateTime
BikeSession::lastUpdate() const
{
    return iPrivate->iLastUpdate;
}

QDateTime
BikeSession::lastNetworkError() const
{
    return iPrivate->iLastNetworkError;
}

QString
BikeSession::firstNames() const
{
    return iPrivate->iFirstNames;
}

QString
BikeSession::lastName() const
{
    return iPrivate->iLastName;
}

QString
BikeSession::fullName() const
{
    return iPrivate->iLastName.isEmpty() ? iPrivate->iFirstNames :
        iPrivate->iFirstNames + ' ' + iPrivate->iLastName;
}

QString
BikeSession::hslCard() const
{
    return iPrivate->iHslCard;
}

QString
BikeSession::nfcid1() const
{
    return iPrivate->iNfcid1;
}

QJsonArray
BikeSession::history() const
{
    return iPrivate->iHistory;
}

bool
BikeSession::rideInProgress() const
{
    return iPrivate->rideInProgress();
}

int
BikeSession::rideDuration() const
{
    return iPrivate->rideDuration();
}

QList<int>
BikeSession::years() const
{
    return iPrivate->iYears;
}

int
BikeSession::lastYear() const
{
    return Private::last(iPrivate->iYears);
}

int
BikeSession::thisYear() const
{
    return iPrivate->iThisYear;
}

void
BikeSession::restart()
{
    iPrivate->start();
}

void
BikeSession::signIn(
    QString aLogin,
    QString aPassword)
{
    iPrivate->signIn(aLogin, aPassword);
    iPrivate->emitQueuedSignals();
}

void
BikeSession::logOut()
{
    iPrivate->logOut();
    iPrivate->emitQueuedSignals();
}

void
BikeSession::refresh()
{
    HDEBUG("Refreshing history...");
    iPrivate->refreshHistory();
    iPrivate->emitQueuedSignals();
}

#include "BikeSession.moc"
