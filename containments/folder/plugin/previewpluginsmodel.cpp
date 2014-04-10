/***************************************************************************
 *   Copyright (C) 2008 Fredrik Höglund <fredrik@kde.org>                  *
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

#include "previewpluginsmodel.h"

#include <KServiceTypeTrader>

static bool lessThan(const KService::Ptr &a, const KService::Ptr &b)
{
    return QString::localeAwareCompare(a->name(), b->name()) < 0;
}

PreviewPluginsModel::PreviewPluginsModel(QObject *parent)
    : QAbstractListModel(parent)
{
    QHash<int, QByteArray> roleNames;
    roleNames[Qt::DisplayRole] = "display";
    roleNames[Qt::CheckStateRole] = "checked";
    setRoleNames(roleNames);

    m_plugins = KServiceTypeTrader::self()->query("ThumbCreator");
    qStableSort(m_plugins.begin(), m_plugins.end(), lessThan);

    m_checkedRows = QVector<bool>(m_plugins.size(), false);
}

PreviewPluginsModel::~PreviewPluginsModel()
{
}

QVariant PreviewPluginsModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= m_plugins.size()) {
        return QVariant();
    }

    switch (role) {
        case Qt::DisplayRole:
            return m_plugins.at(index.row())->name();

        case Qt::CheckStateRole:
            return m_checkedRows.at(index.row()) ? Qt::Checked : Qt::Unchecked;
    }

    return QVariant();
}

void PreviewPluginsModel::setRowChecked(int row, bool checked)
{
    m_checkedRows[row] = checked;

    QModelIndex idx = index(row, 0);

    emit dataChanged(idx, idx);
}

int PreviewPluginsModel::indexOfPlugin(const QString &name) const
{
    for (int i = 0; i < m_plugins.size(); i++) {
        if (m_plugins.at(i)->desktopEntryName() == name) {
            return i;
        }
    }
    return -1;
}

void PreviewPluginsModel::setCheckedPlugins(const QStringList &list)
{
    m_checkedRows = QVector<bool>(m_plugins.size(), false);

    foreach (const QString &name, list) {
        const int row = indexOfPlugin(name);
        if (row != -1) {
            m_checkedRows[row] = true;
        }
    }

    emit dataChanged(index(0, 0), index(m_plugins.size() - 1, 0));

    emit checkedPluginsChanged();
}

QStringList PreviewPluginsModel::checkedPlugins() const
{
    QStringList list;
    for (int i =0; i < m_checkedRows.size(); ++i) {
        if (m_checkedRows.at(i)) {
            list.append(m_plugins.at(i)->desktopEntryName());
        }
    }
    return list;
}
