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

#include "BikeHistoryModel.h"

#include <QtCore/QDateTime>
#include <QtCore/QJsonObject>
#include <QtCore/QTimer>

#include "HarbourDebug.h"

#define ROLES_(first,role,last) \
    first(Bike,bike) \
    role(DepartureDate,departureDate) \
    role(DepartureStation,departureStation) \
    role(Distance,distance) \
    role(Duration,duration) \
    role(ReturnDate,returnDate) \
    role(ReturnStation,returnStation) \
    last(Month,month)

#define ROLES(role) \
    ROLES_(role,role,role)

// ==========================================================================
// BikeHistoryModel::Ride
// ==========================================================================

class BikeHistoryModel::Ride
{
public:
    // e.g. static const QString BikeKey;
    #define KEY_(X) X##Key
    #define ROLE(X,x) static const QString KEY_(X);
    ROLES(ROLE)
    #undef ROLE

    Ride(const QJsonObject&);

    static QDateTime date(const QJsonObject&, const QString&);
    bool inProgress() const;

public:
    QString iBike;
    QDateTime iDepartureDate;
    QString iDepartureStation;
    QDateTime iReturnDate;
    QString iReturnStation;
    int iMonth;
    int iDistance;
    int iDuration;
};

// e.g. const QString BikeHistoryModel::Ride::BikeKey("bike");
#define ROLE(X,x) const QString BikeHistoryModel::Ride::KEY_(X)(#x);
ROLES(ROLE)
#undef ROLE

//static
inline
QDateTime BikeHistoryModel::Ride::date(
    const QJsonObject& aJson,
    const QString& aKey)
{
    return QDateTime::fromString(aJson.value(aKey).toString(), Qt::ISODate);
}

BikeHistoryModel::Ride::Ride(
    const QJsonObject& aJson) :
    iBike(aJson.value(BikeKey).toString()),
    iDepartureDate(date(aJson, DepartureDateKey)),
    iDepartureStation(aJson.value(DepartureStationKey).toString()),
    iReturnDate(date(aJson, ReturnDateKey)),
    iReturnStation(aJson.value(ReturnStationKey).toString()),
    iMonth(iDepartureDate.date().month()),
    iDistance(aJson.value(DistanceKey).toInt()),
    iDuration(aJson.value(DurationKey).toInt())
{}

bool
BikeHistoryModel::Ride::inProgress() const
{
    return iDepartureDate.isValid() && !iDepartureStation.isEmpty() &&
        !iReturnDate.isValid() && iReturnStation.isEmpty();
}

// ==========================================================================
// BikeHistoryModel::Private
// ==========================================================================

class BikeHistoryModel::Private :
    public QObject
{
    Q_OBJECT

public:
    enum Role {
        #define ROLE_(X) X##Role
        #define ROLE(X,x) ROLE_(X),
        #define LAST(X,x) ROLE_(X)
        ROLE_(InProgress) = Qt::UserRole,
        ROLES_(ROLE,ROLE,LAST)
        #undef FIRST
        #undef ROLE
        #undef LAST
    };

    // QtCreator syntax highlighter gets confused by the above macro magic.
    // Somehow this stupid enum unconfuses it :/
    enum { _ };

    Private(BikeHistoryModel*);

    BikeHistoryModel* parentModel();
    bool acceptEntry(const QJsonObject&);
    void updateHistory();

private Q_SLOTS:
    void onRideDurationTimer();

public:
    QTimer* iRideDurationTimer;
    QJsonArray iHistory;
    QList<Ride> iRides;
    int iYear;
    int iMonth; // 1=Jan etc.
    int iMaxCount;
};

BikeHistoryModel::Private::Private(
    BikeHistoryModel* aParent) :
    QObject(aParent),
    iRideDurationTimer(Q_NULLPTR),
    iYear(0),
    iMonth(0),
    iMaxCount(0)
{}

inline
BikeHistoryModel*
BikeHistoryModel::Private::parentModel()
{
    return qobject_cast<BikeHistoryModel*>(parent());
}

bool
BikeHistoryModel::Private::acceptEntry(
    const QJsonObject& aEntry)
{
    if (!iYear && !iMonth) {
        return true;
    } else {
        QDate date(Ride::date(aEntry, Ride::DepartureDateKey).date());

        if (date.isValid()) {
            if (iYear) {
                if (date.year() == iYear) {
                    return !iMonth || date.month() == iMonth;
                }
            } else {
                return date.month() == iMonth;
            }
        }
    }
    return false;
}

void
BikeHistoryModel::Private::updateHistory()
{
    BikeHistoryModel* model = parentModel();
    const int n = iHistory.count();
    int pos = 0;

    // Take a simple approach - reset the model
    model->beginResetModel();
    for (int i = 0; i < n && (!iMaxCount || pos < iMaxCount); i++) {
        QJsonObject entry(iHistory.at(i).toObject());

        if (acceptEntry(entry)) {
            const Ride ride(entry);

            if (iRides.count() <= pos) {
                iRides.append(ride);
            } else {
                iRides[pos] = ride;
            }
            pos++;
        }
    }
    while (iRides.count() > pos) {
        iRides.removeLast();
    }
    HDEBUG(iRides.count() << "ride(s)");
    if (!iRides.isEmpty() &&
        iRides.first().inProgress() &&
        iRides.first().iDepartureDate.isValid()) {
        Ride& currentRide = iRides.first();
        const QDateTime now(QDateTime::currentDateTime());
        const qint64 secs = currentRide.iDepartureDate.secsTo(now);

        currentRide.iDuration = qMax(int(secs), 0);
        HDEBUG("Ride in progress" << currentRide.iDuration << "sec");
        if (!iRideDurationTimer) {
            iRideDurationTimer = new QTimer(this);
            iRideDurationTimer->setInterval(1000);
            iRideDurationTimer->start();
            connect(iRideDurationTimer, SIGNAL(timeout()),
                SLOT(onRideDurationTimer()));
        }
    } else if (iRideDurationTimer) {
        delete iRideDurationTimer;
        iRideDurationTimer = Q_NULLPTR;
    }
    model->endResetModel();
}

void
BikeHistoryModel::Private::onRideDurationTimer()
{
    Ride& currentRide = iRides.first();

    if (currentRide.iDepartureDate.isValid()) {
        const QDateTime now(QDateTime::currentDateTime());
        const qint64 secs = currentRide.iDepartureDate.secsTo(now);

        if (secs > currentRide.iDuration) {
            BikeHistoryModel* model = parentModel();
            const QModelIndex index(model->index(0));
            const QVector<int> role(1, DurationRole);

            HDEBUG(secs);
            currentRide.iDuration = int(secs);
            Q_EMIT model->dataChanged(index, index, role);
        }
    }
}

// ==========================================================================
// BikeHistoryModel
// ==========================================================================

BikeHistoryModel::BikeHistoryModel(
    QObject* aParent) :
    QAbstractListModel(aParent),
    iPrivate(new Private(this))
{}

QJsonArray
BikeHistoryModel::history() const
{
    return iPrivate->iHistory;
}

void
BikeHistoryModel::setHistory(
    QJsonArray aHistory)
{
    if (iPrivate->iHistory != aHistory) {
        iPrivate->iHistory = aHistory;
        iPrivate->updateHistory();
        Q_EMIT historyChanged();
    }
}

int
BikeHistoryModel::year() const
{
    return iPrivate->iYear;
}

void
BikeHistoryModel::setYear(
    int aYear)
{
    if (iPrivate->iYear != aYear) {
        iPrivate->iYear = aYear;
        HDEBUG(aYear);
        iPrivate->updateHistory();
        Q_EMIT yearChanged();
    }
}

int
BikeHistoryModel::month() const
{
    return iPrivate->iMonth;
}

void
BikeHistoryModel::setMonth(
    int aMonth)
{
    if (iPrivate->iMonth!= aMonth) {
        iPrivate->iMonth = aMonth;
        HDEBUG(aMonth);
        iPrivate->updateHistory();
        Q_EMIT monthChanged();
    }
}

int
BikeHistoryModel::maxCount() const
{
    return iPrivate->iMaxCount;
}

void
BikeHistoryModel::setMaxCount(
    int aMaxCount)
{
    if (iPrivate->iMaxCount != aMaxCount) {
        iPrivate->iMaxCount = aMaxCount;
        HDEBUG(aMaxCount);
        iPrivate->updateHistory();
        Q_EMIT maxCountChanged();
    }
}

// static
bool
BikeHistoryModel::rideInProgress(
    QJsonObject aObject)
{
    return Ride(aObject).inProgress();
}

QString
BikeHistoryModel::monthName(
    int aMonth)
{
    return QDate::longMonthName(aMonth, QDate::StandaloneFormat);
}

QHash<int,QByteArray>
BikeHistoryModel::roleNames() const
{
    QHash<int,QByteArray> roles;

    roles.insert(Private::ROLE_(InProgress), "inProgress");
    #define ROLE(X,x) roles.insert(Private::ROLE_(X), #x);
    ROLES(ROLE)
    #undef ROLE
    return roles;
}

int
BikeHistoryModel::rowCount(
    const QModelIndex&) const
{
    return iPrivate->iRides.count();
}

QVariant
BikeHistoryModel::data(
    const QModelIndex& aIndex,
    int aRole) const
{
    const int row = aIndex.row();

    if (row >= 0 && row < iPrivate->iRides.count()) {
        const Ride& ride = iPrivate->iRides.at(row);

        switch ((Private::Role)aRole) {
        #define ROLE(X,x) case Private::ROLE_(X): return ride.i##X;
        ROLES(ROLE)
        #undef ROLE
        case Private::ROLE_(InProgress): return ride.inProgress();
        }
    }
    return QVariant();
}

#include "BikeHistoryModel.moc"
