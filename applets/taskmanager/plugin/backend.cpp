/***************************************************************************
 *   Copyright (C) 2007 by Robert Knight <robertknight@gmail.com>          *
 *   Copyright (C) 2008 by Alexis Ménard <darktears31@gmail.com>           *
 *   Copyright (C) 2012-2013 by Eike Hein <hein@kde.org>                   *
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

#include "backend.h"

#include <groupmanager.h>
#include <taskactions.h>
#include <taskgroup.h>
#include <tasksmodel.h>

#include <QCursor>
#include <QQuickItem>
#include <QQuickWindow>
#include <QScreen>
#include <QTimer>

#include <KAuthorized>
#include <kwindoweffects.h>

Backend::Backend(QObject* parent) : QObject(parent),
    m_groupManager(new TaskManager::GroupManager(this)),
    m_tasksModel(new TaskManager::TasksModel(m_groupManager, this)),
    m_contextMenu(0),
    m_taskManagerItem(0),
    m_toolTipItem(0),
    m_panelWinId(0),
    m_highlightWindows(false)
{
    connect(m_groupManager, SIGNAL(launcherListChanged()), this, SLOT(updateLaunchersCache()));
}

Backend::~Backend()
{
}

TaskManager::GroupManager* Backend::groupManager() const
{
    return m_groupManager;
}

TaskManager::TasksModel* Backend::tasksModel() const
{
    return m_tasksModel;
}

QQuickItem* Backend::taskManagerItem() const
{
    return m_taskManagerItem;
}

void Backend::setTaskManagerItem(QQuickItem* item)
{
    if (item != m_taskManagerItem) {
        m_taskManagerItem = item;
        emit taskManagerItemChanged(item);
    }
}

QQuickItem* Backend::toolTipItem() const
{
    return m_toolTipItem;
}

void Backend::setToolTipItem(QQuickItem* item)
{
    if (item != m_toolTipItem) {
        m_toolTipItem = item;

        connect(item, SIGNAL(windowChanged(QQuickWindow*)), this, SLOT(toolTipWindowChanged(QQuickWindow*)));

        emit toolTipItemChanged(item);
    }
}

void Backend::toolTipWindowChanged(QQuickWindow *window)
{
    Q_UNUSED(window)

    updateWindowHighlight();
}

bool Backend::anyTaskNeedsAttention() const
{
    return groupManager()->rootGroup()->demandsAttention();
}

bool Backend::highlightWindows() const
{
    return m_highlightWindows;
}

void Backend::setHighlightWindows(bool highlight)
{
    if (highlight != m_highlightWindows) {
        m_highlightWindows = highlight;

        updateWindowHighlight();

        emit highlightWindowsChanged(highlight);
    }
}

int Backend::groupingStrategy() const
{
    return m_groupManager->groupingStrategy();
}

void Backend::setGroupingStrategy(int groupingStrategy)
{
    // FIXME TODO: This is fucking terrible.
    m_groupManager->setGroupingStrategy(static_cast<TaskManager::GroupManager::TaskGroupingStrategy>(groupingStrategy));
}

int Backend::sortingStrategy() const
{
    return m_groupManager->sortingStrategy();
}

void Backend::setSortingStrategy(int sortingStrategy)
{
    // FIXME TODO: This is fucking terrible.
    m_groupManager->setSortingStrategy(static_cast<TaskManager::GroupManager::TaskSortingStrategy>(sortingStrategy));
}

void Backend::updateLaunchersCache()
{
    const QList<QUrl> launcherList = m_groupManager->launcherList();

    QStringList launchers;

    foreach(const QUrl& launcher, launcherList) {
        launchers.append(launcher.toString());
    }

    QString joined(launchers.join(','));

    if (joined != m_launchers) {
        m_launchers = joined;
        emit launchersChanged();
    }
}

QString Backend::launchers() const
{
    return m_launchers;
}

void Backend::setLaunchers(const QString& launchers)
{
    if (launchers == m_launchers) {
        return;
    }

    m_launchers = launchers;

    QList<QUrl> launcherList;

    foreach(const QString& launcher, launchers.split(',')) {
        launcherList.append(launcher);
    }

    disconnect(m_groupManager, SIGNAL(launcherListChanged()), this, SLOT(updateLaunchersCache()));
    m_groupManager->setLauncherList(launcherList);
    connect(m_groupManager, SIGNAL(launcherListChanged()), this, SLOT(updateLaunchersCache()));

    emit launchersChanged();
}

void Backend::activateItem(int id, bool toggle)
{
    TaskManager::AbstractGroupableItem* item = m_groupManager->rootGroup()->getMemberById(id);

    if (!item) {
        return;
    }

    if (item->itemType() == TaskManager::TaskItemType && !item->isStartupItem()) {
        TaskManager::TaskItem* taskItem = static_cast<TaskManager::TaskItem*>(item);

        if (toggle) {
            taskItem->task()->activateRaiseOrIconify();
        } else {
            taskItem->task()->activate();
        }
    } else if (item->itemType() == TaskManager::LauncherItemType) {
        static_cast<TaskManager::LauncherItem*>(item)->launch();
    }
}

void Backend::activateWindow(int winId)
{
    KWindowSystem::forceActiveWindow(winId);
}

void Backend::closeWindow(int winId)
{
    TaskManager::AbstractGroupableItem *item = m_groupManager->rootGroup()->getMemberByWId(winId);

    if (item) {
        item->close();
    }
}

void Backend::launchNewInstance(int id)
{
    TaskManager::AbstractGroupableItem* item = m_groupManager->rootGroup()->getMemberById(id);

    if (item) {
        item->launchNewInstance();
    }
}

void Backend::itemContextMenu(QQuickItem *item, QObject *configAction)
{
    TaskManager::AbstractGroupableItem* agItem = m_groupManager->rootGroup()->getMemberById(item->property("itemId").toInt());

    if (!KAuthorized::authorizeKAction("kwin_rmb") || !item || !item->window() || !agItem) {
        return;
    }

    QList <QAction*> actionList;

    QAction *action = static_cast<QAction *>(configAction);

    if (action && action->isEnabled()) {
        actionList << action;
    }

    if (agItem->itemType() == TaskManager::TaskItemType && !agItem->isStartupItem()) {
        TaskManager::TaskItem* taskItem = static_cast<TaskManager::TaskItem*>(agItem);
        m_contextMenu = new TaskManager::BasicMenu(0, taskItem, m_groupManager, actionList);
    } else if (agItem->itemType() == TaskManager::GroupItemType) {
        TaskManager::TaskGroup* taskGroup = static_cast<TaskManager::TaskGroup*>(agItem);
        const int maxWidth = 0.8 * item->window()->screen()->size().width();
        m_contextMenu = new TaskManager::BasicMenu(0, taskGroup, m_groupManager, actionList, QList <QAction*>(), maxWidth);
    } else if (agItem->itemType() == TaskManager::LauncherItemType) {
        m_contextMenu = new TaskManager::BasicMenu(0, static_cast<TaskManager::LauncherItem*>(agItem),
            m_groupManager, actionList);
    }

    if (!m_contextMenu) {
        return;
    }

    m_contextMenu->adjustSize();

    if (m_taskManagerItem && !m_taskManagerItem->property("vertical").toBool()) {
        m_contextMenu->setMinimumWidth(item->width());
    }

    // Close menu when the delegate is destroyed.
    connect(item, &QQuickItem::destroyed, m_contextMenu, &TaskManager::BasicMenu::deleteLater);

    // Interrupt the call chain from the delegate in case it's destroyed while
    // we're in QMenu::exec();
    // FIXME TODO: Just use a lambda on Qt 5.4+.
    QMetaObject::invokeMethod(this, "actuallyOpenContextMenu", Qt::QueuedConnection);
}

void Backend::actuallyOpenContextMenu()
{
    m_contextMenu->exec(QCursor::pos());
    m_contextMenu = 0;
    m_contextMenu->deleteLater();
}

void Backend::itemHovered(int id, bool hovered)
{
    m_windowsToHighlight.clear();

    TaskManager::AbstractGroupableItem* item = m_groupManager->rootGroup()->getMemberById(id);

    if (item && hovered) {
        m_windowsToHighlight = QList<WId>::fromSet(item->winIds());
    }

    updateWindowHighlight();
}

void Backend::windowHovered(int winId, bool hovered)
{
    m_windowsToHighlight.clear();

    if (hovered) {
        m_windowsToHighlight.append(winId);
    }

    updateWindowHighlight();
}

void Backend::updateWindowHighlight()
{
    if (!m_highlightWindows) {
        if (m_panelWinId) {
            KWindowEffects::highlightWindows(m_panelWinId, QList<WId>());

            m_panelWinId = 0;
        }

        return;
    }

    if (m_taskManagerItem && m_taskManagerItem->window()) {
        m_panelWinId = m_taskManagerItem->window()->winId();
    } else {
        return;
    }

    QList<WId> windows = m_windowsToHighlight;

    if (windows.count() && m_toolTipItem && m_toolTipItem->window()) {
        windows.append(m_toolTipItem->window()->winId());
    }

    KWindowEffects::highlightWindows(m_panelWinId, windows);
}

void Backend::itemMove(int id, int newIndex)
{
    m_groupManager->manualSortingRequest(m_groupManager->rootGroup()->getMemberById(id), newIndex);
}

void Backend::itemGeometryChanged(QQuickItem *item, int id)
{
    TaskManager:: AbstractGroupableItem *agItem = m_groupManager->rootGroup()->getMemberById(id);

    if (!item || !item->window() || !agItem || agItem->itemType() != TaskManager::TaskItemType)
    {
        return;
    }

    TaskManager::TaskItem *taskItem = static_cast<TaskManager::TaskItem *>(agItem);

    if (!taskItem->task()) {
        return;
    }

    QRect iconRect(item->x(), item->y(), item->width(), item->height());
    iconRect.moveTopLeft(item->parentItem()->mapToScene(iconRect.topLeft()).toPoint());
    iconRect.moveTopLeft(item->window()->mapToGlobal(iconRect.topLeft()));

    taskItem->task()->publishIconGeometry(iconRect);
}

bool Backend::canPresentWindows() const
{
    return (KWindowSystem::compositingActive() && KWindowEffects::isEffectAvailable(KWindowEffects::PresentWindowsGroup));
}

void Backend::presentWindows(int groupParentId)
{
    TaskManager::AbstractGroupableItem *item = m_groupManager->rootGroup()->getMemberById(groupParentId);

    if (item && m_taskManagerItem && m_taskManagerItem->window()) {
        KWindowEffects::presentWindows(m_taskManagerItem->window()->winId(), QList<WId>::fromSet(item->winIds()));
    }
}
