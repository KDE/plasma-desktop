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

    const qreal leftmargin = 3;
    const qreal rightmargin = 3;
    const qreal topmargin = 3;
    const qreal bottonmargin = 3;

    QFont font = ft;
    // Draw text
    QFontMetrics fm(font);
    QSize textSize = fm.size(Qt::TextSingleLine, text) + QSize(1, 0) + QSize(leftmargin + rightmargin, topmargin + bottonmargin);
    QPixmap textPixmap(textSize.width(), textSize.height());
    textPixmap.fill(bgColor);
    QPainter p(&textPixmap);
    p.setPen(textColor);
    p.setFont(font);
    p.drawText(leftmargin, topmargin + fm.ascent(), text);

    if (drawCursor) {
        int pixelsWide = fm.size(Qt::TextSingleLine, text.left(cursorPos)).width();
        p.drawLine(leftmargin + pixelsWide, topmargin, leftmargin + pixelsWide, topmargin + fm.height());
    }

    p.end();
    return textPixmap;
}
