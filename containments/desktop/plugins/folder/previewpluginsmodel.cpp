/*
    SPDX-FileCopyrightText: 2008 Fredrik Höglund <fredrik@kde.org>
    SPDX-FileCopyrightText: 2014 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "previewpluginsmodel.h"

#include <KServiceTypeTrader>

#include <KIO/PreviewJob>

#include <algorithm>

static bool lessThan(const KService::Ptr &a, const KService::Ptr &b)
{
    return QString::localeAwareCompare(a->name(), b->name()) < 0;
}

PreviewPluginsModel::PreviewPluginsModel(QObject *parent)
    : QAbstractListModel(parent)
{
    m_plugins = KServiceTypeTrader::self()->query(QStringLiteral("ThumbCreator"));
    std::stable_sort(m_plugins.begin(), m_plugins.end(), lessThan);

    m_checkedRows = QVector<bool>(m_plugins.size(), false);
}

PreviewPluginsModel::~PreviewPluginsModel()
{
}

QHash<int, QByteArray> PreviewPluginsModel::roleNames() const
{
    return {{Qt::DisplayRole, "display"}, //
            {Qt::CheckStateRole, "checked"}};
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

bool PreviewPluginsModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.row() < 0 || index.row() >= m_plugins.size()) {
        return false;
    }

    if (role == Qt::CheckStateRole) {
        m_checkedRows[index.row()] = value.toBool();
        Q_EMIT dataChanged(index, index, {role});
        return true;
    }

    return false;
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
    QStringList plugins = list;
    if (plugins.isEmpty()) {
        plugins = KIO::PreviewJob::defaultPlugins();
    }

    m_checkedRows = QVector<bool>(m_plugins.size(), false);

    for (const QString &name : qAsConst(plugins)) {
        const int row = indexOfPlugin(name);
        if (row != -1) {
            m_checkedRows[row] = true;
        }
    }

    Q_EMIT dataChanged(index(0, 0), index(m_plugins.size() - 1, 0), {Qt::CheckStateRole});

    Q_EMIT checkedPluginsChanged();
}

QStringList PreviewPluginsModel::checkedPlugins() const
{
    QStringList list;
    for (int i = 0; i < m_checkedRows.size(); ++i) {
        if (m_checkedRows.at(i)) {
            list.append(m_plugins.at(i)->desktopEntryName());
        }
    }

    const QStringList defaultPlugins = KIO::PreviewJob::defaultPlugins();

    // If the list of checked plugins is the default set, return an empty list
    // which denotes the default set.
    // This ensures newly installed thumbnails are always automatically enabled.
    if (std::is_permutation(list.constBegin(), list.constEnd(), defaultPlugins.begin())) {
        return QStringList();
    }

    return list;
}
