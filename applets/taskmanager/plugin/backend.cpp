/***************************************************************************
 *   Copyright (C) 2012-2016 by Eike Hein <hein@kde.org>                   *
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

#include <KConfigGroup>
#include <KDesktopFile>
#include <KFileItem>
#include <KFilePlacesModel>
#include <KLocalizedString>
#include <KNotificationJobUiDelegate>
#include <KService>
#include <KServiceAction>
#include <KWindowEffects>
#include <KWindowSystem>

#include <KIO/ApplicationLauncherJob>

#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QJsonArray>
#include <QMenu>
#include <QScopedPointer>
#include <QQuickItem>
#include <QQuickWindow>
#include <QVersionNumber>
#include <QTimer>

#include <KActivities/Consumer>
#include <KActivities/Stats/Cleaning>
#include <KActivities/Stats/ResultSet>
#include <KActivities/Stats/Terms>

#include <processcore/process.h>
#include <processcore/processes.h>

namespace KAStats = KActivities::Stats;

using namespace KAStats;
using namespace KAStats::Terms;

Backend::Backend(QObject* parent) : QObject(parent)
    , m_panelWinId(0)
    , m_highlightWindows(false)
    , m_actionGroup(new QActionGroup(this))
{
}

Backend::~Backend()
{
}

QQuickItem *Backend::taskManagerItem() const
{
    return m_taskManagerItem;
}

void Backend::setTaskManagerItem(QQuickItem* item)
{
    if (item != m_taskManagerItem) {
        m_taskManagerItem = item;

        emit taskManagerItemChanged();
    }
}

QQuickItem *Backend::toolTipItem() const
{
    return m_toolTipItem;
}

void Backend::setToolTipItem(QQuickItem *item)
{
    if (item != m_toolTipItem) {
        m_toolTipItem = item;

        connect(item, &QQuickItem::windowChanged, this, &Backend::toolTipWindowChanged);

        emit toolTipItemChanged();
    }
}

QQuickWindow *Backend::groupDialog() const
{
    return m_groupDialog;
}

void Backend::setGroupDialog(QQuickWindow *dialog)
{
    if (dialog != m_groupDialog) {
        m_groupDialog = dialog;

        emit groupDialogChanged();
    }
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

        emit highlightWindowsChanged();
    }
}

QUrl Backend::tryDecodeApplicationsUrl(const QUrl &launcherUrl)
{
    if (launcherUrl.isValid() && launcherUrl.scheme() == QLatin1String("applications")) {
        const KService::Ptr service = KService::serviceByMenuId(launcherUrl.path());

        if (service) {
            return QUrl::fromLocalFile(service->entryPath());
        }
    }

    return launcherUrl;
}

QVariantList Backend::jumpListActions(const QUrl &launcherUrl, QObject *parent)
{
    QVariantList actions;

    if (!parent) {
        return actions;
    }

    QUrl desktopEntryUrl = tryDecodeApplicationsUrl(launcherUrl);

    if (!desktopEntryUrl.isValid() || !desktopEntryUrl.isLocalFile()
        || !KDesktopFile::isDesktopFile(desktopEntryUrl.toLocalFile())) {
        return actions;
    }

    const KService::Ptr service = KService::serviceByDesktopPath(desktopEntryUrl.toLocalFile());
    if (!service) {
        return actions;
    }

    const auto jumpListActions = service->actions();

    for (const KServiceAction &serviceAction : jumpListActions) {
        if (serviceAction.noDisplay()) {
            continue;
        }

        QAction *action = new QAction(parent);
        action->setText(serviceAction.text());
        action->setIcon(QIcon::fromTheme(serviceAction.icon()));
        if (serviceAction.isSeparator()) {
            action->setSeparator(true);
        }

        connect(action, &QAction::triggered, this, [this, serviceAction]() {
            auto *job = new KIO::ApplicationLauncherJob(serviceAction);
            auto *delegate = new KNotificationJobUiDelegate;
            delegate->setAutoErrorHandlingEnabled(true);
            job->setUiDelegate(delegate);
            job->start();
        });

        actions << QVariant::fromValue<QAction *>(action);
    }

    return actions;
}

QVariantList Backend::placesActions(const QUrl &launcherUrl, bool showAllPlaces, QObject *parent)
{
    if (!parent) {
        return QVariantList();
    }

    QUrl desktopEntryUrl = tryDecodeApplicationsUrl(launcherUrl);

    if (!desktopEntryUrl.isValid() || !desktopEntryUrl.isLocalFile()
        || !KDesktopFile::isDesktopFile(desktopEntryUrl.toLocalFile())) {
        return QVariantList();
    }

    QVariantList actions;
    KDesktopFile desktopFile(desktopEntryUrl.toLocalFile());

    // Since we can't have dynamic jump list actions, at least add the user's "Places" for file managers.
    const QStringList &categories = desktopFile.desktopGroup().readXdgListEntry(QStringLiteral("Categories"));
    if (!categories.contains(QLatin1String("FileManager"))) {
        return actions;
    }

    QString previousGroup;
    QMenu *subMenu = nullptr;

    QScopedPointer<KFilePlacesModel> placesModel(new KFilePlacesModel());
    for (int i = 0; i < placesModel->rowCount(); ++i) {
        QModelIndex idx = placesModel->index(i, 0);

        if (placesModel->isHidden(idx)) {
            continue;
        }

        const QString &title = idx.data(Qt::DisplayRole).toString();
        const QIcon &icon = idx.data(Qt::DecorationRole).value<QIcon>();
        const QUrl &url = idx.data(KFilePlacesModel::UrlRole).toUrl();

        QAction *placeAction = new QAction(icon, title, parent);

        connect(placeAction, &QAction::triggered, this, [url, desktopEntryUrl] {
            KService::Ptr service = KService::serviceByDesktopPath(desktopEntryUrl.toLocalFile());
            if (!service) {
                return;
            }

            auto *job = new KIO::ApplicationLauncherJob(service);
            auto *delegate = new KNotificationJobUiDelegate;
            delegate->setAutoErrorHandlingEnabled(true);
            job->setUiDelegate(delegate);

            job->setUrls({url});
            job->start();
        });

        const QString &groupName = idx.data(KFilePlacesModel::GroupRole).toString();
        if (previousGroup.isEmpty()) { // Skip first group heading.
            previousGroup = groupName;
        }

        // Put all subsequent categories into a submenu.
        if (previousGroup != groupName) {
            QAction *subMenuAction = new QAction(groupName, parent);
            subMenu = new QMenu();
            // Cannot parent a QMenu to a QAction, need to delete it manually.
            connect(parent, &QObject::destroyed, subMenu, &QObject::deleteLater);
            subMenuAction->setMenu(subMenu);

            actions << QVariant::fromValue(subMenuAction);

            previousGroup = groupName;
        }

        if (subMenu) {
            subMenu->addAction(placeAction);
        } else {
            actions << QVariant::fromValue(placeAction);
        }
    }

    // There is nothing more frustrating than having a "More" entry that ends up showing just one or two
    // additional entries. Therefore we truncate to max. 5 entries only if there are more than 7 in total.
    if (!showAllPlaces && actions.count() > 7) {
        const int totalActionCount = actions.count();

        while (actions.count() > 5) {
            actions.removeLast();
        }

        QAction *action = new QAction(parent);
        action->setText(i18ncp("Show all user Places", "%1 more Place", "%1 more Places", totalActionCount - actions.count()));
        connect(action, &QAction::triggered, this, &Backend::showAllPlaces);
        actions << QVariant::fromValue(action);
    }

    return actions;
}

QVariantList Backend::recentDocumentActions(const QUrl &launcherUrl, QObject *parent)
{
    if (!parent) {
        return QVariantList();
    }

    QUrl desktopEntryUrl = tryDecodeApplicationsUrl(launcherUrl);

    if (!desktopEntryUrl.isValid() || !desktopEntryUrl.isLocalFile()
        || !KDesktopFile::isDesktopFile(desktopEntryUrl.toLocalFile())) {
        return QVariantList();
    }

    QVariantList actions;
    QString desktopName = desktopEntryUrl.fileName();
    QString storageId = desktopName;

    if (storageId.endsWith(QLatin1String(".desktop"))) {
        storageId = storageId.left(storageId.length() - 8);
    }

    auto query = UsedResources
        | RecentlyUsedFirst
        | Agent(storageId)
        | Type::any()
        | Activity::current()
        | Url::file();

    ResultSet results(query);

    ResultSet::const_iterator resultIt = results.begin();

    int actionCount = 0;

    while (actionCount < 5 && resultIt != results.end()) {
        const QString resource = (*resultIt).resource();
        ++resultIt;

        const QUrl url(resource);

        if (!url.isValid()) {
            continue;
        }

        const KFileItem fileItem(url);

        if (!fileItem.isFile()) {
            continue;
        }

        QAction *action = new QAction(parent);
        action->setText(url.fileName());
        action->setIcon(QIcon::fromTheme(fileItem.iconName(), QIcon::fromTheme(QStringLiteral("unknown"))));
        action->setProperty("agent", storageId);
        action->setProperty("entryPath", desktopEntryUrl);
        action->setData(resource);
        connect(action, &QAction::triggered, this, &Backend::handleRecentDocumentAction);

        actions << QVariant::fromValue<QAction *>(action);

        ++actionCount;
    }

    if (actionCount > 0) {
        QAction *action = new QAction(parent);
        action->setText(i18n("Forget Recent Files"));
        action->setIcon(QIcon::fromTheme(QStringLiteral("edit-clear-history")));
        action->setProperty("agent", storageId);
        connect(action, &QAction::triggered, this, &Backend::handleRecentDocumentAction);

        actions << QVariant::fromValue<QAction *>(action);
    }

    return actions;
}

void Backend::toolTipWindowChanged(QQuickWindow *window)
{
    Q_UNUSED(window)

    updateWindowHighlight();
}

void Backend::handleRecentDocumentAction() const
{
    const QAction *action = qobject_cast<QAction* >(sender());

    if (!action) {
        return;
    }

    const QString agent = action->property("agent").toString();

    if (agent.isEmpty()) {
        return;
    }

    const QString desktopPath = action->property("entryPath").toUrl().toLocalFile();
    const QString resource = action->data().toString();

    if (desktopPath.isEmpty() || resource.isEmpty()) {
        auto query = UsedResources
            | Agent(agent)
            | Type::any()
            | Activity::current()
            | Url::file();

        KAStats::forgetResources(query);

        return;
    }

    KService::Ptr service = KService::serviceByDesktopPath(desktopPath);

        qDebug() << service;

    if (!service) {
        return;
    }

    auto *job = new KIO::ApplicationLauncherJob(service);
    auto *delegate = new KNotificationJobUiDelegate;
    delegate->setAutoErrorHandlingEnabled(true);
    job->setUiDelegate(delegate);

    job->setUrls({QUrl(resource)});
    job->start();
}

void Backend::setActionGroup(QAction *action) const
{
    if (action) {
        action->setActionGroup(m_actionGroup);
    }
}

QRect Backend::globalRect(QQuickItem *item) const
{
    if (!item || !item->window()) {
        return QRect();
    }

    QRect iconRect(item->x(), item->y(), item->width(), item->height());
    iconRect.moveTopLeft(item->parentItem()->mapToScene(iconRect.topLeft()).toPoint());
    iconRect.moveTopLeft(item->window()->mapToGlobal(iconRect.topLeft()));

    return iconRect;
}

void Backend::ungrabMouse(QQuickItem *item) const
{
    //this is a workaround where Qt will fail to realize a mouse has been released

    // this happens if a window which does not accept focus spawns a new window that takes focus and X grab
    // whilst the mouse is depressed
    // https://bugreports.qt.io/browse/QTBUG-59044
    // this causes the next click to go missing

    //by releasing manually we avoid that situation
    auto ungrabMouseHack = [item]() {
        if (item && item->window() &&  item->window()->mouseGrabberItem()) {
            item->window()->mouseGrabberItem()->ungrabMouse();
        }
    };

    //pre 5.8.0 QQuickWindow code is "item->grabMouse(); sendEvent(item, mouseEvent)"
    //post 5.8.0 QQuickWindow code is sendEvent(item, mouseEvent); item->grabMouse()
    if (QVersionNumber::fromString(QString::fromLatin1(qVersion())) > QVersionNumber(5, 8, 0)) {
        QTimer::singleShot(0, item, ungrabMouseHack);
    }
    else {
        ungrabMouseHack();
    }
    //end workaround
}

bool Backend::canPresentWindows() const
{
    return (KWindowSystem::compositingActive() && KWindowEffects::isEffectAvailable(KWindowEffects::PresentWindowsGroup));
}

void Backend::presentWindows(const QVariant &_winIds)
{
    if (!m_taskManagerItem || !m_taskManagerItem->window()) {
        return;
    }

    QList<WId> winIds;

    const QVariantList &_winIdsList = _winIds.toList();

    foreach(const QVariant &_winId, _winIdsList) {
        bool ok = false;
        qlonglong winId = _winId.toLongLong(&ok);

        if (ok) {
            winIds.append(winId);
        }
    }

    if (winIds.isEmpty()) {
        return;
    }

    if (m_windowsToHighlight.count()) {
        m_windowsToHighlight.clear();
        updateWindowHighlight();
    }

    KWindowEffects::presentWindows(m_taskManagerItem->window()->winId(), winIds);
}

bool Backend::isApplication(const QUrl &url) const
{
    if (!url.isValid() || !url.isLocalFile()) {
        return false;
    }

    const QString &localPath = url.toLocalFile();

    if (!KDesktopFile::isDesktopFile(localPath)) {
        return false;
    }

    KDesktopFile desktopFile(localPath);
    return desktopFile.hasApplicationType();
}

QList<QUrl> Backend::jsonArrayToUrlList(const QJsonArray &array) const
{
    QList<QUrl> urls;
    urls.reserve(array.count());

    for (auto it = array.constBegin(), end = array.constEnd(); it != end; ++it) {
        urls << QUrl(it->toString());
    }

    return urls;
}

void Backend::cancelHighlightWindows()
{
    m_windowsToHighlight.clear();
    updateWindowHighlight();
}

qint64 Backend::parentPid(qint64 pid) const
{
    KSysGuard::Processes procs;
    procs.updateOrAddProcess(pid);

    KSysGuard::Process *proc = procs.getProcess(pid);
    if (!proc) {
        return -1;
    }

    return proc->parentPid();
}

void Backend::windowsHovered(const QVariant &_winIds, bool hovered)
{
    m_windowsToHighlight.clear();

    if (hovered) {
        const QVariantList &winIds = _winIds.toList();

        foreach(const QVariant &_winId, winIds) {
            bool ok = false;
            qlonglong winId = _winId.toLongLong(&ok);

            if (ok) {
                m_windowsToHighlight.append(winId);
            }
        }
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

    if (!windows.isEmpty() && m_toolTipItem && m_toolTipItem->window()) {
        windows.append(m_toolTipItem->window()->winId());
    }

    if (!windows.isEmpty() && m_groupDialog) {
        windows.append(m_groupDialog->winId());
    }

    KWindowEffects::highlightWindows(m_panelWinId, windows);
}
