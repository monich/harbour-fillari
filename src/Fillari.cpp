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

#include "Fillari.h"

#include "HarbourDebug.h"

Fillari::Fillari(
    QObject* aParent) :
    QObject(aParent)
{}

// static
QObject*
Fillari::createSingleton(
    QQmlEngine*,
    QJSEngine*)
{
    return new Fillari();
}

// static
QString
Fillari::format(
    int aValue,
    BikeHistoryStats::Mode aMode)
{
    switch (aMode) {
    case BikeHistoryStats::Rides:
        return QString::number(aValue);
    case BikeHistoryStats::Distance:
        return (aValue < 1000) ?
            //: Distance, meters (shortened)
            //% "%1 m"
            qtTrId("fillari-distance-m").arg(aValue) :
            (aValue % 1000)?
            //: Distance, kilometers (shortened)
            //% "%1 km"
            qtTrId("fillari-distance-km").arg(aValue/1000.f, 0, 'f', 1) :
            qtTrId("fillari-distance-km").arg(aValue/1000);
    case BikeHistoryStats::Duration:
        return (aValue < 60) ?
            //: Duration, seconds (shortened)
            //% "%1 sec"
            qtTrId("fillari-duration-sec").arg(aValue) :
            (aValue < 3600) ?
            //: Duration, minutes (shortened)
            //% "%1 min"
            qtTrId("fillari-duration-min").arg(aValue/60) :
            (aValue % 3600) ?
            //: Duration, hours + minutes (shortened)
            //% "%1 h %2 min"
            qtTrId("fillari-duration-h_min").arg(aValue/3600).arg((aValue % 3600)/60) :
            //: Duration, hours(shortened)
            //% "%1 h"
            qtTrId("fillari-duration-h").arg(aValue/3600);
    }
    return QString();
}

// static
int
Fillari::step(
    int aMaxValue,
    int aMaxSteps,
    BikeHistoryStats::Mode aMode)
{

    if (aMaxSteps) {
        static const QVector<int> rideSteps{1, 5, 10, 50, 100};
        static const QVector<int> distanceSteps{1, 10, 100, 500,
            1000, 5000, 10000, 50000, 100000};
        static const QVector<int> durationSteps{1, 5, 60, 5*60, 10*60, 60*60};
        const QVector<int>* steps = Q_NULLPTR;
        switch (aMode) {
        case BikeHistoryStats::Rides:
            steps = &rideSteps;
            break;
        case BikeHistoryStats::Distance:
            steps = &distanceSteps;
            break;
        case BikeHistoryStats::Duration:
            steps = &durationSteps;
            break;
        }

        if (steps) {
            int i = steps->count() - 1;

            while (i > 0 && steps->at(i) * aMaxSteps > aMaxValue) i--;

            if (steps->at(i) <= aMaxValue) {
                const int d = steps->at(i);
                int s = d;

                while (aMaxValue / s >  aMaxSteps) s += d;
                HDEBUG(aMaxValue << aMaxSteps << aMode << "=>" << s);
                return s;
            }
        }
    }

    return 0;
}

