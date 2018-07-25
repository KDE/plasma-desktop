/***************************************************************************
 *   Copyright (C) 2014 by Eike Hein <hein@kde.org>                        *
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

#include "windowsystem.h"

#include <QQuickItem>
#include <QQuickWindow>

#include <KWindowSystem>

WindowSystem::WindowSystem(QObject *parent) : QObject(parent)
{
}

WindowSystem::~WindowSystem()
{
}

bool WindowSystem::eventFilter(QObject* watched, QEvent* event)
{
    if (event->type() == QEvent::FocusIn) {
        removeEventFilter(watched);
        emit focusIn(qobject_cast<QQuickWindow *>(watched));
    }

    return false;
}

void WindowSystem::forceActive(QQuickItem *item)
{
    if (!item || !item->window()) {
        return;
    }

    KWindowSystem::forceActiveWindow(item->window()->winId());
    KWindowSystem::raiseWindow(item->window()->winId());
}

bool WindowSystem::isActive(QQuickItem *item)
{
    if (!item || !item->window()) {
        return false;
    }

    return item->window()->isActive();
}

void WindowSystem::monitorWindowFocus(QQuickItem* item)
{
    if (!item || !item->window()) {
        return;
    }

    item->window()->installEventFilter(this);
}

void WindowSystem::monitorWindowVisibility(QQuickItem* item)
{
    if (!item || !item->window()) {
        return;
    }

    connect(item->window(), &QQuickWindow::visibilityChanged, this,
        &WindowSystem::monitoredWindowVisibilityChanged, Qt::UniqueConnection);
}

void WindowSystem::monitoredWindowVisibilityChanged(QWindow::Visibility visibility) const
{
    bool visible = (visibility != QWindow::Hidden);
    QQuickWindow *w = static_cast<QQuickWindow *>(QObject::sender());

    if (!visible) {
        emit hidden(w);
    }
}
