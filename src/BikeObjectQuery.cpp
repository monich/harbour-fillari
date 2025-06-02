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

#include "BikeObjectQuery.h"

#include "BikeApp.h"

#include <QtCore/QJsonObject>
#include <QtCore/QJsonDocument>

#include "HarbourDebug.h"

// ==========================================================================
// BikeObjectQuery::Private
// ==========================================================================

class BikeObjectQuery::Private
{
public:
    static QNetworkReply* submit(BikeObjectQuery*, const QString&);
};

//static
QNetworkReply*
BikeObjectQuery::Private::submit(
    BikeObjectQuery* aRequest,
    const QString& aUrl)
{
    return aRequest->get(aUrl, jsonApiHeaders());
}

// ==========================================================================
// BikeObjectQuery
// ==========================================================================

BikeObjectQuery::BikeObjectQuery(
    QString aUrl,
    BikeRequest* aParent) :
    BikeRequest(aParent)
{
    connect(Private::submit(this, aUrl), SIGNAL(finished()),
        SLOT(onQueryFinished()));
}

BikeObjectQuery::BikeObjectQuery(
    QString aUrl,
    QNetworkAccessManager* aParent) :
    BikeRequest(aParent)
{
    connect(Private::submit(this, aUrl), SIGNAL(finished()),
        SLOT(onQueryFinished()));
}

void
BikeObjectQuery::onQueryFinished()
{
    Reply reply(sender());
    const int status = statusCode(reply);

    HDEBUG(qPrintable(toString(reply)));
    if (status) {
        QJsonObject replyJson;

        updateCookies(reply);
        if (status == OK) {
            const QByteArray replyData(reply->readAll());

            replyJson = QJsonDocument::fromJson(replyData).object();
            HDEBUG(replyData.constData());
            Q_EMIT finished(replyJson);
        } else {
            HDEBUG(reply->readAll().constData());
            Q_EMIT httpError(status);
        }
    } else {
        Q_EMIT networkError();
    }
    Q_EMIT done();
}

// ==========================================================================
// BikeUserQuery
// ==========================================================================

const QString BikeUserQuery::URL
    (BIKE_API_URL("menu?language=en&path=/omat-tiedot/kaupunkipyorat/matkahistoria"));

BikeUserQuery::BikeUserQuery(
    BikeRequest* aParent) :
    BikeObjectQuery(URL, aParent)
{}

BikeUserQuery::BikeUserQuery(
    QNetworkAccessManager* aParent) :
    BikeObjectQuery(URL, aParent)
{}

// ==========================================================================
// BikeServiceQuery
// ==========================================================================

const QString BikeServiceQuery::URL(BIKE_API_URL("citybikes/cbf/maas-user"));

BikeServiceQuery::BikeServiceQuery(
    QNetworkAccessManager* aParent) :
    BikeObjectQuery(URL, aParent)
{}
