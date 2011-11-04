/***************************************************************************
 *   Copyright (C) 2009 by Wang Hoi <zealot.hoi@gmail.com>                 *
 *   Copyright (C) 2011 by CSSlayer <wengxt@gmail.com>                     *
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

#include "paintutils.h"

// Plasma
#include <Plasma/PaintUtils>

QPixmap renderText(QString text, RenderType type, bool drawCursor, int cursorPos, const QFont& font)
{
    Plasma::Theme *theme = Plasma::Theme::defaultTheme();
    switch (type) {
    case Statusbar:
        return renderText(text, theme->color(Plasma::Theme::TextColor),
                          Qt::transparent, drawCursor, cursorPos, font);
    case Auxiliary:
        return renderText(text, theme->color(Plasma::Theme::TextColor),
                          Qt::transparent, drawCursor, cursorPos, font);
    case Preedit:
        return renderText(text, theme->color(Plasma::Theme::TextColor),
                          Qt::transparent, drawCursor, cursorPos, font);
    case TableLabel:
        return renderText(text, theme->color(Plasma::Theme::LinkColor),
                          Qt::transparent, drawCursor, cursorPos, font);
    case TableEntry:
        return renderText(text, theme->color(Plasma::Theme::TextColor),
                          Qt::transparent, drawCursor, cursorPos, font);
    default:
        return renderText(text, theme->color(Plasma::Theme::TextColor),
                          Qt::transparent, drawCursor, cursorPos, font);
    }
}

QPixmap renderText(QString text,
                   QColor textColor,
                   QColor bgColor,
                   bool drawCursor,
                   int cursorPos,
                   const QFont &ft)
{
    //don't try to paint stuff on a future null pixmap because the text is empty
    if (text.isEmpty()) {
        return QPixmap();
    }

    const qreal margin = 3;

    QFont font = ft;
    // Draw text
    QFontMetrics fm(font);
    QSize textSize = fm.size(0, text) + QSize(1, 0) + QSize(margin, margin);
    QPixmap textPixmap(textSize.width(), textSize.height());
    textPixmap.fill(bgColor);
    QPainter p(&textPixmap);
    p.setPen(textColor);
    p.setFont(font);
    // FIXME: the center alignment here is odd: the rect should be the size needed by
    //        the text, but for some fonts and configurations this is off by a pixel or so
    //        and "centering" the text painting 'fixes' that. Need to research why
    //        this is the case and determine if we should be painting it differently here,
    //        doing soething different with the boundingRect call or if it's a problem
    //        in Qt itself
    p.drawText(textPixmap.rect().translated(margin / 2, margin / 2), Qt::AlignCenter, text);

    if (drawCursor) {
        int pixelsWide = fm.width(text.left(cursorPos));
        p.translate(margin / 2 , margin / 2);
        p.drawLine(pixelsWide, 0, pixelsWide, fm.height());
    }

    p.end();
    return textPixmap;
}
