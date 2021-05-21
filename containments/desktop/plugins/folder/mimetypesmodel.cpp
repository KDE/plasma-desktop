/*
    SPDX-FileCopyrightText: 2014 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "mimetypesmodel.h"

#include <QMimeDatabase>

static bool lessThan(const QMimeType &a, const QMimeType &b)
{
    return QString::localeAwareCompare(a.name(), b.name()) < 0;
}

MimeTypesModel::MimeTypesModel(QObject *parent)
    : QAbstractListModel(parent)
{
    QMimeDatabase db;
    m_mimeTypesList = db.allMimeTypes();
    std::stable_sort(m_mimeTypesList.begin(), m_mimeTypesList.end(), lessThan);

    m_checkedRows = QVector<bool>(m_mimeTypesList.size(), false);
}

MimeTypesModel::~MimeTypesModel()
{
}

QHash<int, QByteArray> MimeTypesModel::roleNames() const
{
    return {{Qt::DisplayRole, "comment"}, {Qt::UserRole, "name"}, {Qt::DecorationRole, "decoration"}, {Qt::CheckStateRole, "checked"}};
}

QVariant MimeTypesModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= m_mimeTypesList.size()) {
        return QVariant();
    }

    switch (role) {
    case Qt::DisplayRole:
        return m_mimeTypesList.at(index.row()).comment();
    case Qt::UserRole:
        return m_mimeTypesList.at(index.row()).name();

    case Qt::DecorationRole: {
        QString icon = m_mimeTypesList.at(index.row()).iconName();

        if (icon.isEmpty()) {
            icon = m_mimeTypesList.at(index.row()).genericIconName();
        }

        return icon;
    }

    case Qt::CheckStateRole:
        return m_checkedRows.at(index.row()) ? Qt::Checked : Qt::Unchecked;
    }

    return QVariant();
}

bool MimeTypesModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.row() < 0 || index.row() >= m_mimeTypesList.size()) {
        return false;
    }

    if (role == Qt::CheckStateRole) {
        const bool newChecked = value.toBool();
        if (m_checkedRows.at(index.row()) != newChecked) {
            m_checkedRows[index.row()] = newChecked;
            Q_EMIT dataChanged(index, index, {role});
            Q_EMIT checkedTypesChanged();
            return true;
        }
    }

    return false;
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

    for (int i = 0; i < m_checkedRows.size(); ++i) {
        if (m_checkedRows.at(i)) {
            list.append(m_mimeTypesList.at(i).name());
        }
    }

    if (!list.isEmpty()) {
        return list;
    }

    return QStringList(QLatin1String(""));
}

void MimeTypesModel::setCheckedTypes(const QStringList &list)
{
    m_checkedRows = QVector<bool>(m_mimeTypesList.size(), false);

    foreach (const QString &name, list) {
        const int row = indexOfType(name);

        if (row != -1) {
            m_checkedRows[row] = true;
        }
    }

    Q_EMIT dataChanged(index(0, 0), index(m_mimeTypesList.size() - 1, 0), {Qt::CheckStateRole});

    Q_EMIT checkedTypesChanged();
}
