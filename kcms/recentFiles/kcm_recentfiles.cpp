/*
    SPDX-FileCopyrightText: 2012-2016 Ivan Cukic <ivan.cukic@kde.org>
    SPDX-FileCopyrightText: 2022 Méven Car <meven.car@kdenet.net>
    SPDX-FileCopyrightText: 2026 James Graham <james.h.graham@protonmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "kcm_recentfiles.h"

#include "kactivitiesdata.h"
#include "resourcescoring_interface.h"

using namespace Qt::StringLiterals;

K_PLUGIN_FACTORY_WITH_JSON(KCMRecentFilesFactory, "kcm_recentFiles.json", registerPlugin<RecentFilesKcm>(); registerPlugin<KActivitiesData>();)

namespace
{
constexpr inline auto ActivityManagerDBusKey = "org.kde.ActivityManager"_L1;
constexpr inline auto ResourceScoringPathKey = "/ActivityManager/Resources/Scoring"_L1;
}

RecentFilesKcm::RecentFilesKcm(QObject *parent, const KPluginMetaData &data)
    : KQuickManagedConfigModule(parent, data)
    , m_data(new KActivitiesData(this))
{
}

KActivityManagerdPluginsSettings *RecentFilesKcm::settings() const
{
    return m_data->settings();
    ;
}

void RecentFilesKcm::forget(int count, const QString &what)
{
    org::kde::ActivityManager::ResourcesScoring rankingsservice(ActivityManagerDBusKey, ResourceScoringPathKey, QDBusConnection::sessionBus());
    rankingsservice.DeleteRecentStats(QString(), count, what);
}

#include "kcm_recentfiles.moc"
