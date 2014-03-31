/*
 * Copyright 2012  Luís Gabriel Lima <lampih@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "model.h"

RectangleModel::RectangleModel(QObject *parent)
    : QAbstractListModel(parent)
{
    setRoleNames(roles());
}

QHash<int, QByteArray> RectangleModel::roles() const
{
    QHash<int, QByteArray> rectRoles;
    rectRoles[WidthRole] = "width";
    rectRoles[HeightRole] = "height";
    rectRoles[XRole] = "x";
    rectRoles[YRole] = "y";
    return rectRoles;
}

void RectangleModel::clear()
{
    m_rects.clear();
}

void RectangleModel::append(const QRectF &rect)
{
    m_rects.append(rect);
}

QRectF &RectangleModel::rectAt(int index)
{
    return m_rects[index];
}

int RectangleModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_rects.count();
}

QVariant RectangleModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() > m_rects.count())
        return QVariant();

    const QRectF &rect = m_rects[index.row()];
    switch(role) {
    case WidthRole:
        return rect.width();
    case HeightRole:
        return rect.height();
    case XRole:
        return rect.x();
    case YRole:
        return rect.y();
    default:
        return QVariant();
    }
}


WindowModel::WindowModel(QObject *parent)
    : RectangleModel(parent)
{
    setRoleNames(roles());
}

QHash<int, QByteArray> WindowModel::roles() const
{
    QHash<int, QByteArray> rectRoles = RectangleModel::roles();
    rectRoles[IdRole] = "windowId";
    rectRoles[ActiveRole] = "active";
    rectRoles[IconRole] = "icon";
    rectRoles[VisibleNameRole] = "visibleName";
    return rectRoles;
}

void WindowModel::clear()
{
    beginResetModel();
    RectangleModel::clear();
    m_ids.clear();
    m_active.clear();
    m_icons.clear();
    m_visibleNames.clear();
    endResetModel();
}

void WindowModel::append(WId windowId, const QRectF &rect, bool active,
                         const QPixmap &icon, const QString &name)
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    m_ids.append(windowId);
    RectangleModel::append(rect);
    m_active.append(active);
    m_icons.append(icon);
    m_visibleNames.append(name);
    endInsertRows();
}

WId WindowModel::idAt(int index) const
{
    return m_ids[index];
}

QString WindowModel::visibleNameAt(int index) const
{
    return m_visibleNames[index];
}

QVariant WindowModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= rowCount())
        return QVariant();

    if (role >= RectangleModel::WidthRole && role < IdRole)
        return RectangleModel::data(index, role);

    switch (role) {
    case IdRole:
        return int(m_ids[index.row()]);
    case ActiveRole:
        return m_active[index.row()];
    case IconRole:
        return m_icons[index.row()];
    case VisibleNameRole:
        return m_visibleNames[index.row()];
    default:
        return QVariant();
    }
}


PagerModel::PagerModel(QObject *parent)
    : QAbstractListModel(parent)
{
    setRoleNames(roles());
}

WindowModel *PagerModel::windowsAt(int index) const
{
    if (index < 0 || index >= m_windows.count())
        return 0;

    return qobject_cast<WindowModel *>(m_windows[index]);
}

QHash<int, QByteArray> PagerModel::roles() const
{
    QHash<int, QByteArray> rectRoles = m_desktops.roles();
    rectRoles[WindowsRole] = "windows";
    rectRoles[DesktopNameRole] = "desktopName";
    return rectRoles;
}

void PagerModel::clearDesktopRects()
{
    beginResetModel();
    m_desktops.clear();
    m_names.clear();
    endResetModel();
}

void PagerModel::appendDesktopRect(const QRectF &rect, const QString &name)
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    m_desktops.append(rect);
    m_names.append(name);
    endInsertRows();
}

QRectF& PagerModel::desktopRectAt(int index)
{
    return m_desktops.rectAt(index);
}

void PagerModel::clearWindowRects()
{
    int removeIndex = -1;
    for (int i = 0; i < m_windows.count(); i++) {
        windowsAt(i)->clear();

        if (i >= rowCount())
            removeIndex = (removeIndex == -1) ? i : -1;
    }

    if (removeIndex != -1) {
        // remove the windows model if the number of desktop has decreased
        for (int i = m_windows.count()-1; i >= removeIndex; i--) {
            windowsAt(i)->deleteLater();
            m_windows.removeAt(i);
        }
    }

    // append more windows model if the number of desktop has increased
    for (int i = m_windows.count(); i < rowCount(); i++)
        m_windows.append(new WindowModel(this));
}

void PagerModel::appendWindowRect(int desktopId, WId windowId, const QRectF &rect,
                                  bool active, const QPixmap &icon, const QString &name)
{
    WindowModel *windows = windowsAt(desktopId);
    if (!windows)
        return;

    windows->append(windowId, rect, active, icon, name);

    QModelIndex i = index(desktopId);
    emit dataChanged(i, i);
}

QVariant PagerModel::data(const QModelIndex &index, int role) const
{
    if (role >= RectangleModel::WidthRole && role < WindowsRole)
        return m_desktops.data(index, role);

    if (index.row() < 0 || index.row() >= m_windows.count())
        return QVariant();

    switch (role) {
    case WindowsRole:
        return QVariant::fromValue(m_windows[index.row()]);
    case DesktopNameRole:
        return m_names[index.row()];
    default:
        return QVariant();
    }
}

int PagerModel::rowCount(const QModelIndex &index) const
{
    return m_desktops.rowCount(index);
}
