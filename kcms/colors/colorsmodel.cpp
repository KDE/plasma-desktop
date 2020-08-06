/*
 * Copyright (C) 2007 Matthew Woehlke <mw_triad@users.sourceforge.net>
 * Copyright (C) 2007 Jeremy Whiting <jpwhiting@kde.org>
 * Copyright (C) 2016 Olivier Churlaud <olivier@churlaud.com>
 * Copyright (C) 2019 Kai Uwe Broulik <kde@privat.broulik.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "colorsmodel.h"

#include <QCollator>
#include <QDir>
#include <QStandardPaths>

#include <KColorScheme>
#include <KConfigGroup>
#include <KSharedConfig>

#include <algorithm>

ColorsModel::ColorsModel(QObject *parent) : QAbstractListModel(parent)
{

}

ColorsModel::~ColorsModel() = default;

int ColorsModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return m_data.count();
}

QVariant ColorsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_data.count()) {
        return QVariant();
    }

    const auto &item = m_data.at(index.row());

    switch (role) {
    case Qt::DisplayRole: return item.display;
    case SchemeNameRole: return item.schemeName;
    case PaletteRole: return item.palette;
    case ActiveTitleBarBackgroundRole: return item.activeTitleBarBackground;
    case ActiveTitleBarForegroundRole: return item.activeTitleBarForeground;
    case PendingDeletionRole: return item.pendingDeletion;
    case RemovableRole: return item.removable;
    }

    return QVariant();
}

bool ColorsModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || index.row() >= m_data.count()) {
        return false;
    }

    if (role == PendingDeletionRole) {
        auto &item = m_data[index.row()];

        const bool pendingDeletion = value.toBool();

        if (item.pendingDeletion != pendingDeletion) {
            item.pendingDeletion = pendingDeletion;
            emit dataChanged(index, index, {PendingDeletionRole});

            if (index.row() == selectedSchemeIndex() && pendingDeletion) {
                // move to the next non-pending theme
                const auto nonPending = match(index, PendingDeletionRole, false);
                if (!nonPending.isEmpty()) {
                    setSelectedScheme(nonPending.first().data(SchemeNameRole).toString());
                }
            }

            emit pendingDeletionsChanged();
            return true;
        }
    }

    return false;
}

QHash<int, QByteArray> ColorsModel::roleNames() const
{
    return {
        {Qt::DisplayRole, QByteArrayLiteral("display")},
        {SchemeNameRole, QByteArrayLiteral("schemeName")},
        {PaletteRole, QByteArrayLiteral("palette")},
        {ActiveTitleBarBackgroundRole, QByteArrayLiteral("activeTitleBarBackground")},
        {ActiveTitleBarForegroundRole, QByteArrayLiteral("activeTitleBarForeground")},
        {RemovableRole, QByteArrayLiteral("removable")},
        {PendingDeletionRole, QByteArrayLiteral("pendingDeletion")}
    };
}

QString ColorsModel::selectedScheme() const
{
    return m_selectedScheme;
}

void ColorsModel::setSelectedScheme(const QString &scheme)
{
    if (m_selectedScheme == scheme) {
        return;
    }

    m_selectedScheme = scheme;

    emit selectedSchemeChanged(scheme);
    emit selectedSchemeIndexChanged();
}

int ColorsModel::indexOfScheme(const QString &scheme) const
{
    auto it = std::find_if(m_data.begin(), m_data.end(), [ &scheme](const ColorsModelData &item) {
        return item.schemeName == scheme;
    });

    if (it != m_data.end()) {
        return std::distance(m_data.begin(), it);
    }

    return -1;
}

int ColorsModel::selectedSchemeIndex() const
{
    return indexOfScheme(m_selectedScheme);
}

void ColorsModel::load()
{
    beginResetModel();

    const int oldCount = m_data.count();

    m_data.clear();

    QStringList schemeFiles;

    const QStringList schemeDirs = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, QStringLiteral("color-schemes"), QStandardPaths::LocateDirectory);
    for (const QString &dir : schemeDirs) {
        const QStringList fileNames = QDir(dir).entryList(QStringList{QStringLiteral("*.colors")});
        for (const QString &file : fileNames) {
            const QString suffixedFileName = QLatin1String("color-schemes/") + file;
            // can't use QSet because of the transform below (passing const QString as this argument discards qualifiers)
            if (!schemeFiles.contains(suffixedFileName)) {
                schemeFiles.append(suffixedFileName);
            }
        }
    }

    std::transform(schemeFiles.begin(), schemeFiles.end(), schemeFiles.begin(), [](const QString &item) {
        return QStandardPaths::locate(QStandardPaths::GenericDataLocation, item);
    });

    for (const QString &schemeFile : schemeFiles) {
        const QFileInfo fi(schemeFile);
        const QString baseName = fi.baseName();

        KSharedConfigPtr config = KSharedConfig::openConfig(schemeFile, KConfig::SimpleConfig);
        KConfigGroup group(config, "General");
        const QString name = group.readEntry("Name", baseName);

        const QPalette palette = KColorScheme::createApplicationPalette(config);

        KColorScheme headerColorScheme(QPalette::Active, KColorScheme::Header, config);
        KConfigGroup wmConfig(config, QStringLiteral("WM"));
        const QColor activeTitleBarBackground = wmConfig.readEntry("activeBackground", headerColorScheme.background().color());
        const QColor activeTitleBarForeground = wmConfig.readEntry("activeForeground", headerColorScheme.foreground().color());

        ColorsModelData item{
            name,
            baseName,
            palette,
            activeTitleBarBackground,
            activeTitleBarForeground,
            fi.isWritable(),
            false, // pending deletion
        };

        m_data.append(item);
    }

    QCollator collator;
    std::sort(m_data.begin(), m_data.end(), [&collator](const ColorsModelData &a, const ColorsModelData &b) {
        return collator.compare(a.display, b.display) < 0;
    });

    endResetModel();

    // an item might have been added before the currently selected one
    if (oldCount != m_data.count()) {
        emit selectedSchemeIndexChanged();
    }
}

QStringList ColorsModel::pendingDeletions() const
{
    QStringList pendingDeletions;

    for (const auto &item : m_data) {
        if (item.pendingDeletion) {
            pendingDeletions.append(item.schemeName);
        }
    }

    return pendingDeletions;
}

void ColorsModel::removeItemsPendingDeletion()
{
    for (int i = m_data.count() - 1; i >= 0; --i) {
        if (m_data.at(i).pendingDeletion) {
            beginRemoveRows(QModelIndex(), i, i);
            m_data.remove(i);
            endRemoveRows();
        }
    }
}
