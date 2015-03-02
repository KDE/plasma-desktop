/***************************************************************************
 *   Copyright (C) 2015 by Eike Hein <hein@kde.org>                   *
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

#ifndef EVENTFORGE_H
#define EVENTFORGE_H

#include <QObject>

class QQuickItem;

class EventForge : public QObject
{
    Q_OBJECT

    public:
        EventForge(QObject *parent = 0);
        ~EventForge();

        Q_INVOKABLE void sendLeftPress(QQuickItem *item, int x, int y);
        Q_INVOKABLE void sendUngrabRecursive(QQuickItem *parentItem);
        Q_INVOKABLE void makeGrab(QQuickItem *item);

    private:
        static QList<QQuickItem *> allChildItemsRecursive(QQuickItem *parentItem);
};

#endif
