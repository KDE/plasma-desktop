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

#ifndef WINDOWSYSTEM_H
#define WINDOWSYSTEM_H

#include <QObject>
#include <QQuickWindow>
class QQuickItem;

class WindowSystem : public QObject
{
    Q_OBJECT

    public:
        explicit WindowSystem(QObject *parent = nullptr);
        ~WindowSystem() override;

        bool eventFilter(QObject *watched, QEvent *event) override;

        Q_INVOKABLE void forceActive(QQuickItem *item);

        Q_INVOKABLE bool isActive(QQuickItem *item);

        Q_INVOKABLE void monitorWindowFocus(QQuickItem *item);

        Q_INVOKABLE void monitorWindowVisibility(QQuickItem *item);

    Q_SIGNALS:
        void focusIn(QQuickWindow *window) const;
        void hidden(QQuickWindow *window) const;

    private Q_SLOTS:
        void monitoredWindowVisibilityChanged(QWindow::Visibility visibility) const;
};

#endif
