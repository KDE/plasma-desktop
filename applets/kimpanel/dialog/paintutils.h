/***************************************************************************
 *   Copyright (C) 2009 by Wang Hoi <zealot.hoi@gmail.com>                 *
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
#ifndef PAINTUTILS_H
#define PAINTUTILS_H

#include <plasma/theme.h>

#include <QApplication>
#include <QImage>
#include <QPainter>
#include <QPainterPath>
#include <QPaintEngine>
#include <QPixmap>

QPixmap renderText(QString text, const QFont &font = qApp->font(), QColor textColor = Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor))
{
    //don't try to paint stuff on a future null pixmap because the text is empty
    if (text.isEmpty()) {
        return QPixmap();
    }

    // Draw text
    QFontMetrics fm(font);
    QRect textRect = fm.boundingRect(text);
    QPixmap textPixmap(textRect.width(), fm.height());
    textPixmap.fill(Qt::transparent);
    QPainter p(&textPixmap);
    p.setPen(textColor);
    p.setFont(font);
    // FIXME: the center alignment here is odd: the rect should be the size needed by
    //        the text, but for some fonts and configurations this is off by a pixel or so
    //        and "centering" the text painting 'fixes' that. Need to research why
    //        this is the case and determine if we should be painting it differently here,
    //        doing soething different with the boundingRect call or if it's a problem
    //        in Qt itself
    p.drawText(textPixmap.rect(), Qt::AlignCenter, text);
    p.end();

    return textPixmap;
}
#endif // PAINTUTILS_H
