/***************************************************************************
 *   Copyright (C) 2014 by Eike Hein <hein@kde.org>                   *
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

#include "systemsettings.h"

#include <QGuiApplication>
#include <QStyle>
#include <QStyleHints>
#include <QWidget>

SystemSettings::SystemSettings(QObject *parent) : QObject(parent),
    m_widget(new QWidget())
{
    m_widget->resize(0, 0);
    m_widget->hide();
}

SystemSettings::~SystemSettings()
{
    delete m_widget;
}

bool SystemSettings::singleClick() const
{
    // FIXME TODO: Check back for whether this eventually got added to
    // QGuiApplication::styleHints() / Qt.styleHints.
    return m_widget->style()->styleHint(QStyle::SH_ItemView_ActivateItemOnSingleClick);
}

int SystemSettings::doubleClickInterval() const
{
    return QGuiApplication::styleHints()->mouseDoubleClickInterval();
}

bool SystemSettings::isDrag(int oldX, int oldY, int newX, int newY) const
{
    // FIXME TODO: QGuiApplication::styleHints() will be available as
    // Qt.styleHints in QML starting with Qt 5.5.
    return ((QPoint(oldX, oldY) - QPoint(newX, newY)).manhattanLength() >= QGuiApplication::styleHints()->startDragDistance());
}
