/*
    SPDX-FileCopyrightText: 2016, 2019 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "smartlauncherbackend.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusServiceWatcher>
#include <QDebug>

#include <KConfigGroup>
#include <KService>
#include <KSharedConfig>

#include <algorithm>

#include "log_settings.h"
#include <settings.h>

using namespace SmartLauncher;
using namespace NotificationManager;

Backend::Backend(QObject *parent)
    : QObject(parent)
    , m_watcher(new QDBusServiceWatcher(this))
    , m_jobsModel(nullptr)
    , m_settings(new Settings(this))
{
    setupUnity();

    reload();
    connect(m_settings, &Settings::settingsChanged, this, &Backend::reload);
}

Backend::~Backend() = default;

void Backend::reload()
{
    m_badgeBlacklist = m_settings->badgeBlacklistedApplications();

    // Unity Launcher API operates on storage IDs ("foo.desktop"), whereas settings return desktop entries "foo"
    std::transform(m_badgeBlacklist.begin(), m_badgeBlacklist.end(), m_badgeBlacklist.begin(), [](const QString &desktopEntry) -> QString {
        return desktopEntry + QStringLiteral(".desktop");
    });

    if (!m_jobsModel) {
        m_jobsModel = JobsModel::createJobsModel();
        m_jobsModel->init();
    }

    Q_EMIT reloadRequested(QString() /*all*/);
}

bool Backend::doNotDisturbMode() const
{
    return m_settings->notificationsInhibitedByApplication()
        || (m_settings->notificationsInhibitedUntil().isValid() && m_settings->notificationsInhibitedUntil() > QDateTime::currentDateTimeUtc());
}

void Backend::setupUnity()
{
    auto sessionBus = QDBusConnection::sessionBus();

    if (!sessionBus.connect({},
                            {},
                            QStringLiteral("com.canonical.Unity.LauncherEntry"),
                            QStringLiteral("Update"),
                            this,
                            SLOT(update(QString, QMap<QString, QVariant>)))) {
        qCWarning(TASKMANAGER_DEBUG) << "failed to register Update signal";
        return;
    }

    if (!sessionBus.registerObject(QStringLiteral("/Unity"), this)) {
        qCWarning(TASKMANAGER_DEBUG) << "Failed to register unity object";
        return;
    }

    if (!sessionBus.registerService(QStringLiteral("com.canonical.Unity"))) {
        qCWarning(TASKMANAGER_DEBUG) << "Failed to register unity service";
        // In case an external process uses this (e.g. Latte Dock), let it just listen.
    }

    KConfigGroup grp(KSharedConfig::openConfig(QStringLiteral("taskmanagerrulesrc")), QStringLiteral("Unity Launcher Mapping"));

    const QStringList keys = grp.keyList();
    for (const QString &key : keys) {
        const QString &value = grp.readEntry(key, QString());
        if (value.isEmpty()) {
            continue;
        }

        m_unityMappingRules.insert(key, value);
    }
}

bool Backend::hasLauncher(const QString &storageId) const
{
    return m_launchers.contains(storageId);
}

int Backend::count(const QString &uri) const
{
    if (!m_settings->badgesInTaskManager() || doNotDisturbMode() || m_badgeBlacklist.contains(uri)) {
        return 0;
    }
    return m_launchers.value(uri).count;
}

bool Backend::countVisible(const QString &uri) const
{
    if (!m_settings->badgesInTaskManager() || doNotDisturbMode() || m_badgeBlacklist.contains(uri)) {
        return false;
    }
    return m_launchers.value(uri).countVisible;
}

int Backend::progress(const QString &uri) const
{
    return m_launchers.value(uri).progress;
}

bool Backend::progressVisible(const QString &uri) const
{
    return m_launchers.value(uri).progressVisible;
}

bool Backend::urgent(const QString &uri) const
{
    return m_launchers.value(uri).urgent;
}

QHash<QString, QString> Backend::unityMappingRules() const
{
    return m_unityMappingRules;
}

void Backend::update(const QString &uri, const QMap<QString, QVariant> &properties)
{
    Q_ASSERT(calledFromDBus());

    QString storageId;

    auto foundStorageId = m_launcherUrlToStorageId.constFind(uri);
    if (foundStorageId == m_launcherUrlToStorageId.constEnd()) { // we don't know this one, register
        // According to Unity Launcher API documentation applications *should* send along their
        // desktop file name with application:// prefix
        const QString applicationSchemePrefix = QStringLiteral("application://");

        QString normalizedUri = uri;
        if (normalizedUri.startsWith(applicationSchemePrefix)) {
            normalizedUri = uri.mid(applicationSchemePrefix.length());
        }

        KService::Ptr service = KService::serviceByStorageId(normalizedUri);
        if (!service) {
            qCWarning(TASKMANAGER_DEBUG) << "Failed to find service for Unity Launcher" << uri;
            return;
        }

        storageId = service->storageId();
        m_launcherUrlToStorageId.insert(uri, storageId);

        m_dbusServiceToLauncherUrl.insert(message().service(), uri);
        m_watcher->addWatchedService(message().service());
    } else {
        storageId = *foundStorageId;
    }

    auto foundEntry = m_launchers.find(storageId);
    if (foundEntry == m_launchers.end()) { // we don't have it yet, create a new Entry
        Entry entry;
        foundEntry = m_launchers.insert(storageId, entry);
    }

    auto propertiesEnd = properties.constEnd();

    auto foundCount = properties.constFind(QStringLiteral("count"));
    if (foundCount != propertiesEnd) {
        qint64 newCount = foundCount->toLongLong();
        // 2 billion unread emails ought to be enough for anybody
        if (newCount < std::numeric_limits<int>::max()) {
            int saneCount = static_cast<int>(newCount);
            if (saneCount != foundEntry->count) {
                foundEntry->count = saneCount;
                Q_EMIT countChanged(storageId, saneCount);
            }
        }
    }

    updateLauncherProperty(storageId, properties, QStringLiteral("count"), &foundEntry->count, &Backend::count, &Backend::countChanged);
    updateLauncherProperty(storageId,
                           properties,
                           QStringLiteral("count-visible"),
                           &foundEntry->countVisible,
                           &Backend::countVisible,
                           &Backend::countVisibleChanged);

    // the API gives us progress as 0..1 double but we'll use percent to avoid unnecessary
    // changes when it just changed a fraction of a percent, hence not using our fancy updateLauncherProperty method
    auto foundProgress = properties.constFind(QStringLiteral("progress"));
    if (foundProgress != propertiesEnd) {
        const int oldSanitizedProgress = progress(storageId);

        foundEntry->progress = qRound(foundProgress->toDouble() * 100);

        const int newSanitizedProgress = progress(storageId);

        if (oldSanitizedProgress != newSanitizedProgress) {
            Q_EMIT progressChanged(storageId, newSanitizedProgress);
        }
    }

    updateLauncherProperty(storageId,
                           properties,
                           QStringLiteral("progress-visible"),
                           &foundEntry->progressVisible,
                           &Backend::progressVisible,
                           &Backend::progressVisibleChanged);
    updateLauncherProperty(storageId, properties, QStringLiteral("urgent"), &foundEntry->urgent, &Backend::urgent, &Backend::urgentChanged);
}

void Backend::onServiceUnregistered(const QString &service)
{
    const QString &launcherUrl = m_dbusServiceToLauncherUrl.take(service);
    if (launcherUrl.isEmpty()) {
        return;
    }

    const QString &storageId = m_launcherUrlToStorageId.take(launcherUrl);
    if (storageId.isEmpty()) {
        return;
    }

    m_launchers.remove(storageId);
    Q_EMIT launcherRemoved(storageId);
}

#include "moc_smartlauncherbackend.cpp"
