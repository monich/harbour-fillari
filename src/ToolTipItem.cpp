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

#include "ToolTipItem.h"

#include <QtGui/QBrush>
#include <QtGui/QPainter>
#include <QtGui/QPainterPath>
#include <QtGui/QPen>

#include "HarbourDebug.h"

// ==========================================================================
// ToolTipItem::Private
// ==========================================================================

class ToolTipItem::Private
{
public:
    Private();

    void paint(QPainter*, const QRectF&);

public:
    QColor iBackgroundColor;
    QColor iBorderColor;
    qreal iBorderWidth;
    qreal iBottomMargin;
    qreal iRadius;
};

ToolTipItem::Private::Private() :
    iBackgroundColor(Qt::white),
    iBorderColor(Qt::black),
    iBorderWidth(1),
    iBottomMargin(0),
    iRadius(0)
{}

void
ToolTipItem::Private::paint(
    QPainter* aPainter,
    const QRectF& aRect)
{
    const qreal d = 2 * iRadius;
    const qreal w2 = iBorderWidth/2;
    const qreal dw2 = d + w2;
    const qreal y1 = aRect.top();
    const qreal y2 = aRect.bottom() - iBottomMargin;
    const qreal x1 = aRect.left();
    const qreal x2 = aRect.right();
    QPainterPath path;

    // bottom right
    path.moveTo(x2 - dw2, y2 - w2);
    if (iRadius > 0) {
        path.arcTo(QRectF(x2 - dw2, y2 - dw2, d, d), 270, 90);
    }

    // top right
    path.lineTo(x2 - w2, y1 + dw2);
    if (iRadius > 0) {
        path.arcTo(QRectF(x2 - dw2, y1 + w2, d, d), 0, 90);
    }

    // top left
    path.lineTo(x1 + dw2, y1 + w2);
    if (iRadius > 0) {
        path.arcTo(QRectF(x1 + w2, y1 + w2, d, d), 90, 90);
    }

    // bottom left
    path.lineTo(x1 + w2, y2 - dw2);
    if (iRadius > 0) {
        path.arcTo(QRectF(x1 + w2, y2 - dw2, d, d), 180, 90);
    }

    // bottom triangle
    if (iBottomMargin > 0) {
        const qreal x3 = (x1 + x2) / 2;

        path.lineTo(x3 - iBottomMargin, y2 - w2);
        path.lineTo(x3, aRect.bottom() - w2);
        path.lineTo(x3 + iBottomMargin, y2 - w2);
    }
    path.lineTo(x2 - dw2, y2 - w2);

    aPainter->setRenderHint(QPainter::Antialiasing);
    aPainter->setBrush(QBrush(iBackgroundColor));
    aPainter->setPen(QPen(iBorderColor, iBorderWidth));
    aPainter->drawPath(path);
}

// ==========================================================================
// ToolTipItem
// ==========================================================================

ToolTipItem::ToolTipItem(
    QQuickItem* aParent) :
    QQuickPaintedItem(aParent),
    iPrivate(new Private())
{
}

ToolTipItem::~ToolTipItem()
{
    delete iPrivate;
}

QColor
ToolTipItem::backgroundColor() const
{
    return iPrivate->iBackgroundColor;
}

void
ToolTipItem::setBackgroundColor(
    const QColor& aColor)
{
    const QColor rgb(aColor.toRgb());
    if (iPrivate->iBackgroundColor.toRgb() != rgb) {
        iPrivate->iBackgroundColor = rgb;
        HDEBUG(rgb);
        Q_EMIT backgroundColorChanged();
        update();
    }
}

QColor
ToolTipItem::borderColor() const
{
    return iPrivate->iBorderColor;
}

void
ToolTipItem::setBorderColor(
    const QColor& aColor)
{
    const QColor rgb(aColor.toRgb());
    if (iPrivate->iBorderColor.toRgb() != rgb) {
        iPrivate->iBorderColor = rgb;
        HDEBUG(rgb);
        Q_EMIT borderColorChanged();
        update();
    }
}

qreal
ToolTipItem::borderWidth() const
{
    return iPrivate->iBorderWidth;
}

void
ToolTipItem::setBorderWidth(
    qreal aWidth)
{
    if (iPrivate->iBorderWidth != aWidth) {
        iPrivate->iBorderWidth = aWidth;
        HDEBUG(aWidth);
        Q_EMIT borderWidthChanged();
        update();
    }
}

qreal
ToolTipItem::bottomMargin() const
{
    return iPrivate->iBottomMargin;
}

void
ToolTipItem::setBottomMargin(
    qreal aMargin)
{
    if (iPrivate->iBottomMargin != aMargin) {
        iPrivate->iBottomMargin = aMargin;
        HDEBUG(aMargin);
        Q_EMIT bottomMarginChanged();
        update();
    }
}

qreal
ToolTipItem::radius() const
{
    return iPrivate->iRadius;
}

void
ToolTipItem::setRadius(
    qreal aRadius)
{
    if (iPrivate->iRadius != aRadius) {
        iPrivate->iRadius = aRadius;
        HDEBUG(aRadius);
        Q_EMIT radiusChanged();
        update();
    }
}

void
ToolTipItem::paint(
    QPainter* aPainter)
{
    HDEBUG(boundingRect());
    aPainter->save();
    iPrivate->paint(aPainter, boundingRect());
    aPainter->restore();
}
