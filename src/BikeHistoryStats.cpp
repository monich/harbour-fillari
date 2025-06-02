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

#include "BikeHistoryStats.h"
#include "Fillari.h"

#include <QtCore/QDate>
#include <QtCore/QDateTime>
#include <QtCore/QList>
#include <QtCore/QJsonObject>
#include <QtCore/QVector>

#include "HarbourDebug.h"

#define X12(x) x(0) x(1) x(2) x(3) x(4) x(5) x(6) x(7) x(8) x(9) x(10) x(11)

// s(SignalName,signalName)
#define QUEUED_SIGNALS(s) \
    s(History,history) \
    s(Mode,mode) \
    s(Year,year) \
    s(MaxValue,maxValue) \
    s(Total,total)

// ==========================================================================
// BikeHistoryStats::Private
// ==========================================================================

class BikeHistoryStats::Private
{
public:
    typedef void (BikeHistoryStats::*SignalEmitter)();
    typedef uint SignalMask;
    enum Signal {
        #define SIGNAL_ENUM_(Name,name) Signal##Name##Changed,
        QUEUED_SIGNALS(SIGNAL_ENUM_)
        #undef  SIGNAL_ENUM_
        SignalCount
    };

    enum {
        Months = 12
    };

    enum Role {
        RoleMonth = Qt::UserRole,
        RoleValue
    };

    Private();

    static QString shortMonthName(int);

    void queueSignal(Signal);
    void emitQueuedSignals(BikeHistoryStats*);
    void emitDataChanged(BikeHistoryStats*);
    void updateStats();
    void updateTotals();

public:
    SignalMask iQueuedSignals;
    Signal iFirstQueuedSignal;
    Mode iMode;
    int iYear;
    QJsonArray iHistory;
    uint iMaxValue;
    uint iTotal;
    const uint iFirstMonth;
    const uint iMonthCount;
    uint iRides[Months];
    uint iDistance[Months];
    uint iDuration[Months];
};

BikeHistoryStats::Private::Private() :
    iQueuedSignals(0),
    iFirstQueuedSignal(SignalCount),
    iMode(Distance),
    iYear(0),
    iMaxValue(0),
    iTotal(0),
    iFirstMonth(3), // April (zero-based)
    iMonthCount(7)
{
    memset(iRides, 0, sizeof(iRides));
    memset(iDistance, 0, sizeof(iDistance));
    memset(iDuration, 0, sizeof(iDuration));
}

//static
QString
BikeHistoryStats::Private::shortMonthName(
    int aMonth)
{
    QString name(QDate::shortMonthName(aMonth));

    // Make sure that the short month name is really short
    name.truncate(3);
    return name;
}

void
BikeHistoryStats::Private::queueSignal(
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
BikeHistoryStats::Private::emitQueuedSignals(
    BikeHistoryStats* aObject)
{
    static const SignalEmitter emitSignal [] = {
        #define SIGNAL_EMITTER_(Name,name) &BikeHistoryStats::name##Changed,
        QUEUED_SIGNALS(SIGNAL_EMITTER_)
        #undef SIGNAL_EMITTER_
    };

    if (iQueuedSignals) {
        uint i = iFirstQueuedSignal;

        // Reset first queued signal before emitting the signals.
        // Signal handlers may emit new signals.
        iFirstQueuedSignal = SignalCount;
        for (; i < SignalCount && iQueuedSignals; i++) {
            const SignalMask signalBit = (SignalMask(1) << i);
            if (iQueuedSignals & signalBit) {
                iQueuedSignals &= ~signalBit;
                Q_EMIT (aObject->*(emitSignal[i]))();
            }
        }
    }
}

void
BikeHistoryStats::Private::emitDataChanged(
    BikeHistoryStats* aModel)
{
    Q_EMIT aModel->dataChanged(aModel->index(0),
        aModel->index(iMonthCount - 1),
        QVector<int>{Private::RoleValue});
}

void
BikeHistoryStats::Private::updateStats()
{
    const uint n = iHistory.size();

    memset(iRides, 0, sizeof(iRides));
    memset(iDistance, 0, sizeof(iDistance));
    memset(iDuration, 0, sizeof(iDuration));

    for (uint i = 0; i < n; i++) {
        const QJsonObject entry(iHistory.at(i).toObject());
        const int distance = entry.value(QStringLiteral("distance")).toInt();
        const int duration = entry.value(QStringLiteral("duration")).toInt();
        const QString isoDate(entry.value(QStringLiteral("departureDate")).toString());
        const QDate date(QDateTime::fromString(isoDate, Qt::ISODate).date());
        const int year = date.year();
        const int month = date.month();

        if (month && (!iYear || iYear == year)) {
            const uint monthIndex = month - 1;

            iRides[monthIndex] += 1;
            iDistance[monthIndex] += distance;
            iDuration[monthIndex] += duration;
        }
    }

    #define PRINT_RIDES(i) << iRides[i]
    #define PRINT_DISTANCE(i) << iDistance[i]
    #define PRINT_DIRATION(i) << iDuration[i]
    HDEBUG("Rides:" X12(PRINT_RIDES));
    HDEBUG("Distances:" X12(PRINT_DISTANCE) << "m");
    HDEBUG("Durations:" X12(PRINT_DIRATION) << "sec");
    #undef PRINT_RIDES
    #undef PRINT_DISTANCE
    #undef PRINT_DIRATION

    updateTotals();
}

void
BikeHistoryStats::Private::updateTotals()
{
    const uint* values = iRides;
    const uint prevMaxValue = iMaxValue;
    const uint prevTotal = iTotal;

    switch (iMode) {
    case Rides:
        values = iRides;
        break;
    case Distance:
        values = iDistance;
        break;
    case Duration:
        values = iDuration;
        break;
    }

    iMaxValue = 0;
    iTotal = 0;
    for (uint i = 0; i < uint(Private::Months); i++) {
        iMaxValue = qMax(values[i], iMaxValue);
        iTotal += values[i];
    }

    if (iMaxValue != prevMaxValue) {
        HDEBUG("Max value" << iMaxValue);
        queueSignal(SignalMaxValueChanged);
    }

    if (iTotal != prevTotal) {
        HDEBUG("Total" << iTotal);
        queueSignal(SignalTotalChanged);
    }
}

// ==========================================================================
// BikeHistoryStats
// ==========================================================================

BikeHistoryStats::BikeHistoryStats(
    QObject* aParent) :
    QAbstractListModel(aParent),
    iPrivate(new Private())
{}

BikeHistoryStats::~BikeHistoryStats()
{
    delete iPrivate;
}

QJsonArray
BikeHistoryStats::history() const
{
    return iPrivate->iHistory;
}

void
BikeHistoryStats::setHistory(
    QJsonArray aHistory)
{
    // Assume that it's actually changed
    iPrivate->iHistory = aHistory;
    iPrivate->queueSignal(Private::SignalHistoryChanged);
    iPrivate->updateStats();
    iPrivate->emitQueuedSignals(this);
    iPrivate->emitDataChanged(this);
}

BikeHistoryStats::Mode
BikeHistoryStats::mode() const
{
    return iPrivate->iMode;
}

void
BikeHistoryStats::setMode(
    Mode aMode)
{
    if (iPrivate->iMode != aMode) {
        HDEBUG(aMode);
        iPrivate->iMode = aMode;
        iPrivate->queueSignal(Private::SignalModeChanged);
        iPrivate->updateTotals();
        iPrivate->emitQueuedSignals(this);
        iPrivate->emitDataChanged(this);
    }
}

int
BikeHistoryStats::year() const
{
    return iPrivate->iYear;
}

void
BikeHistoryStats::setYear(
    int aYear)
{
    if (iPrivate->iYear != aYear) {
        HDEBUG(aYear);
        iPrivate->iYear = aYear;
        iPrivate->queueSignal(Private::SignalYearChanged);
        iPrivate->updateStats();
        iPrivate->emitQueuedSignals(this);
        iPrivate->emitDataChanged(this);
    }
}

int
BikeHistoryStats::maxValue() const
{
    return iPrivate->iMaxValue;
}

int
BikeHistoryStats::total() const
{
    return iPrivate->iTotal;
}

int
BikeHistoryStats::monthTotal(
    int aMonth,
    Mode aMode)
{
    if (aMonth > 0 && aMonth <= Private::Months) {
        switch (aMode) {
        case Rides: return iPrivate->iRides[aMonth - 1];
        case Distance: return iPrivate->iDistance[aMonth - 1];
        case Duration: return iPrivate->iDuration[aMonth - 1];
        }
    }
    return 0;
}

QString
BikeHistoryStats::formatMonthTotal(
    int aMonth,
    Mode aMode)
{
    return Fillari::format(monthTotal(aMonth, aMode), aMode);
}

QHash<int,QByteArray>
BikeHistoryStats::roleNames() const
{
    QHash<int,QByteArray> roles;

    roles.insert(Private::RoleMonth, "month");
    roles.insert(Private::RoleValue, "value");
    return roles;
}

int
BikeHistoryStats::rowCount(
    const QModelIndex&) const
{
    return iPrivate->iMonthCount;
}

QVariant
BikeHistoryStats::data(
    const QModelIndex& aIndex,
    int aRole) const
{
    const int row = aIndex.row();

    if (row >= 0 && uint(row) < iPrivate->iMonthCount) {
        const uint monthIndex = iPrivate->iFirstMonth + row;

        switch ((Private::Role)aRole) {
        case Private::RoleMonth:
            return Private::shortMonthName(monthIndex + 1);
        case Private::RoleValue:
            switch (iPrivate->iMode) {
            case Rides: return iPrivate->iRides[monthIndex];
            case Distance: return iPrivate->iDistance[monthIndex];
            case Duration: return iPrivate->iDuration[monthIndex];
            }
            break;
        }
    }
    return QVariant();
}
