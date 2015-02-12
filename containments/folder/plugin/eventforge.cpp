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

#include "eventforge.h"

#include <QGuiApplication>
#include <QQuickItem>
#include <QQuickWindow>

EventForge::EventForge(QObject *parent) : QObject(parent)
{
}

EventForge::~EventForge()
{
}

void EventForge::sendLeftPress(QQuickItem *item, int x, int y)
{
    if (!item) {
        return;
    }

    QQuickWindow *win = item->window();

    if (!win) {
        return;
    }

    QMouseEvent ev(QEvent::MouseButtonPress, QPointF(x, y), Qt::LeftButton, Qt::LeftButton, 0);

    QGuiApplication::sendEvent(item, &ev);
}

void EventForge::sendUngrabRecursive(QQuickItem *parentItem)
{
    if (!parentItem) {
        return;
    }

    QQuickWindow *win = parentItem->window();

    if (!win) {
        return;
    }

    const QList<QQuickItem *> items = allChildItemsRecursive(parentItem);

    foreach(QQuickItem *item, items) {
        QEvent ev(QEvent::UngrabMouse);
        win->sendEvent(item, &ev);
    }
}

void EventForge::makeGrab(QQuickItem *item)
{
    if (!item) {
        return;
    }

    item->grabMouse();
}

QList<QQuickItem *> EventForge::allChildItemsRecursive(QQuickItem *parentItem)
{
     QList<QQuickItem *> itemList;

     itemList.append(parentItem->childItems());

     foreach(QQuickItem *childItem, parentItem->childItems()) {
         itemList.append(allChildItemsRecursive(childItem));
     }

     return itemList;
}
