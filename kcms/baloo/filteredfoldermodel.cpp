/*
 * This file is part of the KDE Baloo Project
 * Copyright (C) 2014  Vishesh Handa <me@vhanda.in>
 * Copyright (C) 2019 Tomaz Canabrava <tcanabrava@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "filteredfoldermodel.h"

#include <Solid/Device>
#include <Solid/StorageAccess>
#include <Solid/StorageDrive>

#include <QIcon>

#include <QDir>
#include <QTimer>
#include <QUrl>
#include <KLocalizedString>
#include <QStringList>

namespace {
    QStringList addTrailingSlashes(const QStringList& input) {
        QStringList output = input;

        for (QString& str : output) {
            if (!str.endsWith(QDir::separator()))
                str.append(QDir::separator());
        }

        return output;
    }

    QString makeHomePretty(const QString& url) {
        if (url.startsWith(QDir::homePath()))
            return QString(url).replace(0, QDir::homePath().length(), QStringLiteral("~"));
        return url;
    }
}

FilteredFolderModel::FilteredFolderModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

void FilteredFolderModel::setDirectoryList(const QStringList& include, const QStringList& exclude)
{
    beginResetModel();

    m_mountPoints.clear();

    QList<Solid::Device> devices = Solid::Device::listFromType(Solid::DeviceInterface::StorageAccess);

    for (const Solid::Device& dev : devices) {
        const Solid::StorageAccess* sa = dev.as<Solid::StorageAccess>();
        if (!sa->isAccessible())
            continue;

        const QString mountPath = sa->filePath();
        if (!shouldShowMountPoint(mountPath))
            continue;

        m_mountPoints.append(mountPath);
    }
    m_mountPoints.append(QDir::homePath());
    m_mountPoints = addTrailingSlashes(m_mountPoints);

    QStringList includeList = addTrailingSlashes(include);

    m_excludeList = addTrailingSlashes(exclude);

    // This algorithm seems bogus. verify later.
    for (const QString& mountPath : m_mountPoints) {
        if (includeList.contains(mountPath))
            continue;

        if (exclude.contains(mountPath))
            continue;

        if (!m_excludeList.contains(mountPath)) {
            m_excludeList.append(mountPath);
        }
    }

    endResetModel();
}

QVariant FilteredFolderModel::data(const QModelIndex& idx, int role) const
{
    if (!idx.isValid() || idx.row() >= m_excludeList.size()) {
        return {};
    }

    const auto currentUrl = m_excludeList.at(idx.row());
    switch (role) {
        case Qt::DisplayRole: return folderDisplayName(currentUrl);
        case Qt::WhatsThisRole: return currentUrl;
        case Qt::DecorationRole: return QIcon::fromTheme(iconName(currentUrl));
        case Qt::ToolTipRole: return makeHomePretty(currentUrl);
        case Url: return currentUrl;
        case Folder: return  folderDisplayName(currentUrl);
        default:
            return {};
    }

    return {};
 }

int FilteredFolderModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return m_excludeList.count();
}

QStringList FilteredFolderModel::includeFolders() const
{
    const QSet<QString> mountPointSet =
          QSet<QString>::fromList(m_mountPoints)
        - QSet<QString>::fromList(m_excludeList);

    return mountPointSet.values();
}

QStringList FilteredFolderModel::excludeFolders() const
{
    return m_excludeList;
}

QString FilteredFolderModel::fetchMountPoint(const QString& url) const
{
    QString mountPoint;

    for (const QString& mount : qAsConst(m_mountPoints)) {
        if (url.startsWith(mount) && mount.size() > mountPoint.size())
            mountPoint = mount;
    }

    return mountPoint;
}


void FilteredFolderModel::addFolder(const QString& url)
{
    if (m_excludeList.contains(url)) {
        return;
    }
    beginResetModel();
    m_excludeList.append(QUrl(url).toLocalFile());
    std::sort(std::begin(m_excludeList), std::end(m_excludeList));
    endResetModel();
    Q_EMIT folderAdded();
}

void FilteredFolderModel::removeFolder(int row)
{
    beginRemoveRows(QModelIndex(), row, row);
    m_excludeList.removeAt(row);
    endRemoveRows();
    Q_EMIT folderRemoved();
}


QString FilteredFolderModel::folderDisplayName(const QString& url) const
{
    QString name = url;

    // Check Home Dir
    QString homePath = QDir::homePath() + QLatin1Char('/');
    if (url == homePath) {
        return QDir(homePath).dirName();
    }

    if (url.startsWith(homePath)) {
        name = url.mid(homePath.size());
    }
    else {
        // Check Mount allMountPointsExcluded
        for (QString mountPoint : m_mountPoints) {
            if (url.startsWith(mountPoint)) {
                name = QLatin1Char('[') + mountPoint+ QLatin1String("]/") + url.mid(mountPoint.length());
                break;
            }
        }
    }

    if (name.endsWith(QLatin1Char('/'))) {
        name = name.mid(0, name.size() - 1);
    }
    return name;
}

bool FilteredFolderModel::shouldShowMountPoint(const QString& mountPoint)
{
    if (mountPoint == QLatin1String("/"))
        return false;

    if (mountPoint.startsWith(QLatin1String("/boot")))
        return false;

    if (mountPoint.startsWith(QLatin1String("/tmp")))
        return false;

    // The user's home directory is forcibly added so we can ignore /home
    // if /home actually contains the home directory
    return !(mountPoint.startsWith(QLatin1String("/home")) || !QDir::homePath().startsWith(QLatin1String("/home")));
}

QHash<int, QByteArray> FilteredFolderModel::roleNames() const
{
    return {
        {Url, "url"},
        {Folder, "folder"},
        {Qt::DecorationRole, "decoration"}
    };
}

QString FilteredFolderModel::iconName(QString path) const
{
    // Ensure paths end with /
    if (!path.endsWith(QDir::separator()))
        path.append(QDir::separator());

    QString homePath = QDir::homePath();
    if (!homePath.endsWith(QDir::separator()))
        homePath.append(QDir::separator());

    if (path == homePath)
        return QStringLiteral("user-home");

    if (m_mountPoints.contains(path))
        return QStringLiteral("drive-harddisk");

    return QStringLiteral("folder");
}

