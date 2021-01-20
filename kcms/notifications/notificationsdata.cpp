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

#include "notificationsdata.h"

#include <notificationmanager/badgesettings.h>
#include <notificationmanager/behaviorsettings.h>
#include <notificationmanager/donotdisturbsettings.h>
#include <notificationmanager/jobsettings.h>
#include <notificationmanager/notificationsettings.h>

NotificationsData::NotificationsData(QObject *parent, const QVariantList &args)
    : KCModuleData(parent, args)
    , m_dndSettings(new NotificationManager::DoNotDisturbSettings(this))
    , m_notificationSettings(new NotificationManager::NotificationSettings(this))
    , m_jobSettings(new NotificationManager::JobSettings(this))
    , m_badgeSettings(new NotificationManager::BadgeSettings(this))
{
    autoRegisterSkeletons();
    readBehaviorSettings();
}

NotificationManager::DoNotDisturbSettings *NotificationsData::dndSettings() const
{
    return m_dndSettings;
}

NotificationManager::NotificationSettings *NotificationsData::notificationSettings() const
{
    return m_notificationSettings;
}

NotificationManager::JobSettings *NotificationsData::jobSettings() const
{
    return m_jobSettings;
}

NotificationManager::BadgeSettings *NotificationsData::badgeSettings() const
{
    return m_badgeSettings;
}

NotificationManager::BehaviorSettings *NotificationsData::behaviorSettings(int index) const
{
    return m_behaviorSettingsList.value(index);
}

void NotificationsData::insertBehaviorSettings(int index, NotificationManager::BehaviorSettings *settings)
{
    m_behaviorSettingsList[index] = settings;
}

void NotificationsData::loadBehaviorSettings()
{
    for (auto *behaviorSettings : qAsConst(m_behaviorSettingsList)) {
        behaviorSettings->load();
    }
}

void NotificationsData::saveBehaviorSettings()
{
    for (auto *behaviorSettings : qAsConst(m_behaviorSettingsList)) {
        behaviorSettings->save();
    }
}

void NotificationsData::defaultsBehaviorSettings()
{
    for (auto *behaviorSettings : qAsConst(m_behaviorSettingsList)) {
        behaviorSettings->setDefaults();
    }
}

bool NotificationsData::isSaveNeededBehaviorSettings() const
{
    bool needSave = std::any_of(m_behaviorSettingsList.cbegin(), m_behaviorSettingsList.cend(), [](const NotificationManager::BehaviorSettings *settings) {
        return settings->isSaveNeeded();
    });
    return needSave;
}

bool NotificationsData::isDefaultsBehaviorSettings() const
{
    bool notDefault = std::any_of(m_behaviorSettingsList.cbegin(), m_behaviorSettingsList.cend(), [](const NotificationManager::BehaviorSettings *settings) {
        return !settings->isDefaults();
    });
    return !notDefault;
}

void NotificationsData::readBehaviorSettings()
{
    KConfig config("plasmanotifyrc", KConfig::SimpleConfig);

    for (auto groupEntry : {QStringLiteral("Applications"), QStringLiteral("Services")}) {
        KConfigGroup group(&config, groupEntry);
        for (const QString &desktopEntry : group.groupList()) {
            m_behaviorSettingsList.insert(m_behaviorSettingsList.count(), new NotificationManager::BehaviorSettings(groupEntry, desktopEntry, this));
        }
    }
}

bool NotificationsData::isDefaults() const
{
    return KCModuleData::isDefaults() && isDefaultsBehaviorSettings();
}
