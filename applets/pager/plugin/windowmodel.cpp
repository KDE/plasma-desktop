/********************************************************************
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

#include "windowmodel.h"
#include "pagermodel.h"

#include <abstracttasksmodel.h>

#include <QGuiApplication>
#include <QMetaEnum>
#include <QScreen>

#include <KWindowSystem>

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
        QList<QScreen *> screens = QGuiApplication::screens();
        const QRect desktopGeo = screens.at(0)->virtualGeometry();

        if (KWindowSystem::mapViewport()) {
            int x = windowGeo.center().x() % desktopGeo.width();
            int y = windowGeo.center().y() % desktopGeo.height();

            if (x < 0) {
                x = x + desktopGeo.width();
            }

            if (y < 0) {
                y = y + desktopGeo.height();
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
        // TODO: Switch from qBound to std::clamp once we use C++17.
        windowGeo.setX(qBound(0, windowGeo.x(), desktopGeo.width()));
        windowGeo.setY(qBound(0, windowGeo.y(), desktopGeo.height()));

        if ((windowGeo.x() + windowGeo.width()) > desktopGeo.width()) {
            windowGeo.setWidth(desktopGeo.width() - windowGeo.x());
        }

        if ((windowGeo.y() + windowGeo.height()) > desktopGeo.height()) {
            windowGeo.setHeight(desktopGeo.height() - windowGeo.y());
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
