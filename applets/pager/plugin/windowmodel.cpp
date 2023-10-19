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
    connect(parent, &PagerModel::pagerItemSizeChanged, this, &WindowModel::onPagerItemSizeChanged);
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
        const QRect clampingRect(QPoint(0, 0), d->pagerModel->pagerItemSize());

        if (KWindowSystem::isPlatformX11() && KX11Extras::mapViewport()) {
            int x = windowGeo.center().x() % clampingRect.width();
            int y = windowGeo.center().y() % clampingRect.height();

            if (x < 0) {
                x = x + clampingRect.width();
            }

            if (y < 0) {
                y = y + clampingRect.height();
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

        // Restrict to desktop/screen rect.
        return windowGeo.intersected(clampingRect);
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
        Q_EMIT dataChanged(index(0, 0), index(rowCount() - 1, 0), QList<int>{StackingOrder});
    }
}

void WindowModel::onPagerItemSizeChanged()
{
    if (rowCount() > 0) {
        Q_EMIT dataChanged(index(0, 0), index(rowCount() - 1, 0), {AbstractTasksModel::Geometry});
    }
}
