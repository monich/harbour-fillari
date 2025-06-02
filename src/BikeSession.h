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

#ifndef BIKE_SESSION_H
#define BIKE_SESSION_H

#include <QtCore/QDateTime>
#include <QtCore/QJsonArray>
#include <QtCore/QList>
#include <QtCore/QObject>
#include <QtCore/QString>

class BikeSession:
    public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString dataDir READ dataDir WRITE setDataDir NOTIFY dataDirChanged)
    Q_PROPERTY(QString login READ login NOTIFY loginChanged)
    Q_PROPERTY(QString errorText READ errorText NOTIFY errorTextChanged)
    Q_PROPERTY(int httpError READ httpError NOTIFY httpErrorChanged)
    Q_PROPERTY(State sessionState READ sessionState NOTIFY sessionStateChanged)
    Q_PROPERTY(QDateTime lastUpdate READ lastUpdate NOTIFY lastUpdateChanged)
    Q_PROPERTY(QDateTime lastNetworkError READ lastNetworkError NOTIFY lastNetworkErrorChanged)
    Q_PROPERTY(QString firstNames READ firstNames NOTIFY firstNamesChanged)
    Q_PROPERTY(QString lastName READ lastName NOTIFY lastNameChanged)
    Q_PROPERTY(QString fullName READ fullName NOTIFY fullNameChanged)
    Q_PROPERTY(QString identType READ identType NOTIFY identTypeChanged)
    Q_PROPERTY(QString identData READ identData NOTIFY identDataChanged)
    Q_PROPERTY(QJsonArray history READ history NOTIFY historyChanged)
    Q_PROPERTY(bool rideInProgress READ rideInProgress NOTIFY rideInProgressChanged)
    Q_PROPERTY(int rideDuration READ rideDuration NOTIFY rideDurationChanged)
    Q_PROPERTY(QList<int> years READ years NOTIFY yearsChanged)
    Q_PROPERTY(int lastYear READ lastYear NOTIFY lastYearChanged)
    Q_PROPERTY(int thisYear READ thisYear NOTIFY thisYearChanged)
    Q_ENUMS(State)

public:
    enum State {
        None,
        LoginCheck,
        UserInfoQuery,
        HistoryQuery,
        Unauthorized,
        LoggingIn,
        LoginFailed,
        LoginNetworkError,
        NetworkError,
        Ready
    };

    explicit BikeSession(QObject* aParent = Q_NULLPTR);

    QString dataDir() const;
    void setDataDir(QString);

    QString login() const;
    QString errorText() const;
    int httpError() const;
    State sessionState() const;
    QDateTime lastUpdate() const;
    QDateTime lastNetworkError() const;
    QString firstNames() const;
    QString lastName() const;
    QString fullName() const;
    QString identType() const;
    QString identData() const;
    QJsonArray history() const;
    bool rideInProgress() const;
    int rideDuration() const;
    QList<int> years() const;
    int lastYear() const;
    int thisYear() const;

    Q_INVOKABLE void signIn(QString, QString);
    Q_INVOKABLE void restart();
    Q_INVOKABLE void refresh();

Q_SIGNALS:
    void dataDirChanged();
    void loginChanged();
    void errorTextChanged();
    void httpErrorChanged();
    void sessionStateChanged();
    void lastUpdateChanged();
    void lastNetworkErrorChanged();
    void firstNamesChanged();
    void lastNameChanged();
    void fullNameChanged();
    void identTypeChanged();
    void identDataChanged();
    void historyChanged();
    void rideInProgressChanged();
    void rideDurationChanged();
    void yearsChanged();
    void lastYearChanged();
    void thisYearChanged();

private:
    class CookieJar;
    class Private;
    Private* iPrivate;
};

#endif // BIKE_SESSION_H
