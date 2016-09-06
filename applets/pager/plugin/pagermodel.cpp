/********************************************************************
Copyright 2007  Daniel Laidig <d.laidig@gmx.de>
Copyright 2012  Luís Gabriel Lima <lampih@gmail.com>
Copyright 2016  Eike Hein <hein.org>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the
Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .
*********************************************************************/

#include "pagermodel.h"
#include "windowmodel.h"

#include <activityinfo.h>
#include <virtualdesktopinfo.h>
#include <windowtasksmodel.h>
#include <xwindowtasksmodel.h>

#include <QApplication>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCall>
#include <QDesktopWidget>
#include <QMetaEnum>

#include <KWindowSystem>

#include <KActivities/Controller>

using namespace TaskManager;

class PagerModel::Private
{
public:
    Private(PagerModel *q);
    ~Private();

    static int instanceCount;

    PagerType pagerType = VirtualDesktops;
    bool enabled = false;
    bool showDesktop = false;

    WindowTasksModel *tasksModel = nullptr;

    static ActivityInfo *activityInfo;
    QMetaObject::Connection activityInfoConn;

    static VirtualDesktopInfo *virtualDesktopInfo;
    QMetaObject::Connection virtualDesktopInfoConn;

    QDesktopWidget *desktopWidget = QApplication::desktop();

    QList<WindowModel *> windowModels;

#if HAVE_X11
    QList<WId> cachedStackingOrder = KWindowSystem::stackingOrder();
#endif

    void refreshDataSource();

private:
    PagerModel *q;
};

int PagerModel::Private::instanceCount = 0;
ActivityInfo *PagerModel::Private::activityInfo = nullptr;
VirtualDesktopInfo *PagerModel::Private::virtualDesktopInfo = nullptr;

PagerModel::Private::Private(PagerModel *q)
    : q(q)
{
    ++instanceCount;

    if (!activityInfo) {
        activityInfo = new ActivityInfo();
    }

    if (!virtualDesktopInfo) {
        virtualDesktopInfo = new VirtualDesktopInfo();
    }

    QObject::connect(activityInfo, &ActivityInfo::currentActivityChanged, q,
        [this]() {
            if (pagerType == VirtualDesktops && windowModels.count()) {
                for (auto windowModel : windowModels) {
                    windowModel->setActivity(activityInfo->currentActivity());
                }
            }
        }
    );

    QObject::connect(virtualDesktopInfo, &VirtualDesktopInfo::desktopLayoutRowsChanged,
        q, &PagerModel::layoutRowsChanged);

    QObject::connect(desktopWidget, &QDesktopWidget::screenCountChanged,
        q, &PagerModel::pagerItemSizeChanged);
    QObject::connect(desktopWidget, &QDesktopWidget::resized,
        q, &PagerModel::pagerItemSizeChanged);

#if HAVE_X11
    QObject::connect(KWindowSystem::self(), &KWindowSystem::stackingOrderChanged, q,
        [this]() {
            cachedStackingOrder = KWindowSystem::stackingOrder();

            for (auto windowModel : windowModels) {
                windowModel->refreshStackingOrder();
            }
        }
    );
#endif
}

PagerModel::Private::~Private()
{
    --instanceCount;

    if (!instanceCount) {
        delete activityInfo;
        activityInfo = nullptr;
        delete virtualDesktopInfo;
        virtualDesktopInfo = nullptr;
    }
}

void PagerModel::Private::refreshDataSource()
{
    if (pagerType == VirtualDesktops) {
        virtualDesktopInfoConn = QObject::connect(virtualDesktopInfo,
            &VirtualDesktopInfo::numberOfDesktopsChanged,
            q, [this]() { q->refresh(); }, Qt::UniqueConnection);

        QObject::disconnect(activityInfoConn);

        QObject::disconnect(activityInfo, &ActivityInfo::currentActivityChanged,
            q, &PagerModel::currentPageChanged);
        QObject::connect(virtualDesktopInfo, &VirtualDesktopInfo::currentDesktopChanged,
            q, &PagerModel::currentPageChanged, Qt::UniqueConnection);
    } else {
        activityInfoConn = QObject::connect(activityInfo,
            &ActivityInfo::numberOfRunningActivitiesChanged,
            q, [this]() { q->refresh(); }, Qt::UniqueConnection);

        QObject::disconnect(virtualDesktopInfoConn);

        QObject::disconnect(virtualDesktopInfo, &VirtualDesktopInfo::currentDesktopChanged,
            q, &PagerModel::currentPageChanged);
        QObject::connect(activityInfo, &ActivityInfo::currentActivityChanged,
            q, &PagerModel::currentPageChanged, Qt::UniqueConnection);
    }

    emit q->currentPageChanged();
}

PagerModel::PagerModel(QObject *parent)
    : QAbstractListModel(parent)
    , d(new Private(this))
{
    d->tasksModel = new WindowTasksModel(this);
}

PagerModel::~PagerModel()
{
}

QHash<int, QByteArray> PagerModel::roleNames() const
{
    QHash<int, QByteArray> roles = QAbstractItemModel::roleNames();

    QMetaEnum e = metaObject()->enumerator(metaObject()->indexOfEnumerator("AdditionalRoles"));

     for (int i = 0; i < e.keyCount(); ++i) {
         roles.insert(e.value(i), e.key(i));
     }

    return roles;
}

int PagerModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return d->windowModels.count();
}

QVariant PagerModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= d->windowModels.count()) {
        return QVariant();
    }

    if (role == Qt::DisplayRole) {
        if (d->pagerType == VirtualDesktops) {
            return d->virtualDesktopInfo->desktopNames().at(index.row());
        } else {
            QString activityId = d->activityInfo->runningActivities().at(index.row());
            return d->activityInfo->activityName(activityId);
        }
    } else if (role == TasksModel) {
        return QVariant::fromValue(d->windowModels.at(index.row()));
    }

    return QVariant();
}

PagerModel::PagerType PagerModel::pagerType() const
{
    return d->pagerType;
}

void PagerModel::setPagerType(PagerType type)
{
    if (d->pagerType != type) {
        d->pagerType = type;

        refresh();

        emit pagerTypeChanged();
    }
}

bool PagerModel::enabled() const
{
    return d->enabled;
}

void PagerModel::setEnabled(bool enabled)
{
    if (enabled && !d->enabled) {
        refresh();
        d->enabled = true;
        emit enabledChanged();
    } else if (!enabled && d->enabled) {
        beginResetModel();

        disconnect(d->activityInfoConn);
        disconnect(d->virtualDesktopInfoConn);

        qDeleteAll(d->windowModels);
        d->windowModels.clear();

        endResetModel();

        d->enabled = false;
        emit enabledChanged();

        emit countChanged();
    }
}

bool PagerModel::showDesktop() const
{
    return d->showDesktop;
}

void PagerModel::setShowDesktop(bool show)
{
    if (d->showDesktop != show) {
        d->showDesktop = show;

        emit showDesktopChanged();
    }
}

int PagerModel::currentPage() const
{
    if (d->pagerType == VirtualDesktops) {
        return d->virtualDesktopInfo->currentDesktop();
    } else {
        return d->activityInfo->runningActivities().indexOf(d->activityInfo->currentActivity());
    }
}

int PagerModel::layoutRows() const
{
    return qBound(1, d->virtualDesktopInfo->desktopLayoutRows(),
        d->virtualDesktopInfo->numberOfDesktops());
}

QSize PagerModel::pagerItemSize() const
{
    QRect totalRect;

    for (int i = 0; i < d->desktopWidget->screenCount(); ++i) {
        totalRect |= d->desktopWidget->screenGeometry(i);
    }

    return totalRect.size();
}

#if HAVE_X11
QList<WId> PagerModel::stackingOrder() const
{
    return d->cachedStackingOrder;
}
#endif

void PagerModel::refresh()
{
    beginResetModel();

    d->refreshDataSource();

    int modelCount = d->windowModels.count();
    const int modelsNeeded = ((d->pagerType == VirtualDesktops)
        ? d->virtualDesktopInfo->numberOfDesktops()
        : d->activityInfo->numberOfRunningActivities());

    if (modelCount > modelsNeeded) {
        while (modelCount != modelsNeeded) {
            delete d->windowModels.takeLast();
            --modelCount;
        }
    } else if (modelsNeeded > modelCount) {
        while (modelCount != modelsNeeded) {
            WindowModel *windowModel = new WindowModel(this);
            windowModel->setFilterSkipPager(true);
            windowModel->setFilterByVirtualDesktop(true);
            windowModel->setFilterByActivity(true);
            windowModel->setSourceModel(d->tasksModel);
            d->windowModels.append(windowModel);
            ++modelCount;
        }
    }

    if (d->pagerType == VirtualDesktops) {
        int virtualDesktop = 1;

        for (auto windowModel : d->windowModels) {
            windowModel->setVirtualDesktop(virtualDesktop);
            ++virtualDesktop;

            windowModel->setActivity(d->activityInfo->currentActivity());
        }
    } else {
        int activityIndex = 0;
        const QStringList &runningActivities = d->activityInfo->runningActivities();

        for (auto windowModel : d->windowModels) {
            windowModel->setVirtualDesktop(0);

            windowModel->setActivity(runningActivities.at(activityIndex));
            ++activityIndex;
        }
    }

    endResetModel();

    emit countChanged();
}

void PagerModel::moveWindow(int window, double x, double y, int targetItemId, int sourceItemId,
    qreal widthScaleFactor, qreal heightScaleFactor)
{
#if HAVE_X11
    if (!KWindowSystem::isPlatformX11()) {
        return;
    }

    const WId windowId = (WId)window;

    QPointF dest(x / widthScaleFactor, y / heightScaleFactor);

    // Don't move windows to negative positions.
    dest = QPointF(qMax(dest.x(), qreal(0.0)), qMax(dest.y(), qreal(0.0)));

    // Use _NET_MOVERESIZE_WINDOW rather than plain move, so that the WM knows this is a pager request.
    NETRootInfo info(QX11Info::connection(), 0);
    const int flags = (0x20 << 12) | (0x03 << 8) | 1; // From tool, x/y, northwest gravity.

    if (!KWindowSystem::mapViewport()) {
        KWindowInfo windowInfo(windowId, NET::WMDesktop | NET::WMState);

        if (d->pagerType == VirtualDesktops) {
            if (!windowInfo.onAllDesktops()) {
                KWindowSystem::setOnDesktop(windowId, targetItemId + 1);
            }
        } else {
            const QStringList &runningActivities = d->activityInfo->runningActivities();

            if (targetItemId < runningActivities.length()) {
                KActivities::Controller activitiesController;
                activitiesController.setCurrentActivity(runningActivities.at(targetItemId));
            }
        }

        // Only move the window if it is not full screen and if it is kept within the same desktop.
        // Moving when dropping between desktop is too annoying due to the small drop area.
        if (!(windowInfo.state() & NET::FullScreen) &&
            (targetItemId == sourceItemId || windowInfo.onAllDesktops())) {
            const QPoint &d = dest.toPoint();
            info.moveResizeWindowRequest(windowId, flags, d.x(), d.y(), 0, 0);
        }
    } else {
        // setOnDesktop() with viewports is also moving a window, and since it takes a moment
        // for the WM to do the move, there's a race condition with figuring out how much to move,
        // so do it only as one move.
        dest += KWindowSystem::desktopToViewport(targetItemId + 1, false);
        const QPoint &d = KWindowSystem::constrainViewportRelativePosition(dest.toPoint());
        info.moveResizeWindowRequest(windowId, flags, d.x(), d.y(), 0, 0);
    }
#else
    Q_UNUSED(window)
    Q_UNUSED(x)
    Q_UNUSED(y)
    Q_UNUSED(targetDesktop)
    Q_UNUSED(sourceDesktop)
#endif
}

void PagerModel::changePage(int itemId)
{
#if HAVE_X11
    if (!KWindowSystem::isPlatformX11()) {
        return;
    }

    const int targetId = (d->pagerType == VirtualDesktops) ? itemId + 1 : itemId;

    if (currentPage() == targetId) {
        if (d->showDesktop) {
            QDBusConnection::sessionBus().asyncCall(QDBusMessage::createMethodCall(QLatin1String("org.kde.plasmashell"),
                QLatin1String("/PlasmaShell"),
                QLatin1String("org.kde.PlasmaShell"),
                QLatin1String("toggleDashboard")));
        }
    } else {
        if (d->pagerType == VirtualDesktops) {
            KWindowSystem::setCurrentDesktop(targetId);
        } else {
            const QStringList &runningActivities = d->activityInfo->runningActivities();

            if (targetId < runningActivities.length()) {
                KActivities::Controller activitiesController;
                activitiesController.setCurrentActivity(runningActivities.at(targetId));
            }
        }
    }
#else
    Q_UNUSED(itemId)
#endif
}

void PagerModel::drop(QMimeData *mimeData, int itemId)
{
    if (!mimeData) {
        return;
    }

#if HAVE_X11
    if (KWindowSystem::isPlatformX11()) {
        bool ok;

        const QList<WId> &ids = TaskManager::XWindowTasksModel::winIdsFromMimeData(mimeData, &ok);

        if (!ok) {
            return;
        }

        if (d->pagerType == VirtualDesktops) {
            for (const auto &id : ids) {
                KWindowSystem::setOnDesktop(id, itemId + 1);
            }
        } else {
            QString newActivity;
            const QStringList &runningActivities = d->activityInfo->runningActivities();

            if (itemId < runningActivities.length()) {
                newActivity = runningActivities.at(itemId);
            }

            for (const auto &id : ids) {
                KWindowSystem::setOnDesktop(id, itemId + 1);
            }
        }
    }
#else
    Q_UNUSED(itemId)
#endif
}

void PagerModel::addDesktop()
{
#if HAVE_X11
    if (!KWindowSystem::isPlatformX11()) {
        return;
    }

    NETRootInfo info(QX11Info::connection(), NET::NumberOfDesktops);
    info.setNumberOfDesktops(info.numberOfDesktops() + 1);
#endif
}

void PagerModel::removeDesktop()
{
#if HAVE_X11
    if (!KWindowSystem::isPlatformX11()) {
        return;
    }

    NETRootInfo info(QX11Info::connection(), NET::NumberOfDesktops);

    if (info.numberOfDesktops() > 1) {
        info.setNumberOfDesktops(info.numberOfDesktops() - 1);
    }
#endif
}

#include "moc_pagermodel.cpp"
