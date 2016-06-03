/***************************************************************************
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

#ifndef TEXTLABEL_H
#define TEXTLABEL_H

#include <QQuickPaintedItem>
#include <QColor>
#include <QPixmap>
#include <QTextLayout>
#include <QTextOption>

class TextLabel : public QQuickPaintedItem
{
    Q_OBJECT

    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled)
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
    Q_PROPERTY(QColor color READ color WRITE setColor)
    Q_PROPERTY(bool elide READ elide WRITE setElide)

    public:
        TextLabel(QQuickItem *parent = 0);
        ~TextLabel();

        bool enabled() const;
        void setEnabled(bool enabled);

        QString text() const;
        void setText(const QString& text);

        QColor color() const;
        void setColor(const QColor& color);

        bool elide() const;
        void setElide(bool elide);

        void paint(QPainter *painter);

    Q_SIGNALS:
        void textChanged(const QString& text);

    protected:
        void geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry);

    private:
        void updateImplicitSize();
        QColor textColor() const;
        QTextOption textOption() const;;
        void layoutText(QTextLayout &layout, const QString &text,  const QSize &constraints);
        void drawTextLayout(QPainter *painter, const QTextLayout &layout, const QRect &rect);

        bool m_enabled;
        QString m_text;
        QColor m_color;
        bool m_elide;
        QTextLayout m_layout;
        QPixmap m_cachedShadow;
};

#endif
