/*
 * Copyright (c) 2019 Benjamin Port <benjamin.port@enioka.com>
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

#include "iconsizecategorymodel.h"
#include <KLocalizedString>
#include <KIconLoader>

IconSizeCategoryModel::IconSizeCategoryModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_data({
        {QStringLiteral("desktopSize"), I18N_NOOP("Desktop"), QStringLiteral("Desktop"), KIconLoader::Desktop},
        {QStringLiteral("toolbarSize"), I18N_NOOP("Toolbar"), QStringLiteral("Toolbar"), KIconLoader::Toolbar},
        {QStringLiteral("mainToolbarSize"), I18N_NOOP("Main Toolbar"), QStringLiteral("MainToolbar"), KIconLoader::MainToolbar},
        {QStringLiteral("smallSize"), I18N_NOOP("Small Icons"), QStringLiteral("Small"), KIconLoader::Small},
        {QStringLiteral("panelSize"), I18N_NOOP("Panel"), QStringLiteral("Panel"), KIconLoader::Panel},
        {QStringLiteral("dialogSize"), I18N_NOOP("Dialogs"), QStringLiteral("Dialog"), KIconLoader::Dialog}
    })
{
}

IconSizeCategoryModel::~IconSizeCategoryModel() = default;

int IconSizeCategoryModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return m_data.count();
}

QVariant IconSizeCategoryModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_data.count()) {
        return QVariant();
    }

    const auto &item = m_data.at(index.row());

    switch (role) {
    case Qt::DisplayRole:
        return item.display;
    case ConfigKeyRole:
        return item.configKey;
    case ConfigSectionRole:
        return item.configSection;
    case  KIconLoaderGroupRole:
        return item.kIconloaderGroup;
    }

    return QVariant();
}

QHash<int, QByteArray> IconSizeCategoryModel::roleNames() const
{
    QHash<int, QByteArray> roleNames = QAbstractListModel::roleNames();
    roleNames[ConfigKeyRole] = QByteArrayLiteral("configKey");
    roleNames[ConfigSectionRole] = QByteArrayLiteral("configSectionRole");
    roleNames[KIconLoaderGroupRole] = QByteArrayLiteral("KIconLoaderGroup");
    return roleNames;
}
