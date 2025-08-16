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
#include <QtCore/QHash>
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

    struct Stats {
        uint iRides;
        uint iDistance;
        uint iDuration;
    };

    struct Stash {
        Stash(Private*);
        void queueSignals(Private*);

        const int iMaxValue;
        const int iTotal;
    };

    Private();

    static QString shortMonthName(int);
    static int value(const Stats*, Mode);

    void queueSignal(Signal);
    void emitQueuedSignals(BikeHistoryStats*);
    void emitDataChanged(BikeHistoryStats*);
    void updateStats();
    int maxValue();
    int total();
    int yearTotal(int, Mode);
    QVariant data(int, Role);

public:
    SignalMask iQueuedSignals;
    Signal iFirstQueuedSignal;
    QJsonArray iHistory;
    Mode iMode;
    int iYear;
    QHash<int,Stats> iYearTotal;
    Stats iMonthTotal[Months];
    Stats iMaxPerMonth;
    const uint iFirstMonth;
    const uint iMonthCount;
};

BikeHistoryStats::Private::Private() :
    iQueuedSignals(0),
    iFirstQueuedSignal(SignalCount),
    iMode(Distance),
    iYear(0),
    iFirstMonth(3), // April (zero-based)
    iMonthCount(7)
{
    memset(iMonthTotal, 0, sizeof(iMonthTotal));
    memset(&iMaxPerMonth, 0, sizeof(iMaxPerMonth));
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

// static
int
BikeHistoryStats::Private::value(
    const Stats* aStats,
    Mode aMode)
{
    switch (aMode) {
    case Rides: return aStats->iRides;
    case Distance: return aStats->iDistance;
    case Duration: return aStats->iDuration;
    }
    return 0;
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

inline
int
BikeHistoryStats::Private::maxValue()
{
    return value(&iMaxPerMonth, iMode);
}

inline
int
BikeHistoryStats::Private::total()
{
    return yearTotal(iYear, iMode);
}

int
BikeHistoryStats::Private::yearTotal(
    int aYear,
    Mode aMode)
{
    return iYearTotal.contains(aYear) ? value(&iYearTotal[aYear], aMode) : 0;
}

void
BikeHistoryStats::Private::updateStats()
{
    const uint n = iHistory.size();

    iYearTotal.clear();
    memset(iMonthTotal, 0, sizeof(iMonthTotal));
    memset(&iMaxPerMonth, 0, sizeof(iMaxPerMonth));

    for (uint i = 0; i < n; i++) {
        const QJsonObject entry(iHistory.at(i).toObject());
        const int distance = entry.value(QStringLiteral("distance")).toInt();
        const int duration = entry.value(QStringLiteral("duration")).toInt();
        const QString isoDate(entry.value(QStringLiteral("departureDate")).toString());
        const QDate date(QDateTime::fromString(isoDate, Qt::ISODate).date());
        const int year = date.year();
        const int month = date.month();

        if (year) {
            Stats* stats = &iYearTotal[year];

            stats->iRides += 1;
            stats->iDistance += distance;
            stats->iDuration += duration;
        }

        if (month && (!iYear || iYear == year)) {
            Stats* stats = iMonthTotal + (month - 1);

            if ((stats->iRides += 1) > iMaxPerMonth.iRides) {
                iMaxPerMonth.iRides = stats->iRides;
            }
            if ((stats->iDistance += distance) > iMaxPerMonth.iDistance) {
                iMaxPerMonth.iDistance = stats->iDistance;
            }
            if ((stats->iDuration += duration) > iMaxPerMonth.iDuration) {
                iMaxPerMonth.iDuration = stats->iDuration;
            }
        }
    }

    #define PRINT_RIDES(i) << iMonthTotal[i].iRides
    #define PRINT_DISTANCE(i) << iMonthTotal[i].iDistance
    #define PRINT_DIRATION(i) << iMonthTotal[i].iDuration
    HDEBUG("Rides:" X12(PRINT_RIDES));
    HDEBUG("Distances:" X12(PRINT_DISTANCE) << "m");
    HDEBUG("Durations:" X12(PRINT_DIRATION) << "sec");
    #undef PRINT_RIDES
    #undef PRINT_DISTANCE
    #undef PRINT_DIRATION
}

QVariant
BikeHistoryStats::Private::data(
    int aRow,
    Role aRole)
{
    if (uint(aRow) < iMonthCount) {
        const uint monthIndex = iFirstMonth + aRow;
        const Stats* stats = iMonthTotal + monthIndex;

        switch (aRole) {
        case RoleMonth:
            return shortMonthName(monthIndex + 1);
        case RoleValue:
            return value(stats, iMode);
        }
    }
    return QVariant();
}

// ==========================================================================
// BikeHistoryStats::Private::Stash
// ==========================================================================

BikeHistoryStats::Private::Stash::Stash(
    Private* aPrivate) :
    iMaxValue(aPrivate->maxValue()),
    iTotal(aPrivate->total())
{}

void
BikeHistoryStats::Private::Stash::queueSignals(
    Private* aPrivate)
{
    if (aPrivate->maxValue() != iMaxValue) {
        HDEBUG("Max value" << aPrivate->maxValue());
        aPrivate->queueSignal(SignalMaxValueChanged);
    }
    if (aPrivate->total() != iTotal) {
        HDEBUG("Total" << aPrivate->total());
        aPrivate->queueSignal(SignalTotalChanged);
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
    if (iPrivate->iHistory != aHistory) {
        Private::Stash stash(iPrivate);

        iPrivate->iHistory = aHistory;
        iPrivate->updateStats();

        stash.queueSignals(iPrivate);
        iPrivate->queueSignal(Private::SignalHistoryChanged);
        iPrivate->emitQueuedSignals(this);
        iPrivate->emitDataChanged(this);
    }
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
        Private::Stash stash(iPrivate);

        HDEBUG(aMode);
        iPrivate->iMode = aMode;

        stash.queueSignals(iPrivate);
        iPrivate->queueSignal(Private::SignalModeChanged);
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
        Private::Stash stash(iPrivate);

        HDEBUG(aYear);
        iPrivate->iYear = aYear;
        iPrivate->updateStats();

        stash.queueSignals(iPrivate);
        iPrivate->queueSignal(Private::SignalYearChanged);
        iPrivate->emitQueuedSignals(this);
        iPrivate->emitDataChanged(this);
    }
}

int
BikeHistoryStats::maxValue() const
{
    return iPrivate->maxValue();
}

int
BikeHistoryStats::total() const
{
    return iPrivate->total();
}

int
BikeHistoryStats::yearTotal(
    int aYear,
    Mode aMode)
{
    return iPrivate->yearTotal(aYear, aMode);
}

int
BikeHistoryStats::monthTotal(
    int aMonth,
    Mode aMode)
{
    return (aMonth > 0 && aMonth <= Private::Months) ?
        Private::value(iPrivate->iMonthTotal + (aMonth - 1), aMode) : 0;
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
    return iPrivate->data(aIndex.row(), (Private::Role) aRole);
}
