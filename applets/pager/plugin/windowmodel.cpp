/*
    SPDX-FileCopyrightText: 2016 Eike Hein <hein.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "windowmodel.h"
#include "pagermodel.h"

#include <abstracttasksmodel.h>

#include <QGuiApplication>
#include <QMetaEnum>
#include <QScreen>

#include <KWindowSystem>
#include <KX11Extras>

#include <algorithm>

using namespace TaskManager;

class WindowModel::Private
{
public:
    Private(WindowModel *q);

    PagerModel *pagerModel = nullptr;

private:
    WindowModel *q;
};

WindowModel::Private::Private(WindowModel *q)
    : q(q)
{
    Q_UNUSED(this->q);
}

WindowModel::WindowModel(PagerModel *parent)
    : TaskFilterProxyModel(parent)
    , d(new Private(this))
{
    d->pagerModel = parent;
}

WindowModel::~WindowModel()
{
}

QHash<int, QByteArray> WindowModel::roleNames() const
{
    QHash<int, QByteArray> roles = TaskFilterProxyModel::roleNames();

    QMetaEnum e = metaObject()->enumerator(metaObject()->indexOfEnumerator("WindowModelRoles"));

    for (int i = 0; i < e.keyCount(); ++i) {
        roles.insert(e.value(i), e.key(i));
    }

    return roles;
}

QVariant WindowModel::data(const QModelIndex &index, int role) const
{
    if (role == AbstractTasksModel::Geometry) {
        QRect windowGeo = TaskFilterProxyModel::data(index, role).toRect();
        const QScreen *const screen = QGuiApplication::screens().constFirst();
        const QSize desktopSize = screen->virtualSize();

        // KWindowInfoPrivateX11::frameGeometry() returns the true geometry of a window, so devicePixelRatio is needed.
        if (const auto ratio = screen->devicePixelRatio(); KWindowSystem::isPlatformX11() && ratio != 1.0) {
            windowGeo.setTopLeft(windowGeo.topLeft() / ratio);
            windowGeo.setBottomRight(windowGeo.bottomRight() / ratio);
        }

        if (KWindowSystem::isPlatformX11() && KX11Extras::mapViewport()) {
            int x = windowGeo.center().x() % desktopSize.width();
            int y = windowGeo.center().y() % desktopSize.height();

            if (x < 0) {
                x = x + desktopSize.width();
            }

            if (y < 0) {
                y = y + desktopSize.height();
            }

            const QRect mappedGeo(x - windowGeo.width() / 2, y - windowGeo.height() / 2, windowGeo.width(), windowGeo.height());

            if (filterByScreen() && screenGeometry().isValid()) {
                const QPoint &screenOffset = screenGeometry().topLeft();

                windowGeo = mappedGeo.translated(0 - screenOffset.x(), 0 - screenOffset.y());
            }
        } else if (filterByScreen() && screenGeometry().isValid()) {
            const QPoint &screenOffset = screenGeometry().topLeft();

            windowGeo.translate(0 - screenOffset.x(), 0 - screenOffset.y());
        }

        // Clamp to desktop rect.
        windowGeo.setX(std::clamp(windowGeo.x(), 0, desktopSize.width()));
        windowGeo.setY(std::clamp(windowGeo.y(), 0, desktopSize.height()));

        if ((windowGeo.x() + windowGeo.width()) > desktopSize.width()) {
            windowGeo.setWidth(desktopSize.width() - windowGeo.x());
        }

        if ((windowGeo.y() + windowGeo.height()) > desktopSize.height()) {
            windowGeo.setHeight(desktopSize.height() - windowGeo.y());
        }

        return windowGeo;
    } else if (role == StackingOrder) {
#if HAVE_X11
        const QVariantList &winIds = TaskFilterProxyModel::data(index, AbstractTasksModel::WinIdList).toList();

        if (winIds.count()) {
            const WId winId = winIds.at(0).toLongLong();
            const int z = d->pagerModel->stackingOrder().indexOf(winId);

            if (z != -1) {
                return z;
            }
        }
#endif

        return 0;
    }

    return TaskFilterProxyModel::data(index, role);
}

void WindowModel::refreshStackingOrder()
{
    if (rowCount()) {
        Q_EMIT dataChanged(index(0, 0), index(rowCount() - 1, 0), QVector<int>{StackingOrder});
    }
}
