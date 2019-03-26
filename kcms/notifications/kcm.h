/*
 * Copyright (c) 2019 Kai Uwe Broulik <kde@privat.broulik.de>
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

#pragma once

#include <QScopedPointer>
#include <QPointer>

#include <KQuickAddons/ConfigModule>

class SourcesModel;
class FilterProxyModel;

namespace NotificationManager {
class Settings;
}

namespace KActivities {
class ActivitiesModel;
}

class KCMNotifications : public KQuickAddons::ConfigModule
{
    Q_OBJECT

    Q_PROPERTY(SourcesModel *sourcesModel READ sourcesModel CONSTANT)
    Q_PROPERTY(FilterProxyModel *filteredModel READ filteredModel CONSTANT)

    Q_PROPERTY(NotificationManager::Settings *settings READ settings CONSTANT)

    Q_PROPERTY(KActivities::ActivitiesModel *activitiesModel READ activitiesModel CONSTANT)

public:
    KCMNotifications(QObject *parent, const QVariantList &args);
    ~KCMNotifications() override;

    enum Roles {
        SchemeNameRole = Qt::UserRole + 1,
        PaletteRole,
        RemovableRole,
        PendingDeletionRole
    };

    SourcesModel *sourcesModel() const;
    FilterProxyModel *filteredModel() const;

    NotificationManager::Settings *settings() const;

    KActivities::ActivitiesModel *activitiesModel() const;

public Q_SLOTS:
    void load() override;
    void save() override;
    void defaults() override;

private:
    SourcesModel *m_sourcesModel;
    FilterProxyModel *m_filteredModel;

    NotificationManager::Settings *m_settings;

    KActivities::ActivitiesModel *m_activitiesModel;

};
