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

#include "BikeRequest.h"
#include "BikeApp.h"

#include <QtCore/QUrl>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkCookie>
#include <QtNetwork/QNetworkCookieJar>

// ==========================================================================
// BikeRequest::Private
// ==========================================================================

class BikeRequest::Private
{
public:
    static const QString USER_AGENT;
    static const QList<QNetworkReply::RawHeaderPair> DEFAULT_HEADERS;
};

const QString
BikeRequest::Private::USER_AGENT("Mozilla/5.0 (X11; Linux x86_64; rv:138.0) Gecko/20100101 Firefox/138.0");
const QList<QNetworkReply::RawHeaderPair>
BikeRequest::Private::DEFAULT_HEADERS(
    QList<QNetworkReply::RawHeaderPair>() <<
    QNetworkReply::RawHeaderPair("DNT", "1") <<
    QNetworkReply::RawHeaderPair("Sec-GPC", "1") <<
    QNetworkReply::RawHeaderPair("Connection", "keep-alive"));

// ==========================================================================
// BikeRequest::Reply
// ==========================================================================

BikeRequest::Reply::Reply(QObject* aSender) :
    QScopedPointer<QNetworkReply, QScopedPointerObjectDeleteLater<QNetworkReply>>
        (qobject_cast<QNetworkReply*>(aSender))
    { HASSERT(data()); }

// ==========================================================================
// BikeRequest
// ==========================================================================

BikeRequest::BikeRequest(
    BikeRequest* aParent) :
    QObject(aParent)
{}

BikeRequest::BikeRequest(
    QNetworkAccessManager* aParent) :
    QObject(aParent)
{}

QNetworkAccessManager*
BikeRequest::getNetworkAccessManager() const
{
    QObject* parentObject = parent();
    QNetworkAccessManager* am = qobject_cast<QNetworkAccessManager*>(parentObject);

    if (am) {
        return am;
    } else {
        BikeRequest* parentReq = qobject_cast<BikeRequest*>(parentObject);

        if (parentReq) {
            return parentReq->getNetworkAccessManager();
        }
    }
    return Q_NULLPTR;
}

QNetworkRequest
BikeRequest::createRequest(
    QString aUrl,
    QList<HeaderPair> aHeaders) const
{
    const QUrl url(aUrl);
    QNetworkRequest req(url);

    req.setHeader(QNetworkRequest::UserAgentHeader, Private::USER_AGENT);

    QListIterator<HeaderPair> defaultHeaders(Private::DEFAULT_HEADERS);
    while (defaultHeaders.hasNext()) {
        HeaderPair h = defaultHeaders.next();
        req.setRawHeader(h.first, h.second);
    }

    QListIterator<HeaderPair> headers(aHeaders);
    while (headers.hasNext()) {
        HeaderPair h = headers.next();
        req.setRawHeader(h.first, h.second);
    }

    QList<QNetworkCookie> cookies(getNetworkAccessManager()->cookieJar()->
        cookiesForUrl(url));
    if (!cookies.isEmpty()) {
        req.setHeader(QNetworkRequest::CookieHeader,
            QVariant::fromValue<QList<QNetworkCookie>>(cookies));
    }

    return req;
}

QNetworkReply*
BikeRequest::get(
    QString aUrl,
    QList<HeaderPair> aHeaders) const
{
    QNetworkRequest req(createRequest(aUrl, aHeaders));

    HDEBUG("============ GET ================");
    HDEBUG(qPrintable(toString(req)));

    return getNetworkAccessManager()->get(req);
}

QNetworkReply*
BikeRequest::post(
    QString aUrl,
    QList<HeaderPair> aHeaders,
    QString aContentType,
    QByteArray aPostData) const
{
    QNetworkRequest req(createRequest(aUrl, aHeaders));

    req.setHeader(QNetworkRequest::ContentTypeHeader, aContentType);
    HDEBUG("============ POST================");
    HDEBUG(qPrintable(toString(req)));
    HDEBUG("Data:");
    HDEBUG(aPostData.constData());

    return getNetworkAccessManager()->post(req, aPostData);
}

void
BikeRequest::updateCookies(
    QNetworkReply* aReply)
{
    if (aReply) {
        getNetworkAccessManager()->cookieJar()->setCookiesFromUrl(aReply->
            header(QNetworkRequest::SetCookieHeader). value<QList<QNetworkCookie>>(),
            aReply->url());
    }
}

//static
QList<BikeRequest::HeaderPair>
BikeRequest::jsonApiHeaders()
{
    static const QList<HeaderPair> headers(QList<HeaderPair>() <<
        HeaderPair("Referer", BIKE_DEFAULT_REFERER) <<
        HeaderPair("Sec-Fetch-Dest", "empty") <<
        HeaderPair("Sec-Fetch-Mode", "cors") <<
        HeaderPair("Sec-Fetch-Site", "same-origin") <<
        HeaderPair("TE", "trailers") <<
        HeaderPair("Priority", "u=4"));

    return headers;
}

//static
int
BikeRequest::statusCode(
    const QNetworkReply* aReply)
{
    return aReply ?
        aReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() :
        0;
}

#if HARBOUR_DEBUG
//static
QString
BikeRequest::toString(
    const QNetworkRequest& aReq)
{
    QString str(aReq.url().toString());
    const QList<QByteArray> headers(aReq.rawHeaderList());

    if (!headers.isEmpty()) {
        QListIterator<QByteArray> it(headers);

        str += "\nHeaders:";
        while (it.hasNext()) {
            const QByteArray h(it.next());

            str += "\n  ";
            str += h.constData();
            str += ": ";
            str += aReq.rawHeader(h).constData();
        }
    }
    return str;
}
#endif

#if HARBOUR_DEBUG
//static
QString
BikeRequest::toString(
    const QNetworkReply* aReply)
{
    QString str(aReply->url().toString());
    const QList<HeaderPair>& headers = aReply->rawHeaderPairs();

    str += " " + QString::number(statusCode(aReply));
    if (!headers.isEmpty()) {
        QListIterator<HeaderPair> it(headers);

        str += "\nHeaders:";
        while (it.hasNext()) {
            HeaderPair h = it.next();

            str += "\n  ";
            str += h.first.constData();
            str +=  ": ";
            str +=  h.second.constData();
        }
    }
    return str;
}
#endif
