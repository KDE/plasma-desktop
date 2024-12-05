/*
    SPDX-FileCopyrightText: 2007 Daniel Laidig <d.laidig@gmx.de>
    SPDX-FileCopyrightText: 2012 Luís Gabriel Lima <lampih@gmail.com>
    SPDX-FileCopyrightText: 2016 Eike Hein <hein.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "pagermodel.h"
#include "windowmodel.h"

#include <activityinfo.h>
#include <virtualdesktopinfo.h>
#include <waylandtasksmodel.h>
#include <windowtasksmodel.h>
#include <xwindowtasksmodel.h>

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCall>
#include <QGuiApplication>
#include <QMetaEnum>
#include <QScreen>
#include <QUuid>

#include <KWindowSystem>

#include <config-X11.h>

#if HAVE_X11
#include <KX11Extras>

#include <QtGui/private/qtx11extras_p.h>
#endif

#include <PlasmaActivities/Controller>

using namespace TaskManager;

class PagerModel::Private
{
public:
    Private(PagerModel *q);
    ~Private();

    static int instanceCount;

    bool componentComplete = false;

    PagerType pagerType = VirtualDesktops;
    bool enabled = false;
    bool showDesktop = false;

    bool showOnlyCurrentScreen = false;
    QRect screenGeometry;
    QRect virtualGeometry;

    WindowTasksModel *tasksModel = nullptr;

    static ActivityInfo *activityInfo;
    QMetaObject::Connection activityNumberConn;
    QMetaObject::Connection activityNamesConn;

    static VirtualDesktopInfo *virtualDesktopInfo;
    QMetaObject::Connection virtualDesktopNumberConn;
    QMetaObject::Connection virtualDesktopNamesConn;

    QList<WindowModel *> windowModels;

    void refreshDataSource();

private:
    PagerModel *const q;
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

    QObject::connect(activityInfo, &ActivityInfo::numberOfRunningActivitiesChanged, q, &PagerModel::shouldShowPagerChanged);

    if (!virtualDesktopInfo) {
        virtualDesktopInfo = new VirtualDesktopInfo();
    }

    QObject::connect(virtualDesktopInfo, &VirtualDesktopInfo::numberOfDesktopsChanged, q, &PagerModel::shouldShowPagerChanged);

    QObject::connect(activityInfo, &ActivityInfo::currentActivityChanged, q, [this]() {
        if (pagerType == VirtualDesktops && windowModels.count()) {
            for (auto windowModel : std::as_const(windowModels)) {
                windowModel->setActivity(activityInfo->currentActivity());
            }
        }
    });

    QObject::connect(virtualDesktopInfo, &VirtualDesktopInfo::desktopLayoutRowsChanged, q, &PagerModel::layoutRowsChanged);
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
        QObject::disconnect(virtualDesktopNumberConn);
        virtualDesktopNumberConn = QObject::connect(virtualDesktopInfo, &VirtualDesktopInfo::numberOfDesktopsChanged, q, [this]() {
            q->refresh();
        });

        QObject::disconnect(virtualDesktopNamesConn);
        virtualDesktopNamesConn = QObject::connect(virtualDesktopInfo, &VirtualDesktopInfo::desktopNamesChanged, q, [this]() {
            if (q->rowCount()) {
                Q_EMIT q->dataChanged(q->index(0, 0), q->index(q->rowCount() - 1, 0), QList<int>{Qt::DisplayRole});
            }
        });

        QObject::disconnect(activityNumberConn);
        QObject::disconnect(activityNamesConn);

        QObject::disconnect(activityInfo, &ActivityInfo::currentActivityChanged, q, &PagerModel::currentPageChanged);
        QObject::connect(virtualDesktopInfo, &VirtualDesktopInfo::currentDesktopChanged, q, &PagerModel::currentPageChanged, Qt::UniqueConnection);
    } else {
        QObject::disconnect(activityNumberConn);
        activityNumberConn = QObject::connect(activityInfo, &ActivityInfo::numberOfRunningActivitiesChanged, q, [this]() {
            q->refresh();
        });

        QObject::disconnect(activityNamesConn);
        activityNamesConn = QObject::connect(activityInfo, &ActivityInfo::namesOfRunningActivitiesChanged, q, [this]() {
            q->refresh();
        });

        QObject::disconnect(virtualDesktopNumberConn);
        QObject::disconnect(virtualDesktopNamesConn);

        QObject::disconnect(virtualDesktopInfo, &VirtualDesktopInfo::currentDesktopChanged, q, &PagerModel::currentPageChanged);
        QObject::connect(activityInfo, &ActivityInfo::currentActivityChanged, q, &PagerModel::currentPageChanged, Qt::UniqueConnection);
    }

    Q_EMIT q->currentPageChanged();
}

PagerModel::PagerModel(QObject *parent)
    : QAbstractListModel(parent)
    , d(new Private(this))
{
    d->tasksModel = new WindowTasksModel(this);

    computePagerItemSize();
    const auto screens = qGuiApp->screens();
    for (QScreen *screen : screens) {
        connect(screen, &QScreen::geometryChanged, this, &PagerModel::computePagerItemSize);
    }
    connect(qGuiApp, &QGuiApplication::screenAdded, this, [this](QScreen *screen) {
        connect(screen, &QScreen::geometryChanged, this, &PagerModel::computePagerItemSize);
        computePagerItemSize();
    });
    connect(qGuiApp, &QGuiApplication::screenRemoved, this, &PagerModel::computePagerItemSize);
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

        Q_EMIT pagerTypeChanged();
        Q_EMIT shouldShowPagerChanged();
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
        Q_EMIT enabledChanged();
    } else if (!enabled && d->enabled) {
        beginResetModel();

        disconnect(d->activityNumberConn);
        disconnect(d->activityNamesConn);
        disconnect(d->virtualDesktopNumberConn);
        disconnect(d->virtualDesktopNamesConn);

        qDeleteAll(d->windowModels);
        d->windowModels.clear();

        endResetModel();

        d->enabled = false;
        Q_EMIT enabledChanged();

        Q_EMIT countChanged();
    }
}

bool PagerModel::shouldShowPager() const
{
    return (d->pagerType == VirtualDesktops) ? d->virtualDesktopInfo->numberOfDesktops() > 1 : d->activityInfo->numberOfRunningActivities() > 1;
}

bool PagerModel::showDesktop() const
{
    return d->showDesktop;
}

void PagerModel::setShowDesktop(bool show)
{
    if (d->showDesktop != show) {
        d->showDesktop = show;

        Q_EMIT showDesktopChanged();
    }
}

bool PagerModel::showOnlyCurrentScreen() const
{
    return d->showOnlyCurrentScreen;
}

void PagerModel::setShowOnlyCurrentScreen(bool show)
{
    if (d->showOnlyCurrentScreen != show) {
        d->showOnlyCurrentScreen = show;

        if (d->screenGeometry.isValid()) {
            Q_EMIT pagerItemSizeChanged();

            refresh();
        }

        Q_EMIT showOnlyCurrentScreenChanged();
    }
}

QRect PagerModel::screenGeometry() const
{
    return d->screenGeometry;
}

void PagerModel::setScreenGeometry(const QRect &geometry)
{
    if (d->screenGeometry != geometry) {
        d->screenGeometry = geometry;

        if (d->showOnlyCurrentScreen) {
            Q_EMIT pagerItemSizeChanged();

            refresh();
        }

        Q_EMIT showOnlyCurrentScreenChanged();
    }
}

int PagerModel::currentPage() const
{
    if (d->pagerType == VirtualDesktops) {
        return d->virtualDesktopInfo->desktopIds().indexOf(d->virtualDesktopInfo->currentDesktop());
    } else {
        return d->activityInfo->runningActivities().indexOf(d->activityInfo->currentActivity());
    }
}

int PagerModel::layoutRows() const
{
    const int rows = std::min(d->virtualDesktopInfo->desktopLayoutRows(), d->virtualDesktopInfo->numberOfDesktops());
    return std::max(1, rows);
}

QSize PagerModel::pagerItemSize() const
{
    if (d->showOnlyCurrentScreen && d->screenGeometry.isValid()) {
#if HAVE_X11
        const double devicePixelRatio = KWindowSystem::isPlatformWayland() ? 1.0 : qGuiApp->devicePixelRatio();
#else
        constexpr int devicePixelRatio = 1;
#endif
        return d->screenGeometry.size() * devicePixelRatio;
    }

    return d->virtualGeometry.size();
}

QList<QVariant> PagerModel::stackingOrder(const QModelIndex &index) const
{
    return index.data(TaskManager::AbstractTasksModel::StackingOrder).toList();
}

void PagerModel::refresh()
{
    if (!d->componentComplete) {
        return;
    }

    beginResetModel();

    d->refreshDataSource();

    int modelCount = d->windowModels.count();
    const int modelsNeeded = ((d->pagerType == VirtualDesktops) ? d->virtualDesktopInfo->numberOfDesktops() : d->activityInfo->numberOfRunningActivities());

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
            windowModel->setDemandingAttentionSkipsFilters(false);
            windowModel->setSourceModel(d->tasksModel);
            d->windowModels.append(windowModel);
            ++modelCount;
        }
    }

    if (d->pagerType == VirtualDesktops) {
        int virtualDesktop = 0;

        for (auto windowModel : std::as_const(d->windowModels)) {
            windowModel->setVirtualDesktop(d->virtualDesktopInfo->desktopIds().at(virtualDesktop));
            ++virtualDesktop;

            windowModel->setActivity(d->activityInfo->currentActivity());
        }
    } else {
        int activityIndex = 0;
        const QStringList &runningActivities = d->activityInfo->runningActivities();

        for (auto windowModel : std::as_const(d->windowModels)) {
            windowModel->setVirtualDesktop();

            windowModel->setActivity(runningActivities.at(activityIndex));
            ++activityIndex;
        }
    }

    for (auto windowModel : std::as_const(d->windowModels)) {
        if (d->showOnlyCurrentScreen && d->screenGeometry.isValid()) {
            windowModel->setScreenGeometry(d->screenGeometry);
            windowModel->setFilterByScreen(true);
        } else {
            windowModel->setFilterByScreen(false);
        }
    }

    endResetModel();

    Q_EMIT countChanged();
}

void PagerModel::moveWindow(const QModelIndex &index,
                            double x,
                            double y,
                            const QVariant &targetItemId,
                            const QVariant &sourceItemId,
                            qreal widthScaleFactor,
                            qreal heightScaleFactor)
{
    const auto taskModelIndex = static_cast<const WindowModel *>(index.model())->mapToSource(index);
    const bool isOnAllDesktops = index.data(TaskManager::AbstractTasksModel::IsOnAllVirtualDesktops).toBool();

    if (d->pagerType == VirtualDesktops) {
        if (!isOnAllDesktops) {
            d->tasksModel->requestVirtualDesktops(taskModelIndex, {targetItemId});
        }
    } else {
        const QStringList &runningActivities = d->activityInfo->runningActivities();
        const QString &newActivity = targetItemId.toString();
        if (runningActivities.contains(newActivity)) {
            QStringList activities = index.data(TaskManager::AbstractTasksModel::Activities).toStringList();
            if (!activities.contains(newActivity)) {
                activities.removeOne(sourceItemId.toString());
                activities.append(newActivity);
                d->tasksModel->requestActivities(taskModelIndex, activities);
            }
        }
    }
#if HAVE_X11
    if (KWindowSystem::isPlatformX11() && !index.data(TaskManager::AbstractTasksModel::IsFullScreen).toBool()
        && (targetItemId == sourceItemId || isOnAllDesktops)) {
        const auto winIds = index.data(TaskManager::AbstractTasksModel::WinIdList).toList();
        if (winIds.isEmpty()) {
            return;
        }
        QPointF dest(x / widthScaleFactor, y / heightScaleFactor);

        // Don't move windows to negative positions.
        dest = QPointF(qMax(dest.x(), qreal(0.0)), qMax(dest.y(), qreal(0.0)));

        // Use _NET_MOVERESIZE_WINDOW rather than plain move, so that the WM knows this is a pager request.
        NETRootInfo info(QX11Info::connection(), NET::Properties());
        const int flags = (0x20 << 12) | (0x03 << 8) | 1; // From tool, x/y, northwest gravity.
        const QPoint &d = dest.toPoint();
        info.moveResizeWindowRequest(winIds[0].toUInt(), flags, d.x(), d.y(), 0, 0);
    }
#endif
}

void PagerModel::changePage(int page)
{
    if (currentPage() == page) {
        if (d->showDesktop) {
            QDBusConnection::sessionBus().asyncCall(QDBusMessage::createMethodCall(QLatin1String("org.kde.plasmashell"),
                                                                                   QLatin1String("/PlasmaShell"),
                                                                                   QLatin1String("org.kde.PlasmaShell"),
                                                                                   QLatin1String("toggleDashboard")));
        }
    } else {
        if (d->pagerType == VirtualDesktops) {
            d->virtualDesktopInfo->requestActivate(d->virtualDesktopInfo->desktopIds().at(page));
        } else {
            const QStringList &runningActivities = d->activityInfo->runningActivities();
            if (page < runningActivities.length()) {
                KActivities::Controller activitiesController;
                activitiesController.setCurrentActivity(runningActivities.at(page));
            }
        }
    }
}

void PagerModel::drop(QMimeData *mimeData, int modifiers, const QVariant &itemId)
{
    if (!mimeData) {
        return;
    }

    auto findWindows = [this](const auto &windowIds) -> QList<QModelIndex> {
        QList<QModelIndex> indices;
        for (const auto &id : windowIds) {
            for (int i = 0; i < d->tasksModel->rowCount(); ++i) {
                const QModelIndex &idx = d->tasksModel->index(i, 0);
                const QVariantList &winIds = idx.data(TaskManager::AbstractTasksModel::WinIdList).toList();
                if (!winIds.isEmpty() && winIds.at(0).value<typename std::remove_reference_t<decltype(windowIds)>::value_type>() == id) {
                    indices.push_back(idx);
                    break;
                }
            }
        }
        return indices;
    };

    bool ok = false;
    QList<QModelIndex> indices;
    if (KWindowSystem::isPlatformX11()) {
        indices = findWindows(TaskManager::XWindowTasksModel::winIdsFromMimeData(mimeData, &ok));
    } else if (KWindowSystem::isPlatformWayland()) {
        indices = findWindows(TaskManager::WaylandTasksModel::winIdsFromMimeData(mimeData, &ok));
    }
    if (!ok) {
        return;
    }

    for (const auto &index : std::as_const(indices)) {
        if (d->pagerType == VirtualDesktops) {
            if (!index.data(TaskManager::AbstractTasksModel::IsOnAllVirtualDesktops).toBool()) {
                d->tasksModel->requestVirtualDesktops(index, {itemId});
            }
        } else {
            QString newActivity = itemId.toString();
            const QStringList &runningActivities = d->activityInfo->runningActivities();

            if (!runningActivities.contains(newActivity)) {
                return;
            }

            QStringList activities = index.data(TaskManager::AbstractTasksModel::Activities).toStringList();

            if (modifiers & Qt::ControlModifier) { // 'copy' => add to activity
                if (!activities.contains(newActivity)) {
                    activities << newActivity;
                }
            } else { // 'move' to activity
                // if on only one activity, set it to only the new activity
                // if on >1 activity, remove it from the current activity and add it to the new activity
                const QString currentActivity = d->activityInfo->currentActivity();
                activities.removeAll(currentActivity);
                activities << newActivity;
            }
            d->tasksModel->requestActivities(index, activities);
        }
    }
}

void PagerModel::addDesktop()
{
    d->virtualDesktopInfo->requestCreateDesktop(d->virtualDesktopInfo->numberOfDesktops());
}

void PagerModel::removeDesktop()
{
    if (d->virtualDesktopInfo->numberOfDesktops() == 1) {
        return;
    }

    d->virtualDesktopInfo->requestRemoveDesktop(d->virtualDesktopInfo->numberOfDesktops() - 1);
}

void PagerModel::classBegin()
{
}

void PagerModel::componentComplete()
{
    d->componentComplete = true;

    if (d->enabled) {
        refresh();
    }
}

void PagerModel::computePagerItemSize()
{
#if HAVE_X11
    const double devicePixelRatio = KWindowSystem::isPlatformWayland() ? 1.0 : qGuiApp->devicePixelRatio();
#else
    constexpr int devicePixelRatio = 1;
#endif
    QRect wholeScreen;
    for (const auto screens = qGuiApp->screens(); auto screen : screens) {
        const QRect geometry = screen->geometry();
        wholeScreen |= QRect(geometry.topLeft(), geometry.size() * devicePixelRatio);
    }

    if (d->virtualGeometry != wholeScreen) {
        d->virtualGeometry = wholeScreen;
        Q_EMIT pagerItemSizeChanged();
    }
}

#include "moc_pagermodel.cpp"
