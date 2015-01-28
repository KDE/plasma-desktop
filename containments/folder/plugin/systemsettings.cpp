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

#include <QApplication>
#include <QEvent>
#include <QStyle>
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
    return m_widget->style()->styleHint(QStyle::SH_ItemView_ActivateItemOnSingleClick);
}

int SystemSettings::doubleClickInterval() const
{
    if (!qApp) {
        return 400;
    }

    return qApp->doubleClickInterval();
}

bool SystemSettings::isDrag(int oldX, int oldY, int newX, int newY) const
{
    return ((QPoint(oldX, oldY) - QPoint(newX, newY)).manhattanLength() >= QApplication::startDragDistance());
}
