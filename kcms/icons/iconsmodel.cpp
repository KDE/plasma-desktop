/*
 * Copyright (c) 1999 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
 * Copyright (c) 2000 Antonio Larrosa <larrosa@kde.org>
 * Copyright (C) 2000 Geert Jansen <jansen@kde.org>
 * KDE Frameworks 5 port Copyright (C) 2013 Jonathan Riddell <jr@jriddell.org>
 * Copyright (C) 2018 Kai Uwe Broulik <kde@privat.broulik.de>
 *
 * Requires the Qt widget libraries, available at no cost at
 * http://www.troll.no/
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "iconsmodel.h"

#include <QFileIconProvider>

#include <KIconTheme>

IconsModel::IconsModel(QObject *parent) : QAbstractListModel(parent)
{

}

IconsModel::~IconsModel() = default;

int IconsModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return m_data.count();
}

QVariant IconsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_data.count()) {
        return QVariant();
    }

    const auto &item = m_data.at(index.row());

    switch (role) {
    case Qt::DisplayRole: return item.display;
    case ThemeNameRole: return item.themeName;
    case DescriptionRole: return item.description;
    case RemovableRole: return item.removable;
    case PendingDeletionRole: return item.pendingDeletion;
    }

    return QVariant();
}

bool IconsModel::setData(const QModelIndex &index, const QVariant &value, int role)
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

            // move to the next non-pending theme
            const auto nonPending = match(index, PendingDeletionRole, false);
            if (!nonPending.isEmpty()) {
                setSelectedTheme(nonPending.first().data(ThemeNameRole).toString());
            }

            emit pendingDeletionsChanged();
            return true;
        }
    }

    return false;
}

QHash<int, QByteArray> IconsModel::roleNames() const
{
    return {
        {Qt::DisplayRole, QByteArrayLiteral("display")},
        {DescriptionRole, QByteArrayLiteral("description")},
        {ThemeNameRole, QByteArrayLiteral("themeName")},
        {RemovableRole, QByteArrayLiteral("removable")},
        {PendingDeletionRole, QByteArrayLiteral("pendingDeletion")}
    };
}

QString IconsModel::selectedTheme() const
{
    return m_selectedTheme;
}

void IconsModel::setSelectedTheme(const QString &theme)
{
    if (m_selectedTheme == theme) {
        return;
    }

    const bool firstTime = m_selectedTheme.isNull();
    m_selectedTheme = theme;

    if (!firstTime) {
        emit selectedThemeChanged();
    }
    emit selectedThemeIndexChanged();
}

int IconsModel::selectedThemeIndex() const
{
    auto it = std::find_if(m_data.begin(), m_data.end(), [this](const IconsModelData &item) {
        return item.themeName == m_selectedTheme;
    });

    if (it != m_data.end()) {
        return std::distance(m_data.begin(), it);
    }

    return -1;
}

void IconsModel::load()
{
    beginResetModel();

    const int oldCount = m_data.count();

    m_data.clear();

    const QStringList themes = KIconTheme::list();

    m_data.reserve(themes.count());

    for (const QString &themeName : themes) {
        KIconTheme theme(themeName);
        if (!theme.isValid()) {
            //qCWarning(KCM_ICONS) << "Not a valid theme" << themeName;
        }
        if (theme.isHidden()) {
            continue;
        }

        IconsModelData item{
            theme.name(),
            themeName,
            theme.description(),
            themeName != KIconTheme::defaultThemeName()
                && QFileInfo(theme.dir()).isWritable(),
            false // pending deletion
        };

        m_data.append(item);
    }

    std::sort(m_data.begin(), m_data.end(), [](const IconsModelData &a, const IconsModelData &b) {
        return a.display < b.display;
    });

    endResetModel();

    // an item might have been added before the currently selected one
    if (oldCount != m_data.count()) {
        emit selectedThemeIndexChanged();
    }
}

QStringList IconsModel::pendingDeletions() const
{
    QStringList pendingDeletions;

    for (const auto &item : m_data) {
        if (item.pendingDeletion) {
            pendingDeletions.append(item.themeName);
        }
    }

    return pendingDeletions;
}

void IconsModel::removeItemsPendingDeletion()
{
    for (int i = m_data.count() - 1; i >= 0; --i) {
        if (m_data.at(i).pendingDeletion) {
            beginRemoveRows(QModelIndex(), i, i);
            m_data.remove(i);
            endRemoveRows();
        }
    }
}
