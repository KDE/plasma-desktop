/*
    SPDX-FileCopyrightText: 2012-2016 Ivan Cukic <ivan.cukic@kde.org>
    SPDX-FileCopyrightText: 2022 Méven Car <meven.car@kdenet.net>
    SPDX-FileCopyrightText: 2026 James Graham <james.h.graham@protonmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "kcm_recentfiles.h"

#include "resourcescoring_interface.h"

using namespace Qt::StringLiterals;

K_PLUGIN_CLASS_WITH_JSON(RecentFilesKcm, "kcm_recentfiles.json")

namespace
{
constexpr inline auto ActivityManagerDBusKey = "org.kde.ActivityManager"_L1;
constexpr inline auto ResourceScoringPathKey = "/ActivityManager/Resources/Scoring"_L1;
}

RecentFilesKcm::RecentFilesKcm(QObject *parent, const KPluginMetaData &data)
    : KQuickConfigModule(parent, data)
    , m_settings(new KActivityManagerdPluginsSettings(this))
    , m_excludedApplicationsModel(new ExcludedApplicationsModel(this))
{
    connect(m_excludedApplicationsModel, &ExcludedApplicationsModel::changed, this, [this](bool changed) {
        if (changed) {
            setNeedsSave(true);
        }
    });
}

int RecentFilesKcm::whatToRemember() const
{
    return m_settings->whatToRemember();
}

void RecentFilesKcm::setWhatToRemember(int whatToRemember)
{
    if (whatToRemember == m_settings->whatToRemember()) {
        return;
    }
    m_settings->setWhatToRemember(whatToRemember);
    setNeedsSave(true);
    Q_EMIT whatToRememberChanged();
}

int RecentFilesKcm::keepHistoryFor() const
{
    return m_settings->keepHistoryFor();
}

void RecentFilesKcm::setKeepHistoryFor(int keepHistoryFor)
{
    if (keepHistoryFor == m_settings->keepHistoryFor()) {
        return;
    }
    m_settings->setKeepHistoryFor(keepHistoryFor);
    setNeedsSave(true);
    Q_EMIT keepHistoryForChanged();
}

bool RecentFilesKcm::blockByDefault() const
{
    return m_settings->blockedByDefault();
}

void RecentFilesKcm::setBlockByDefault(bool blockByDefault)
{
    if (blockByDefault == m_settings->blockedByDefault()) {
        return;
    }
    m_settings->setBlockedByDefault(blockByDefault);
    setNeedsSave(true);
    Q_EMIT blockByDefaultChanged();
}

ExcludedApplicationsModel *RecentFilesKcm::excludedApplicationsModel() const
{
    return m_excludedApplicationsModel;
}

void RecentFilesKcm::defaults()
{
    m_excludedApplicationsModel->defaults();
    setWhatToRemember(0);
}

void RecentFilesKcm::load()
{
    m_excludedApplicationsModel->load();
}

void RecentFilesKcm::save()
{
    m_excludedApplicationsModel->save();
    m_settings->save();
}

void RecentFilesKcm::forget(int count, const QString &what)
{
    org::kde::ActivityManager::ResourcesScoring rankingsservice(ActivityManagerDBusKey, ResourceScoringPathKey, QDBusConnection::sessionBus());
    rankingsservice.DeleteRecentStats(QString(), count, what);
}

#include "kcm_recentfiles.moc"
