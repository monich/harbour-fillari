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

#ifndef BIKE_HISTORY_STATS_H
#define BIKE_HISTORY_STATS_H

#include <QtCore/QAbstractListModel>
#include <QtCore/QJsonArray>

// The JSON array contains objects like this:
//
// {
//   "bike": "XXXX",
//   "departureDate": "2025-05-30T01:24:21Z",
//   "departureStation": "XXX yyyyyyyyy",
//   "distance": 1341,
//   "duration": 509,
//   "providerName": "helsinki-espoo",
//   "returnDate": "2025-05-30T01:32:56Z",
//   "returnStation": "XXX yyyyyyyyy"
// }
//

class BikeHistoryStats :
    public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QJsonArray history READ history WRITE setHistory NOTIFY historyChanged)
    Q_PROPERTY(Mode mode READ mode WRITE setMode NOTIFY modeChanged)
    Q_PROPERTY(int year READ year WRITE setYear NOTIFY yearChanged)
    Q_PROPERTY(int maxValue READ maxValue NOTIFY maxValueChanged)
    Q_PROPERTY(int total READ total NOTIFY totalChanged)
    Q_ENUMS(Mode)

public:
    enum Mode {
        Rides,
        Distance,   // meters
        Duration    // minutes
    };

    BikeHistoryStats(QObject* aParent = Q_NULLPTR);
    ~BikeHistoryStats();

    QJsonArray history() const;
    void setHistory(QJsonArray);

    Mode mode() const;
    void setMode(Mode);

    int year() const;
    void setYear(int);

    int maxValue() const;
    int total() const;

    Q_INVOKABLE int yearTotal(int, Mode);
    Q_INVOKABLE int monthTotal(int, Mode);
    Q_INVOKABLE QString formatMonthTotal(int, Mode);

    // QAbstractItemModel
    QHash<int,QByteArray> roleNames() const Q_DECL_OVERRIDE;
    int rowCount(const QModelIndex&) const Q_DECL_OVERRIDE;
    QVariant data(const QModelIndex&, int) const Q_DECL_OVERRIDE;

Q_SIGNALS:
    void historyChanged();
    void modeChanged();
    void yearChanged();
    void maxValueChanged();
    void totalChanged();

private:
    class Private;
    Private* iPrivate;
};

#endif // BIKE_HISTORY_STATS_H
