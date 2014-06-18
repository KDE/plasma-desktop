/***************************************************************************
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

#include "mimetypesmodel.h"

#include <QMimeDatabase>

static bool lessThan(const QMimeType &a, const QMimeType &b)
{
    return QString::localeAwareCompare(a.name(), b.name()) < 0;
}

MimeTypesModel::MimeTypesModel(QObject *parent) : QAbstractListModel(parent)
{
    QHash<int, QByteArray> roleNames;
    roleNames[Qt::DisplayRole] = "display";
    roleNames[Qt::DecorationRole] = "decoration";
    roleNames[Qt::CheckStateRole] = "checked";
    setRoleNames(roleNames);

    QMimeDatabase db;
    m_mimeTypesList = db.allMimeTypes();
    qStableSort(m_mimeTypesList.begin(), m_mimeTypesList.end(), lessThan);

    checkedRows = QVector<bool>(m_mimeTypesList.size(), false);
}

MimeTypesModel::~MimeTypesModel()
{
}

QVariant MimeTypesModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= m_mimeTypesList.size()) {
        return QVariant();
    }

    switch (role) {
        case Qt::DisplayRole:
            return m_mimeTypesList.at(index.row()).name();

        case Qt::DecorationRole:
        {
            QString icon = m_mimeTypesList.at(index.row()).iconName();

            if (icon.isEmpty()) {
                icon = m_mimeTypesList.at(index.row()).genericIconName();
            }

            return icon;
        }

        case Qt::CheckStateRole:
            return checkedRows.at(index.row()) ? Qt::Checked : Qt::Unchecked;
    }

    return QVariant();
}

void MimeTypesModel::setRowChecked(int row, bool checked)
{
    checkedRows[row] = checked;

    QModelIndex idx = index(row, 0);

    emit dataChanged(idx, idx);
}

void MimeTypesModel::checkAll()
{
    checkedRows = QVector<bool>(m_mimeTypesList.size(), true);

    emit dataChanged(index(0, 0), index(m_mimeTypesList.size() - 1, 0));
}

int MimeTypesModel::indexOfType(const QString &name) const
{
    for (int i = 0; i < m_mimeTypesList.size(); i++) {
        if (m_mimeTypesList.at(i).name() == name) {
            return i;
        }
    }
    return -1;
}

QStringList MimeTypesModel::checkedTypes() const
{
    QStringList list;

    for (int i =0; i < checkedRows.size(); ++i) {
        if (checkedRows.at(i)) {
            list.append(m_mimeTypesList.at(i).name());
        }
    }

    if (!list.isEmpty()) {
        return list;
    }

    return QStringList("");
}

void MimeTypesModel::setCheckedTypes(const QStringList &list)
{
    checkedRows = QVector<bool>(m_mimeTypesList.size(), false);

    foreach (const QString &name, list) {
        const int row = indexOfType(name);

        if (row != -1) {
            checkedRows[row] = true;
        }
    }

    emit dataChanged(index(0, 0), index(m_mimeTypesList.size() - 1, 0));

    emit checkedTypesChanged();
}

FilterableMimeTypesModel::FilterableMimeTypesModel(QObject *parent) : QSortFilterProxyModel(parent),
    m_sourceModel(new MimeTypesModel(this))
{
    setSourceModel(m_sourceModel);
    setDynamicSortFilter(true);

    connect(m_sourceModel, SIGNAL(checkedTypesChanged()), this, SIGNAL(checkedTypesChanged()));
}

FilterableMimeTypesModel::~FilterableMimeTypesModel()
{
}

void FilterableMimeTypesModel::setRowChecked(int row, bool checked)
{
    QModelIndex idx = index(row, 0);

    m_sourceModel->setRowChecked(mapToSource(idx).row(), checked);
}

void FilterableMimeTypesModel::checkAll()
{
    m_sourceModel->checkAll();
}

QStringList FilterableMimeTypesModel::checkedTypes() const
{
    return m_sourceModel->checkedTypes();
}

void FilterableMimeTypesModel::setCheckedTypes(const QStringList &list)
{
    m_sourceModel->setCheckedTypes(list);
}

QString FilterableMimeTypesModel::filter() const
{
    return m_filter;
}

void FilterableMimeTypesModel::setFilter(const QString &filter)
{
    if (m_filter != filter) {
        m_filter = filter;

        setFilterFixedString(m_filter);

        emit filterChanged();
    }
}

