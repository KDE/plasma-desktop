/*
    SPDX-FileCopyrightText: 2014 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "placesmodel.h"

#include <QStandardPaths>

#include <KFilePlacesModel>
#include <KPluginMetaData>

PlacesModel::PlacesModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    m_sourceModel = new KFilePlacesModel(this);

    connect(m_sourceModel, &KFilePlacesModel::rowsInserted, this, &PlacesModel::placesChanged);
    connect(m_sourceModel, &KFilePlacesModel::rowsRemoved, this, &PlacesModel::placesChanged);

    setSourceModel(m_sourceModel);

    setDynamicSortFilter(true);
}

PlacesModel::~PlacesModel()
{
}

QHash<int, QByteArray> PlacesModel::roleNames() const
{
    QHash<int, QByteArray> roleNames = QSortFilterProxyModel::roleNames();
    roleNames[Qt::DisplayRole] = "display";
    roleNames[Qt::DecorationRole] = "decoration";
    return roleNames;
}

bool PlacesModel::activityLinkingEnabled() const
{
    KPluginMetaData plugin = KPluginMetaData::findPluginById("kf5/kfileitemaction", QStringLiteral("kactivitymanagerd_fileitem_linking_plugin"));
    return plugin.isValid();
}

bool PlacesModel::showDesktopEntry() const
{
    return m_showDesktopEntry;
}

void PlacesModel::setShowDesktopEntry(bool showDesktopEntry)
{
    if (m_showDesktopEntry != showDesktopEntry) {
        m_showDesktopEntry = showDesktopEntry;

        invalidateFilter();

        Q_EMIT showDesktopEntryChanged();
    }
}

QString PlacesModel::urlForIndex(int idx) const
{
    return m_sourceModel->url(mapToSource(index(idx, 0))).toString();
}

int PlacesModel::indexForUrl(const QString &url) const
{
    QUrl _url(url);
    QModelIndex idx;

    for (int i = 0; i < rowCount(); i++) {
        if (_url == m_sourceModel->url(mapToSource(index(i, 0)))) {
            idx = index(i, 0);
            break;
        }
    }

    if (idx.isValid()) {
        return idx.row();
    }

    return -1;
}

bool PlacesModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    const QModelIndex index = m_sourceModel->index(sourceRow, 0, sourceParent);

    if (!m_showDesktopEntry) {
        const QUrl url = index.data(KFilePlacesModel::UrlRole).toUrl();
        const QUrl desktopUrl = QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::DesktopLocation));
        if (url == desktopUrl) {
            return false;
        }
    }

    return !m_sourceModel->isHidden(index);
}
