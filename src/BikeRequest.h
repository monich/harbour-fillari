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

#ifndef BIKE_REQUEST_H
#define BIKE_REQUEST_H

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QList>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>

#include "HarbourDebug.h"

class QJsonObject;
class QNetworkAccessManager;

class BikeRequest :
    public QObject
{
    Q_OBJECT
    class Private;

public:
    enum HttpCode {
        OK = 200,
        Found = 302,
        Unauthorized = 401,
        Forbidden = 403
    };

    typedef QScopedPointer<BikeRequest,
        QScopedPointerObjectDeleteLater<BikeRequest>> Ptr;

    // Automates deleteLater() call
    class Reply : public QScopedPointer<QNetworkReply,
        QScopedPointerObjectDeleteLater<QNetworkReply>>
    {
    public:
        Reply(QObject*);
        operator QNetworkReply*() const { return data(); }
    };

protected:
    using HeaderPair = QNetworkReply::RawHeaderPair;
    BikeRequest(BikeRequest*);
    BikeRequest(QNetworkAccessManager*);

    QNetworkAccessManager* getNetworkAccessManager() const;
    QNetworkRequest createRequest(QString, QList<HeaderPair>) const;
    QNetworkReply* get(QString, QList<HeaderPair>) const;
    QNetworkReply* post(QString, QList<HeaderPair>, QString, QByteArray) const;
    void updateCookies(QNetworkReply*);

    static QList<HeaderPair> jsonApiHeaders();
    static int statusCode(const QNetworkReply*);

    #if HARBOUR_DEBUG
    static QString toString(const QNetworkRequest&);
    static QString toString(const QNetworkReply*);
    #endif

Q_SIGNALS:
    void httpError(int);
    void networkError();
    void done();
};

#endif // BIKE_REQUEST_H
