/***************************************************************************
 *   Copyright (C) 2007 Jani Huhtanen <jani.huhtanen@tut.fi>               *
 *   Copyright (C) 2007 by Robert Knight <robertknight@gmail.com>          *
 *   Copyright (C) 2008 by Alexis Ménard <darktears31@gmail.com>           *
 *   Copyright (C) 2008 by Marco Martin <notmart@gmail.com>                *
 *   Copyright (C) 2012-2013 by Eike Hein <hein@kde.org>                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include "textlabel.h"
#include "blur.cpp"

#include <QApplication>
#include <QPainter>
#include <QStyle>

TextLabel::TextLabel(QQuickItem* parent) : QQuickPaintedItem(parent),
    m_enabled(true),
    m_elide(false)
{
}

TextLabel::~TextLabel()
{
}

bool TextLabel::enabled() const
{
    return m_enabled;
}

void TextLabel::setEnabled(bool enabled)
{
    if (enabled != m_enabled) {
        m_enabled = enabled;
        update();
    }
}

QString TextLabel::text() const
{
    return m_text;
}

void TextLabel::setText(const QString& text)
{
    if (text != m_text)
    {
        m_text = text;
        m_cachedShadow = QPixmap();
        updateImplicitSize();
        update();
        emit textChanged(text);
    }
}

QColor TextLabel::color() const
{
    return m_color;
}

void TextLabel::setColor(const QColor& color)
{
    if (color != m_color)
    {
        m_color = color;
        m_cachedShadow = QPixmap();
        update();
    }
}


bool TextLabel::elide() const
{
    return m_elide;
}

void TextLabel::setElide(bool elide)
{
    m_elide = elide;

    updateImplicitSize();
}

void TextLabel::updateImplicitSize()
{
    if (m_elide) {
        setImplicitWidth(0);
        setImplicitHeight(0);
    } else {
        QFontMetrics fm(QFontDatabase::systemFont(QFontDatabase::GeneralFont));

        setImplicitWidth(fm.width(m_text));
        setImplicitHeight(fm.height());
    }
}

void TextLabel::geometryChanged(const QRectF& newGeometry, const QRectF& oldGeometry)
{
    if (newGeometry.size() != oldGeometry.size()) {
        m_cachedShadow = QPixmap();
    }
}

QColor TextLabel::textColor() const
{
    QColor color(m_color);

    if (!m_enabled) {
        color.setAlphaF(0.5);
    }

    return color;
}

QTextOption TextLabel::textOption() const
{
    Qt::LayoutDirection direction = QApplication::layoutDirection();
    Qt::Alignment alignment = QStyle::visualAlignment(direction, Qt::AlignLeft | Qt::AlignVCenter);

    QTextOption option;
    option.setTextDirection(direction);
    option.setAlignment(alignment);

    return option;
}

void TextLabel::layoutText(QTextLayout &layout, const QString &text, const QSize &constraints)
{
    QFontMetrics metrics(layout.font());
    int leading = metrics.leading();
    int height = 0;
    int maxWidth = constraints.width();
    int widthUsed = 0;
    int lineSpacing = metrics.lineSpacing();
    QTextLine line;

    layout.setText(text);

    layout.beginLayout();
    while ((line = layout.createLine()).isValid()) {
        height += leading;

        // Make the last line that will fit infinitely long.
        // drawTextLayout() will handle this by fading the line out
        // if it won't fit in the constraints.
        if (height + 2 * lineSpacing > constraints.height()) {
            line.setPosition(QPoint(0, height));
            break;
        }

        line.setLineWidth(maxWidth);
        line.setPosition(QPoint(0, height));

        height += int(line.height());
        widthUsed = int(qMax(qreal(widthUsed), line.naturalTextWidth()));
    }
    layout.endLayout();
}

void TextLabel::drawTextLayout(QPainter *painter, const QTextLayout &layout, const QRect &rect)
{
    if (rect.width() < 1 || rect.height() < 1) {
        return;
    }

    QPixmap pixmap(rect.size());
    pixmap.fill(Qt::transparent);

    QPainter p(&pixmap);
    p.setPen(painter->pen());

    // Create the alpha gradient for the fade out effect
    QLinearGradient alphaGradient(0, 0, 1, 0);
    alphaGradient.setCoordinateMode(QGradient::ObjectBoundingMode);
    if (layout.textOption().textDirection() == Qt::LeftToRight) {
        alphaGradient.setColorAt(0, QColor(0, 0, 0, 255));
        alphaGradient.setColorAt(1, QColor(0, 0, 0, 0));
    } else {
        alphaGradient.setColorAt(0, QColor(0, 0, 0, 0));
        alphaGradient.setColorAt(1, QColor(0, 0, 0, 255));
    }

    QFontMetrics fm(layout.font());
    int textHeight = layout.lineCount() * fm.lineSpacing();

    QPointF position(0, (rect.height() - textHeight) / 2);
    QList<QRect> fadeRects;
    int fadeWidth = 30;

    // Draw each line in the layout
    for (int i = 0; i < layout.lineCount(); i++) {
        QTextLine line = layout.lineAt(i);
        line.draw(&p, position);

        // Add a fade out rect to the list if the line is too long
        if (line.naturalTextWidth() > rect.width())
        {
            int x = int(qMin(line.naturalTextWidth(), (qreal)pixmap.width())) - fadeWidth;
            int y = int(line.position().y() + position.y());
            QRect r = QStyle::visualRect(layout.textOption().textDirection(), pixmap.rect(),
                                         QRect(x, y, fadeWidth, int(line.height())));
            fadeRects.append(r);
        }
    }

    // Reduce the alpha in each fade out rect using the alpha gradient
    if (!fadeRects.isEmpty()) {
        p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        foreach (const QRect &rect, fadeRects) {
            p.fillRect(rect, alphaGradient);
        }
    }

    p.end();


    QColor shadowColor;
    if (qGray(textColor().rgb()) > 192) {
        shadowColor = Qt::black;
    } else {
        shadowColor = Qt::white;
    }

    if (m_cachedShadow.isNull()) {
        QImage shadow = pixmap.toImage();
        expblur<16, 7>(shadow, 1);
        QPainter p(&shadow);
        p.setCompositionMode(QPainter::CompositionMode_SourceIn);
        p.fillRect(shadow.rect(), shadowColor);
        p.end();
        m_cachedShadow = QPixmap(shadow.size());
        m_cachedShadow.fill(Qt::transparent);
        QPainter buffPainter(&m_cachedShadow);
        buffPainter.drawImage(QPoint(0,0), shadow);
    }

    if (shadowColor == Qt::white) {
        painter->drawPixmap(rect.topLeft(), m_cachedShadow);
    } else {
        painter->drawPixmap(rect.topLeft() + QPoint(1, 2), m_cachedShadow);
    }
    painter->drawPixmap(rect.topLeft(), pixmap);
}

void TextLabel::paint(QPainter* painter)
{
    painter->setPen(QPen(textColor(), 1.0));

    if (QFontDatabase::systemFont(QFontDatabase::GeneralFont) != m_layout.font()) {
        m_cachedShadow = QPixmap();
    }

    m_layout.setFont(QFontDatabase::systemFont(QFontDatabase::GeneralFont));
    m_layout.setTextOption(textOption());

    layoutText(m_layout, text(), boundingRect().toRect().size());
    drawTextLayout(painter, m_layout, boundingRect().toRect());
}
