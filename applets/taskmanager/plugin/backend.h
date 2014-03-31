/***************************************************************************
 *   Copyright (C) 2013 by Eike Hein <hein@kde.org>                        *
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

#ifndef BACKEND_H
#define BACKEND_H

#include <QObject>

#include <groupmanager.h>
#include <tasksmodel.h>

class QQuickItem;

class Backend : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QObject* groupManager READ groupManager CONSTANT)
    Q_PROPERTY(QObject* tasksModel READ tasksModel CONSTANT)
    Q_PROPERTY(QQuickItem* taskManagerItem READ taskManagerItem WRITE setTaskManagerItem NOTIFY taskManagerItemChanged)
    Q_PROPERTY(bool anyTaskNeedsAttention READ anyTaskNeedsAttention)
    Q_PROPERTY(bool highlightWindows READ highlightWindows WRITE setHighlightWindows NOTIFY highlightWindowsChanged)
    Q_PROPERTY(int groupingStrategy READ groupingStrategy WRITE setGroupingStrategy)
    Q_PROPERTY(int sortingStrategy READ sortingStrategy WRITE setSortingStrategy)
    Q_PROPERTY(QString launchers READ launchers WRITE setLaunchers NOTIFY launchersChanged)

    public:
        Backend(QObject *parent = 0);
        ~Backend();

        TaskManager::GroupManager *groupManager() const;
        TaskManager::TasksModel *tasksModel() const;

        QQuickItem* taskManagerItem() const;
        void setTaskManagerItem(QQuickItem *item);

        bool anyTaskNeedsAttention() const;

        bool highlightWindows() const;
        void setHighlightWindows(bool highlight);

        int groupingStrategy() const;
        void setGroupingStrategy(int groupingStrategy);

        int sortingStrategy() const;
        void setSortingStrategy(int sortingStrategy);

        QString launchers() const;
        void setLaunchers(const QString& launchers);

    public Q_SLOTS:
        void activateItem(int id, bool toggle);
        void itemContextMenu(QQuickItem *item, QObject *configAction);
        void itemHovered(int id, bool hovered);
        void itemMove(int id, int newIndex);
        void itemGeometryChanged(QQuickItem *item, int id);

    private Q_SLOTS:
        void updateLaunchersCache();

    Q_SIGNALS:
        void taskManagerItemChanged(QQuickItem*);
        void highlightWindowsChanged(bool);
        void launchersChanged();

    private:
        TaskManager::GroupManager *m_groupManager;
        TaskManager::TasksModel *m_tasksModel;
        QQuickItem* m_taskManagerItem;
        WId m_lastWindowId;
        bool m_highlightWindows;
        QString m_launchers;
};

#endif
