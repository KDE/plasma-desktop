/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2014 Vishesh Handa <me@vhanda.in>
    SPDX-FileCopyrightText: 2019 Tomaz Canabrava <tcanabrava@kde.org>
    SPDX-FileCopyrightText: 2020 Benjamin Port <benjamin.port@enioka.com>

    SPDX-License-Identifier: LGPL-2.1-or-later

*/

#include "filteredfoldermodel.h"

#include <QIcon>

#include <KLocalizedString>
#include <QDir>
#include <QStringList>
#include <QTimer>
#include <QUrl>

#include "baloo/baloosettings.h"

namespace
{
QString normalizeTrailingSlashes(QString &&input)
{
    if (!input.endsWith('/'))
        return input + QLatin1Char('/');
    return input;
}

QStringList addTrailingSlashes(QStringList &&list)
{
    for (QString &str : list) {
        str = normalizeTrailingSlashes(std::move(str));
    }
    return list;
}

QString makeHomePretty(const QString &url)
{
    if (url.startsWith(QDir::homePath()))
        return QString(url).replace(0, QDir::homePath().length(), QStringLiteral("~"));
    return url;
}
}

FilteredFolderModel::FilteredFolderModel(BalooSettings *settings, QObject *parent)
    : QAbstractListModel(parent)
    , m_settings(settings)
{
}

void FilteredFolderModel::updateDirectoryList()
{
    beginResetModel();

    const QStringList runtimeExcluded = m_runtimeConfig.excludeFolders();

    QStringList settingsIncluded = addTrailingSlashes(m_settings->folders());
    QStringList settingsExcluded = addTrailingSlashes(m_settings->excludedFolders());

    const QString homePath = normalizeTrailingSlashes(QDir::homePath());

    auto folderListEntry = [&homePath](const QString &url, bool include, bool fromConfig) {
        QString displayName = url;
        if (displayName.size() > 1) {
            displayName.chop(1);
        }
        if (displayName.startsWith(homePath)) {
            displayName.replace(0, homePath.length(), QStringLiteral("~/"));
        }

        QString icon = QStringLiteral("folder");
        if (url == homePath) {
            icon = QStringLiteral("user-home");
        } else if (!fromConfig) {
            icon = QStringLiteral("drive-harddisk");
        }

        return FolderInfo{url, displayName, icon, include, fromConfig};
    };
    m_folderList.clear();

    for (const QString &folder : settingsIncluded) {
        m_folderList.append(folderListEntry(folder, true, true));
    }
    for (const QString &folder : settingsExcluded) {
        m_folderList.append(folderListEntry(folder, false, true));
    }

    // Add any automatically excluded mounts to the list
    for (const QString &folder : runtimeExcluded) {
        if (settingsIncluded.contains(folder) || settingsExcluded.contains(folder)) {
            // Do not add any duplicates
            continue;
        }
        if (m_deletedSettings.contains(folder)) {
            // Skip entries deleted from the config
            continue;
        }
        m_folderList.append(folderListEntry(folder, false, false));
    }

    std::sort(m_folderList.begin(), m_folderList.end(), [](const FolderInfo &a, const FolderInfo &b) {
        return a.url < b.url;
    });

    endResetModel();
}

QVariant FilteredFolderModel::data(const QModelIndex &idx, int role) const
{
    if (!idx.isValid() || idx.row() >= m_folderList.size()) {
        return {};
    }

    const auto entry = m_folderList.at(idx.row());
    switch (role) {
    case Qt::DisplayRole:
        return entry.displayName;
    case Qt::WhatsThisRole:
        return entry.url;
    case Qt::DecorationRole:
        return entry.icon;
    case Qt::ToolTipRole:
        return makeHomePretty(entry.url);
    case Url:
        return entry.url;
    case Folder:
        return entry.displayName;
    case EnableIndex:
        return entry.enableIndex;
    case Deletable:
        return entry.isFromConfig && entry.url != normalizeTrailingSlashes(QDir::homePath());
    default:
        return {};
    }
}

bool FilteredFolderModel::setData(const QModelIndex &idx, const QVariant &value, int role)
{
    if (!idx.isValid() || idx.row() >= m_folderList.size()) {
        return false;
    }
    FolderInfo &entry = m_folderList[idx.row()];
    if (role == EnableIndex) {
        entry.enableIndex = value.toBool();
        syncFolderConfig(entry);
        Q_EMIT dataChanged(idx, idx);
        return true;
    }
    return false;
}

int FilteredFolderModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_folderList.count();
}

void FilteredFolderModel::addFolder(const QString &url, const bool included = false)
{
    QString nUrl = normalizeTrailingSlashes(QUrl(url).toLocalFile());

    auto it = std::find_if(m_folderList.begin(), m_folderList.end(), [nUrl](const FolderInfo &folder) {
        return folder.url == nUrl;
    });
    if (it != m_folderList.end() && (*it).isFromConfig) {
        return;
    }
    if (included) {
        auto included = addTrailingSlashes(m_settings->folders());
        included.append(nUrl);
        std::sort(std::begin(included), std::end(included));
        m_settings->setFolders(included);
    } else {
        auto excluded = addTrailingSlashes(m_settings->excludedFolders());
        excluded.append(nUrl);
        std::sort(std::begin(excluded), std::end(excluded));
        m_settings->setExcludedFolders(excluded);
    }
    m_deletedSettings.removeAll(nUrl);
}

void FilteredFolderModel::removeFolder(int row)
{
    auto entry = m_folderList.at(row);
    if (!entry.isFromConfig) {
        return;
    }
    if (!entry.enableIndex) {
        auto excluded = addTrailingSlashes(m_settings->excludedFolders());
        excluded.removeAll(entry.url);
        std::sort(std::begin(excluded), std::end(excluded));
        m_settings->setExcludedFolders(excluded);
    } else {
        auto included = addTrailingSlashes(m_settings->folders());
        included.removeAll(entry.url);
        std::sort(std::begin(included), std::end(included));
        m_settings->setFolders(included);
    }
    m_deletedSettings.append(entry.url);
}

void FilteredFolderModel::syncFolderConfig(const FolderInfo &entry)
{
    auto excluded = addTrailingSlashes(m_settings->excludedFolders());
    auto included = addTrailingSlashes(m_settings->folders());
    if (entry.enableIndex) {
        included.append(entry.url);
        std::sort(std::begin(included), std::end(included));
        if (excluded.removeAll(entry.url)) {
            std::sort(std::begin(excluded), std::end(excluded));
            m_settings->setExcludedFolders(excluded);
        }
        m_settings->setFolders(included);
    } else {
        excluded.append(entry.url);
        std::sort(std::begin(excluded), std::end(excluded));
        if (included.removeAll(entry.url)) {
            std::sort(std::begin(included), std::end(included));
            m_settings->setFolders(included);
        }
        m_settings->setExcludedFolders(excluded);
    }
}

QHash<int, QByteArray> FilteredFolderModel::roleNames() const
{
    return {{Url, "url"}, //
            {Folder, "folder"},
            {EnableIndex, "enableIndex"},
            {Deletable, "deletable"},
            {Qt::DecorationRole, "decoration"}};
}
