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

#include "BikeLogout.h"

#include "HarbourDebug.h"

// ==========================================================================
// BikeLogout::Private
// ==========================================================================

class BikeLogout::Private :
    public QObject
{
    Q_OBJECT

public:
    Private(BikeLogout*);

private Q_SLOTS:
    void onRequestFinished();

public:
    int iRedirectCount;
    const QList<HeaderPair> iHeaders;
};

BikeLogout::Private::Private(
    BikeLogout* aParent) :
    QObject(aParent),
    iRedirectCount(0),
    iHeaders(QList<HeaderPair>() <<
        HeaderPair("Referer", "https://www.hsl.fi/") <<
        HeaderPair("Sec-Fetch-Dest", "iframe") <<
        HeaderPair("Sec-Fetch-Mode", "navigate") <<
        HeaderPair("Sec-Fetch-Site", "same-site") <<
        HeaderPair("Priority", "u=4") <<
        HeaderPair("Upgrade-Insecure-Requests", "1"))
{
    connect(aParent->get("https://www.hsl.fi/user/auth/logout", iHeaders),
        SIGNAL(finished()), SLOT(onRequestFinished()));
}

void
BikeLogout::Private::onRequestFinished()
{
    Reply reply(sender());
    BikeLogout* owner = qobject_cast<BikeLogout*>(parent());
    const int status = statusCode(reply);

    HDEBUG(qPrintable(toString(reply)));
    if (status) {
        if (status == Found && iRedirectCount < 2) {
            // Not done yet
            iRedirectCount++;
            HDEBUG("Following redirect" << iRedirectCount);
            owner->updateCookies(reply);
            connect(owner->get(reply->rawHeader("Location"), iHeaders),
                SIGNAL(finished()), SLOT(onRequestFinished()));
            return;
        } else {
            HDEBUG(reply->readAll().constData());
            if (status != OK) {
                Q_EMIT owner->httpError(status);
            }
            Q_EMIT owner->finished();
        }
    } else {
        Q_EMIT owner->networkError();
    }
    Q_EMIT owner->done();
}

// ==========================================================================
// BikeLogout
// ==========================================================================

BikeLogout::BikeLogout(
    QNetworkAccessManager* aParent) :
    BikeRequest(aParent),
    iPrivate(new Private(this))
{}

#include "BikeLogout.moc"
