/*
 * Copyright (c) 2020 Cyril Rossi <cyril.rossi@enioka.com>
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

#ifndef NOTIFICATIONSDATA_H
#define NOTIFICATIONSDATA_H

#include <QObject>

#include <KCModuleData>

namespace NotificationManager
{
class DoNotDisturbSettings;
class NotificationSettings;
class JobSettings;
class BadgeSettings;
class BehaviorSettings;
}

class NotificationsData : public KCModuleData
{
    Q_OBJECT

public:
    explicit NotificationsData(QObject *parent = nullptr, const QVariantList &args = QVariantList());

    bool isDefaults() const override;

    NotificationManager::DoNotDisturbSettings *dndSettings() const;
    NotificationManager::NotificationSettings *notificationSettings() const;
    NotificationManager::JobSettings *jobSettings() const;
    NotificationManager::BadgeSettings *badgeSettings() const;

    NotificationManager::BehaviorSettings *behaviorSettings(int index) const;
    void insertBehaviorSettings(int index, NotificationManager::BehaviorSettings *settings);
    void loadBehaviorSettings();
    void saveBehaviorSettings();
    void defaultsBehaviorSettings();
    bool isSaveNeededBehaviorSettings() const;
    bool isDefaultsBehaviorSettings() const;

private:
    void readBehaviorSettings();

    NotificationManager::DoNotDisturbSettings *m_dndSettings;
    NotificationManager::NotificationSettings *m_notificationSettings;
    NotificationManager::JobSettings *m_jobSettings;
    NotificationManager::BadgeSettings *m_badgeSettings;
    QHash<int, NotificationManager::BehaviorSettings *> m_behaviorSettingsList;
};

#endif // NOTIFICATIONSDATA_H
