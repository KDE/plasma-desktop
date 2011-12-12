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

#ifndef PAINTUTILS_H
#define PAINTUTILS_H

#include "kimpanelsettings.h"

// Qt
#include <QApplication>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>

enum RenderType {
    Statusbar,
    Auxiliary,
    Preedit,
    TableLabel,
    TableEntry
};

QPixmap renderText(QString text, RenderType type = Statusbar, bool drawCursor = false, int cursorPos = 0, const QFont& font = KimpanelSettings::self()->font());
QPixmap renderText(QString text, QColor textColor, QColor bgColor, bool drawCursor, int cursorPos, const QFont &ft);

#endif // PAINTUTILS_H

