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

#include "menuhelper.h"

#include <QAction>
#include <QMenu>

MenuHelper::MenuHelper(QObject *parent) : QObject(parent)
{
}

MenuHelper::~MenuHelper()
{
}

QString MenuHelper::iconName(QAction *action) const
{
    if (!action) {
        return QString();
    }

    return action->icon().name();
}

void MenuHelper::setMenu(QAction* action, QObject* menu)
{
    QMenu* bla = qobject_cast<QMenu *>(menu);

    if (action && bla) {
        action->setMenu(bla);
    }
}
