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

#ifndef BIKE_OBJECT_QUERY_H
#define BIKE_OBJECT_QUERY_H

#include "BikeRequest.h"

class QJsonObject;

// Query that receives a JSON object from the server
class BikeObjectQuery :
    public BikeRequest
{
    Q_OBJECT
    class Private;
    friend class Private;

public:
    BikeObjectQuery(QString, BikeRequest*);
    BikeObjectQuery(QString, QNetworkAccessManager*);

Q_SIGNALS:
    void finished(const QJsonObject&);

private Q_SLOTS:
    void onQueryFinished();
};

// On success, emits the "finished" signal with the JSON containing
// something like this:
//
// {
//   "authenticated": true,
//   "user": {
//     "id": "5b23d3826f45767f03d1473b",
//     "addresses": [
//       {
//         "streetAddress": "xxxxxxxxxxxxx",
//         "type": "PERMANENT",
//         "postalCode": "xxxxx",
//         "language": "fi",
//         "from": "yyyy-mm-dd",
//         "id": "xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx",
//       }
//     ],
//     "emails": [
//       {
//         "address": "xxx@xxxxxx",
//         "verified": true
//       }
//     ],
//     "mobiles": [
//       {
//         "number": "+358xxxxxxxxx",
//         "verified": true
//       }
//     ],
//     "name": {
//       "givenName": "xxx",
//       "familyName": "xxx"
//     },
//     "dateOfBirth": "yyyy-mm-dd",
//   }
//   ...
// }
//
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

class BikeUserQuery :
    public BikeObjectQuery
{
    Q_OBJECT
    static const QString URL;

public:
    BikeUserQuery(BikeRequest*);
    BikeUserQuery(QNetworkAccessManager*);
};

//
class BikeServiceQuery :
    public BikeObjectQuery
{
    Q_OBJECT
    static const QString URL;

public:
    BikeServiceQuery(QNetworkAccessManager*);
};


#endif // BIKE_OBJECT_QUERY_H
