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

#include "BikeLogin.h"
#include "BikeObjectQuery.h"

#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>

#include "HarbourDebug.h"

// ==========================================================================
// BikeLogin::Private
// ==========================================================================

class BikeLogin::Private :
    public QObject
{
    Q_OBJECT

    static const QLatin1String csrfTokenKey;
    static const QLatin1String rpcKey;
    static const QLatin1String syncIdKey;
    static const QLatin1String clientIdKey;
    static const QString JsonContentType;

public:
    Private(BikeLogin*, QString, QString);

    BikeLogin* parentObject();
    static QJsonObject replyToJsonObject(const QByteArray&);

public Q_SLOTS:
    void onGetLoginFinished();
    void onGetAuthFinished();
    void onGetAuthUiFinished();
    void onPostAuthUiFinished();
    void onPostDelayedCallbackRpcReceivedFinished();
    void onPostUIServerRpcResizeFinished();
    void onPostAbstractTextFieldServerRpcSetPasswordFinished();
    void onPostAbstractTextFieldServerRpcSetUsernameFinished();
    void onPostButtonServerRpcClickFinished();
    void onGetAuthRedirectFinished();
    void onGetHslidFinished();

private:
    const QString iLogin;
    const QString iPassword;
    QString iAuthUiUrl;
    QString iAuthUidlUrl;
    QString iCsrfToken;
    QString iUsernameField;
    QString iPasswordField;
    QString iLoginButton;
    QString iWsver;
    int iSyncId;
    int iClientId;
    QList<HeaderPair> iAuthUidlHeaders;
};

const QLatin1String BikeLogin::Private::csrfTokenKey("csrfToken");
const QLatin1String BikeLogin::Private::rpcKey("rpc");
const QLatin1String BikeLogin::Private::syncIdKey("syncId");
const QLatin1String BikeLogin::Private::clientIdKey("clientId");
const QString BikeLogin::Private::JsonContentType("application/json;charset=utf-8");

BikeLogin::Private::Private(
    BikeLogin* aParent,
    QString aLogin,
    QString aPassword) :
    QObject(aParent),
    iLogin(aLogin),
    iPassword(aPassword),
    iSyncId(0),
    iClientId(0)
{}

inline
BikeLogin*
BikeLogin::Private::parentObject()
{
    return qobject_cast<BikeLogin*>(parent());
}

//static
QJsonObject
BikeLogin::Private::replyToJsonObject(
    const QByteArray& aData)
{
    static const QByteArray prefix("for(;;);[");
    static const QByteArray suffix("]");
    QJsonObject obj;

    // Typically we get something like this:
    // for(;;);[{"syncId": 1, "clientId": 1, "changes" : [], ... }]
    if (aData.startsWith(prefix) && aData.endsWith(suffix)) {
        const QByteArray cleanData(aData.mid(prefix.length(),
            aData.length() - prefix.length() - suffix.length()));
        HDEBUG(cleanData.constData());
        obj = QJsonDocument::fromJson(cleanData).object();
    } else {
        obj = QJsonDocument::fromJson(aData).object();
    }
    HDEBUG(obj);
    return obj;
}

void
BikeLogin::Private::onGetLoginFinished()
{
    Reply reply(sender());
    BikeLogin* owner = parentObject();
    const int status = statusCode(reply);

    HDEBUG(qPrintable(toString(reply)));
    if (status == Found) {
        owner->updateCookies(reply);
        connect(owner->get(reply->rawHeader("Location"), QList<HeaderPair>() <<
            HeaderPair("Referer", "https://www.hsl.fi/") <<
            HeaderPair("Sec-Fetch-Dest", "document") <<
            HeaderPair("Sec-Fetch-Mode", "navigate") <<
            HeaderPair("Sec-Fetch-Site", "same-site") <<
            HeaderPair("Sec-Fetch-User", "?1") <<
            HeaderPair("Priority", "u=0, i") <<
            HeaderPair("Upgrade-Insecure-Requests", "1")), SIGNAL(finished()), SLOT(onGetAuthFinished()));
    } else {
        HDEBUG(reply->readAll().constData());
        Q_EMIT owner->httpError(status);
    }
}

void
BikeLogin::Private::onGetAuthFinished()
{
    Reply reply(sender());
    BikeLogin* owner = parentObject();
    const int status = statusCode(reply);

    HDEBUG(qPrintable(toString(reply)));
    if (status == Found) {
        iAuthUiUrl = QString::fromLatin1(reply->rawHeader("Location"));
        owner->updateCookies(reply);
        connect(owner->get(iAuthUiUrl, QList<HeaderPair>() <<
            HeaderPair("Referer", "https://www.hsl.fi/") <<
            HeaderPair("Sec-Fetch-Dest", "document") <<
            HeaderPair("Sec-Fetch-Mode", "navigate") <<
            HeaderPair("Sec-Fetch-Site", "same-site") <<
            HeaderPair("Sec-Fetch-User", "?1") <<
            HeaderPair("Priority", "u=0, i") <<
            HeaderPair("TE", "trailers") <<
            HeaderPair("Upgrade-Insecure-Requests", "1")),
            SIGNAL(finished()), SLOT(onGetAuthUiFinished()));
    } else {
        Q_EMIT owner->httpError(status);
    }
}

void
BikeLogin::Private::onGetAuthUiFinished()
{
    Reply reply(sender());
    BikeLogin* owner = parentObject();
    const int status = statusCode(reply);

    HDEBUG(qPrintable(toString(reply)));
    if (status == OK) {
        const QString curDate(QString::number(QDateTime::currentMSecsSinceEpoch()));
        const QString authUiUrl(QString("%1&v-%2").arg(iAuthUiUrl, curDate));

        TODO("extract wsver and appId")
        const QString appId("ROOT-2521314");
        iWsver = "8.27.3";

        owner->updateCookies(reply);
        connect(owner->post(authUiUrl, QList<HeaderPair>() <<
            HeaderPair("Origin", "https://id.hsl.fi") <<
            HeaderPair("Referer", iAuthUiUrl.toUtf8()) <<
            HeaderPair("Sec-Fetch-Dest", "empty") <<
            HeaderPair("Sec-Fetch-Mode", "cors") <<
            HeaderPair("Sec-Fetch-Site", "same-origin") <<
            HeaderPair("TE", "trailers"),
            "application/x-www-form-urlencoded",
            QString("v-browserDetails=1&v-sh=1440&v-sw=2560&v-cw=1702&v-ch=679&v-vw=1702&v-vh=0"
                "&theme=openid&v-appId=%1&v-loc=%2&v-wn=%1-1").
                arg(appId, QString::fromLatin1(QUrl::toPercentEncoding(authUiUrl))).toUtf8()),
            SIGNAL(finished()), SLOT(onPostAuthUiFinished()));
    } else {
        Q_EMIT owner->httpError(status);
    }
}

void
BikeLogin::Private::onPostAuthUiFinished()
{
    Reply reply(sender());
    BikeLogin* owner = parentObject();
    const int status = statusCode(reply);

    HDEBUG(qPrintable(toString(reply)));
    if (status == OK) {
        const QByteArray replyData(reply->readAll());

        // {"v-uiId":0,"uidl":"..."}
        HDEBUG(replyData.constData());
        const QJsonObject replyJson(QJsonDocument::fromJson(replyData).object());
        const QString uidlString(replyJson.value("uidl").toString());
        const QJsonObject uidl(QJsonDocument::fromJson(uidlString.toUtf8()).object());

        // Parse the reply
        iSyncId = uidl.value(syncIdKey).toInt();
        iClientId = uidl.value(clientIdKey).toInt();
        iCsrfToken = uidl.value("Vaadin-Security-Key").toString();
        iAuthUidlUrl = QString("https://id.hsl.fi/UIDL/?v-uiId=%1").
            arg(replyJson.value("v-uiId").toInt());
        iAuthUidlHeaders = QList<HeaderPair>() <<
            HeaderPair("Accept", "*/*") <<
            HeaderPair("Origin", "https://id.hsl.fi") <<
            HeaderPair("Referer", iAuthUiUrl.toUtf8()) <<
            HeaderPair("Sec-Fetch-Dest", "empty") <<
            HeaderPair("Sec-Fetch-Mode", "cors") <<
            HeaderPair("Sec-Fetch-Site", "same-origin") <<
            HeaderPair("TE", "trailers");

        // Find the states we are interested tin
        const QJsonObject state(uidl.value("state").toObject());
        QJsonObject::const_iterator stateEnd = state.constEnd();
        for (QJsonObject::const_iterator it = state.constBegin(); it != stateEnd; it++) {
            // We are iterating through something like this
            // "29": {
            //   "clickShortcutKeyCode": 13,
            //   "caption": "Kirjaudu",
            //   "styles": [
            //     "primary",
            //     "button-main",
            //     "login-button"
            //   ]
            // }
            const QString key(it.key());
            const QJsonArray styles(it.value().toObject().value("styles").toArray());
            const int n = styles.count();

            for (int i = 0; i < n; i++) {
                const QString style(styles.at(i).toString());

                if (style == QStringLiteral("login-username-field")) {
                    iUsernameField = key;
                } else if (style == QStringLiteral("password-hidden-field")) {
                    iPasswordField = key;
                } else if (style == QStringLiteral("login-button")) {
                    iLoginButton = key;
                }
            }
        }

        HDEBUG("csrfToken" << iCsrfToken << "usernameField" << iUsernameField <<
            "passwordField" << iPasswordField << "loginButton" << iLoginButton);

        // Request:
        //
        // {
        //   "csrfToken": "xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx",
        //   "rpc": [
        //     [
        //       "0",
        //       "com.vaadin.shared.ui.DelayedCallbackRpc",
        //       "received",
        //       [
        //         0
        //       ]
        //     ]
        //   ],
        //   "syncId": 0,
        //   "clientId": 0,
        //   "wsver": "8.27.3"
        // }
        //
        // Response:
        //
        // [
        //   {
        //     "syncId": 1,
        //     "clientId": 1,
        //     "changes": [],
        //     "state": {},
        //     "types": {},
        //     "hierarchy": {},
        //     "rpc": [],
        //     "meta": {},
        //     "resources": {}
        //   }
        // ]
        owner->updateCookies(reply);
        connect(owner->post(iAuthUidlUrl, iAuthUidlHeaders, JsonContentType,
            QJsonDocument(QJsonObject{
               { "wsver", iWsver },
               { csrfTokenKey, iCsrfToken },
               { syncIdKey, iSyncId },
               { clientIdKey, iClientId },
               { rpcKey, QJsonArray{
                    QJsonArray{
                        "0",
                        "com.vaadin.shared.ui.DelayedCallbackRpc",
                        "received",
                        QJsonArray{0}
                    }}
               }
            }).toJson(QJsonDocument::Compact)), SIGNAL(finished()),
            SLOT(onPostDelayedCallbackRpcReceivedFinished()));
    } else {
        Q_EMIT owner->httpError(status);
    }
}

void
BikeLogin::Private::onPostDelayedCallbackRpcReceivedFinished()
{
    Reply reply(sender());
    BikeLogin* owner = parentObject();
    const int status = statusCode(reply);

    HDEBUG(qPrintable(toString(reply)));
    if (status == OK) {
        const QByteArray replyData(reply->readAll());
        const QJsonObject replyJson(replyToJsonObject(replyData));

        iSyncId = replyJson.value(syncIdKey).toInt(iSyncId + 1);
        iClientId = replyJson.value(clientIdKey).toInt(iClientId + 1);

        // {
        //   "csrfToken": "xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx",
        //   "rpc": [
        //     [
        //       "0",
        //       "com.vaadin.shared.ui.ui.UIServerRpc",
        //       "resize",
        //       [
        //         1700,
        //         680,
        //         1700,
        //         680
        //       ]
        //     ]
        //   ],
        //   "syncId": 1,
        //   "clientId": 1
        // }
        owner->updateCookies(reply);
        connect(owner->post(iAuthUidlUrl, iAuthUidlHeaders, JsonContentType,
            QJsonDocument(QJsonObject{
               { csrfTokenKey, iCsrfToken },
               { syncIdKey, iSyncId },
               { clientIdKey, iClientId },
               { rpcKey, QJsonArray{
                    QJsonArray{
                        "0",
                        "com.vaadin.shared.ui.ui.UIServerRpc",
                        "resize",
                        QJsonArray{1700, 680, 1700, 680}
                    }}
               }
            }).toJson(QJsonDocument::Compact)), SIGNAL(finished()),
            SLOT(onPostUIServerRpcResizeFinished()));
    } else {
        Q_EMIT owner->httpError(status);
    }
}

void
BikeLogin::Private::onPostUIServerRpcResizeFinished()
{
    Reply reply(sender());
    BikeLogin* owner = parentObject();
    const int status = statusCode(reply);

    HDEBUG(qPrintable(toString(reply)));
    if (status == OK) {
        const QByteArray replyData(reply->readAll());
        const QJsonObject replyJson(replyToJsonObject(replyData));

        iSyncId = replyJson.value(syncIdKey).toInt(iSyncId + 1);
        iClientId = replyJson.value(clientIdKey).toInt(iClientId + 1);

        // {
        //   "csrfToken": "xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx",
        //   "rpc": [
        //     [
        //       "21",
        //       "com.vaadin.shared.ui.textfield.AbstractTextFieldServerRpc",
        //       "setText",
        //       [
        //         "xxxxxxxx",
        //         8 <= length of the above
        //       ]
        //     ]
        //   ],
        //   "syncId": 1,
        //   "clientId": 1
        // }
        owner->updateCookies(reply);
        connect(owner->post(iAuthUidlUrl, iAuthUidlHeaders, JsonContentType,
            QJsonDocument(QJsonObject{
               { csrfTokenKey, iCsrfToken },
               { syncIdKey, iSyncId },
               { clientIdKey, iClientId },
               { rpcKey, QJsonArray{
                    QJsonArray{
                        iPasswordField,
                        "com.vaadin.shared.ui.textfield.AbstractTextFieldServerRpc",
                        "setText",
                        QJsonArray{iPassword, iPassword.length()}
                    }}
               }
            }).toJson(QJsonDocument::Compact)), SIGNAL(finished()),
            SLOT(onPostAbstractTextFieldServerRpcSetPasswordFinished()));
    } else {
        Q_EMIT owner->httpError(status);
    }
}

void
BikeLogin::Private::onPostAbstractTextFieldServerRpcSetPasswordFinished()
{
    Reply reply(sender());
    BikeLogin* owner = parentObject();
    const int status = statusCode(reply);

    HDEBUG(qPrintable(toString(reply)));
    if (status == OK) {
        const QByteArray replyData(reply->readAll());
        const QJsonObject replyJson(replyToJsonObject(replyData));

        iSyncId = replyJson.value(syncIdKey).toInt(iSyncId + 1);
        iClientId = replyJson.value(clientIdKey).toInt(iClientId + 1);

        // {
        //   "csrfToken": "xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx",
        //   "rpc": [
        //     [
        //       "17",
        //       "com.vaadin.shared.ui.textfield.AbstractTextFieldServerRpc",
        //       "setText",
        //       [
        //         "xxxxxxxx",
        //         8 <= length of the above
        //       ]
        //     ]
        //   ],
        //   "syncId": 2,
        //   "clientId": 2
        // }
        owner->updateCookies(reply);
        connect(owner->post(iAuthUidlUrl, iAuthUidlHeaders, JsonContentType,
            QJsonDocument(QJsonObject{
               { csrfTokenKey, iCsrfToken },
               { syncIdKey, iSyncId },
               { clientIdKey, iClientId },
               { rpcKey, QJsonArray{
                    QJsonArray{
                         iUsernameField,
                         "com.vaadin.shared.ui.textfield.AbstractTextFieldServerRpc",
                         "setText",
                         QJsonArray{iLogin, iLogin.length()}
                    }}
               }
            }).toJson(QJsonDocument::Compact)), SIGNAL(finished()),
            SLOT(onPostAbstractTextFieldServerRpcSetUsernameFinished()));
    } else {
        Q_EMIT owner->httpError(status);
    }
}

void
BikeLogin::Private::onPostAbstractTextFieldServerRpcSetUsernameFinished()
{
    Reply reply(sender());
    BikeLogin* owner = parentObject();
    const int status = statusCode(reply);

    HDEBUG(qPrintable(toString(reply)));
    if (statusCode(reply) == OK) {
        const QByteArray replyData(reply->readAll());
        const QJsonObject replyJson(replyToJsonObject(replyData));

        iSyncId = replyJson.value(syncIdKey).toInt(iSyncId + 1);
        iClientId = replyJson.value(clientIdKey).toInt(iClientId + 1);

        // {
        //   "csrfToken": "xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx",
        //   "rpc": [
        //     [
        //       "29",
        //       "com.vaadin.shared.ui.button.ButtonServerRpc",
        //       "click",
        //       [
        //         {
        //           "altKey": false,
        //           "button": "LEFT",
        //           "clientX": 832,
        //           "clientY": 480,
        //           "ctrlKey": false,
        //           "metaKey": false,
        //           "relativeX": 94,
        //           "relativeY": 45,
        //           "shiftKey": false,
        //           "type": 1
        //         }
        //       ]
        //     ]
        //   ],
        //   "syncId": 3,
        //   "clientId": 3
        // }
        owner->updateCookies(reply);
        connect(owner->post(iAuthUidlUrl, iAuthUidlHeaders, JsonContentType,
            QJsonDocument(QJsonObject{
               { csrfTokenKey, iCsrfToken },
               { syncIdKey, iSyncId },
               { clientIdKey, iClientId },
               { rpcKey, QJsonArray{
                    QJsonArray{
                        iLoginButton,
                        "com.vaadin.shared.ui.button.ButtonServerRpc",
                        "click",
                        QJsonArray{ QJsonObject{
                            { "altKey", false },
                            { "button", "LEFT" },
                            { "clientX", 832 },
                            { "clientY", 480 },
                            { "ctrlKey", false },
                            { "metaKey", false },
                            { "relativeX", 94 },
                            { "relativeY", 45 },
                            { "shiftKey", false },
                            { "type", 1 }
                        }}
                    }}
               }
            }).toJson(QJsonDocument::Compact)), SIGNAL(finished()),
            SLOT(onPostButtonServerRpcClickFinished()));
    } else {
        Q_EMIT owner->httpError(status);
    }
}

void
BikeLogin::Private::onPostButtonServerRpcClickFinished()
{
    Reply reply(sender());
    BikeLogin* owner = parentObject();
    const int status = statusCode(reply);

    HDEBUG(qPrintable(toString(reply)));
    if (status == OK) {
        const QByteArray replyData(reply->readAll());
        const QJsonObject replyJson(replyToJsonObject(replyData));
        const QJsonArray changes(replyJson.value("changes").toArray());

        owner->updateCookies(reply);

        // Looking for something like this on success:
        //
        // {
        //   "changes": [
        //     [
        //       "change",
        //       {
        //         "pid": "0"
        //       },
        //       [
        //         "0",
        //         {
        //           "id": "0",
        //           "v": {
        //             "action": ""
        //           }
        //         },
        //         [
        //           "open",
        //           {
        //             "src": "https://id.hsl.fi/openid/post-login-redirect?t=xxxxxxxxxxxxxxxx",
        //             "name": "_self",
        //             "popup": false
        //           }
        //         ],
        //         [
        //           "actions",
        //           {}
        //         ]
        //       ]
        //     ]
        //   ],
        //   ...
        // }
        //

        const int n = changes.size();
        for (int i = 0; i < n; i++) {
            QJsonArray change = changes.at(i).toArray();
            if (change.size() > 2 && change.at(0).toString() == "change") {
                change = change.at(2).toArray();
                if (change.size() > 2 && change.at(0).toString() == "0") {
                    change = change.at(2).toArray();
                    if (change.size() > 1 && change.at(0).toString() == "open") {
                        connect(owner->get(change.at(1).toObject().value("src").toString(),
                            QList<HeaderPair>() <<
                            HeaderPair("Sec-Fetch-Dest", "document") <<
                            HeaderPair("Sec-Fetch-Mode", "navigate") <<
                            HeaderPair("Sec-Fetch-Site", "same-origin") <<
                            HeaderPair("Sec-Fetch-User", "?1") <<
                            HeaderPair("Priority", "u=0, i") <<
                            HeaderPair("TE", "trailers") <<
                            HeaderPair("Upgrade-Insecure-Requests", "1")),
                            SIGNAL(finished()),
                            SLOT(onGetAuthRedirectFinished()));
                        return;
                    }
                }
            }
        }

        // or something like this on failure:
        //
        // {
        //   "syncId": 5,
        //   "clientId": 5,
        //   "changes": [],
        //   "state": {
        //     ...
        //     "25": {
        //       "text": "Sisäänkirjautuminen epäonnistui! ...",
        //       "styles": [
        //         "failure"
        //       ]
        //     }
        //   },
        //   "types": {
        //     ...
        //   },
        //   "hierarchy": {
        //     ...
        //   },
        //   "rpc": [],
        //   "meta": {},
        //   "resources": {}
        // }

        QString errorMessage;
        const QJsonObject state(replyJson.value("state").toObject());
        QJsonObject::const_iterator stateEnd = state.constEnd();
        for (QJsonObject::const_iterator it = state.constBegin(); it != stateEnd; it++) {
            const QString key(it.key());
            const QJsonObject value(it.value().toObject());
            const QJsonArray styles(value.value("styles").toArray());
            const int n = styles.count();

            for (int i = 0; i < n; i++) {
                const QString style(styles.at(i).toString());

                if (style == QStringLiteral("failure")) {
                    errorMessage = value.value("text").toString();
                    break;
                }
            }
        }
        Q_EMIT owner->failure(errorMessage);
    } else {
        Q_EMIT owner->httpError(status);
    }
}

void
BikeLogin::Private::onGetAuthRedirectFinished()
{
    Reply reply(sender());
    BikeLogin* owner = parentObject();
    const int status = statusCode(reply);

    HDEBUG(qPrintable(toString(reply)));
    if (status == OK) {
        const QString redirectHtml(reply->readAll());
        const int start = redirectHtml.indexOf("https://www.hsl.fi/user/auth/hslid?");

        owner->updateCookies(reply);
        HDEBUG(qPrintable(redirectHtml));
        if (start > 0) {
            const QChar delimiter(redirectHtml.at(start - 1));
            const int end = redirectHtml.indexOf(delimiter, start);
            if (end > 0) {
                // The response to this request will send us the hslid= cookie
                // that we have been looking for
                connect(owner->get(redirectHtml.mid(start, end - start),
                    QList<HeaderPair>() <<
                    HeaderPair("Sec-Fetch-Dest", "document") <<
                    HeaderPair("Sec-Fetch-Mode", "navigate") <<
                    HeaderPair("Sec-Fetch-Site", "same-origin") <<
                    HeaderPair("Sec-Fetch-User", "?1") <<
                    HeaderPair("Priority", "u=0, i") <<
                    HeaderPair("TE", "trailers") <<
                    HeaderPair("Upgrade-Insecure-Requests", "1")),
                    SIGNAL(finished()),
                    SLOT(onGetHslidFinished()));
                return;
            }
        }
    }
    Q_EMIT owner->httpError(status);
}

void
BikeLogin::Private::onGetHslidFinished()
{
    Reply reply(sender());
    BikeLogin* owner = parentObject();
    const int status = statusCode(reply);

    // Normally we get 302 (Found) here which redirects us to
    // https://www.hsl.fi//omat-tiedot/kaupunkipyorat/matkahistoria?fromLogin=true
    // but we don't follow the regirect
    HDEBUG(qPrintable(toString(reply)));
    if (status == OK || status == Found) {
        owner->updateCookies(reply);
        BikeUserQuery* query = new BikeUserQuery(owner);
        connect(query, SIGNAL(networkError()), owner, SIGNAL(networkError()));
        connect(query, SIGNAL(httpError(int)), owner, SIGNAL(httpError(int)));
        connect(query, SIGNAL(finished(QJsonObject)), owner, SIGNAL(success(QJsonObject)));
        connect(query, SIGNAL(done()), owner, SIGNAL(done()));
        connect(query, SIGNAL(done()), query, SLOT(deleteLater())); // And self-destruct
        return;
    }
    Q_EMIT owner->httpError(status);
}

// ==========================================================================
// BikeLogin
// ==========================================================================

BikeLogin::BikeLogin(
    QNetworkAccessManager* aParent,
    QString aLogin,
    QString aPassword) :
    BikeRequest(aParent),
    iPrivate(new Private(this, aLogin, aPassword))
{
    QNetworkReply* reply = get("https://www.hsl.fi/user/auth/login?language=en",  QList<HeaderPair>() <<
        HeaderPair("Referer", "https://www.hsl.fi/omat-tiedot/kaupunkipyorat/matkahistoria") <<
        HeaderPair("Sec-Fetch-Dest", "document") <<
        HeaderPair("Sec-Fetch-Mode", "navigate") <<
        HeaderPair("Sec-Fetch-Site", "same-origin") <<
        HeaderPair("Sec-Fetch-User", "?1") <<
        HeaderPair("Priority", "u=0, i") <<
        HeaderPair("TE", "trailers") <<
        HeaderPair("Upgrade-Insecure-Requests", "1"));

    iPrivate->connect(reply, SIGNAL(finished()), SLOT(onGetLoginFinished()));
}

#include "BikeLogin.moc"
