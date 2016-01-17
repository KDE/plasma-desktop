/***************************************************************************
 *   Copyright (C) 2016 Kai Uwe Broulik <kde@privat.broulik.de>            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include "smartlauncherbackend.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusServiceWatcher>
#include <QDebug>

#include <Plasma/DataEngineConsumer>
#include <Plasma/DataEngine>

#include <KConfig>
#include <KService>

using namespace SmartLauncher;

Backend::Backend(QObject *parent)
    : QObject(parent)
    , m_watcher(new QDBusServiceWatcher(this))
    , m_dataEngineConsumer(new Plasma::DataEngineConsumer)
    , m_dataEngine(m_dataEngineConsumer->dataEngine(QStringLiteral("applicationjobs")))
{
    m_available = setupUnity();
    m_available = setupApplicationJobs() || m_available;
}

Backend::~Backend()
{
    delete m_dataEngineConsumer;
}

bool Backend::setupUnity()
{
    auto sessionBus = QDBusConnection::sessionBus();

    if (!sessionBus.registerService(QStringLiteral("com.canonical.Unity"))) {
        qWarning() << "Failed to register unity service";
        return false;
    }

    if (!sessionBus.registerObject(QStringLiteral("/Unity"), this)) {
        qWarning() << "Failed to register unity object";
        return false;
    }

    if (!sessionBus.connect({}, {}, QStringLiteral("com.canonical.Unity.LauncherEntry"),
                            QStringLiteral("Update"), this, SLOT(update(QString,QMap<QString,QVariant>)))) {
        qWarning() << "failed to register Update signal";
        return false;
    }

    KConfig cfg(QStringLiteral("taskmanagerrulesrc"));
    KConfigGroup grp(&cfg, QStringLiteral("Unity Launcher Mapping"));

    foreach (const QString &key, grp.keyList()) {
        const QString &value = grp.readEntry(key, QString());
        if (value.isEmpty()) {
            continue;
        }

        m_unityMappingRules.insert(key, value);
    }

    return true;
}

bool Backend::setupApplicationJobs()
{
    if (!m_dataEngine->isValid()) {
        qWarning() << "Failed to setup application jobs, data engine is not valid";
        return false;
    }

    const QStringList &sources = m_dataEngine->sources();
    for (const QString &source : sources) {
        onApplicationJobAdded(source);
    }

    connect(m_dataEngine, &Plasma::DataEngine::sourceAdded, this, &Backend::onApplicationJobAdded);
    connect(m_dataEngine, &Plasma::DataEngine::sourceRemoved, this, &Backend::onApplicationJobRemoved);

    return true;
}

bool Backend::available() const
{
    return m_available;
}

bool Backend::hasLauncher(const QString &storageId) const
{
    return m_launchers.contains(storageId);
}

int Backend::count(const QString &uri) const
{
    return m_launchers.value(uri).count;
}

bool Backend::countVisible(const QString &uri) const
{
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
        KService::Ptr service = KService::serviceByStorageId(uri);
        if (!service) {
            qWarning() << "Failed to find service for Unity Launcher" << uri;
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
                emit countChanged(storageId, saneCount);
            }
        }
    }

    updateLauncherProperty(storageId, properties, QStringLiteral("count"), &foundEntry->count, &Backend::countChanged);
    updateLauncherProperty(storageId, properties, QStringLiteral("count-visible"), &foundEntry->countVisible, &Backend::countVisibleChanged);

    // the API gives us progress as 0..1 double but we'll use percent to avoid unneccessary
    // changes when it just changed a fraction of a percent, hence not using our fancy updateLauncherProperty method
    auto foundProgress = properties.constFind(QStringLiteral("progress"));
    if (foundProgress != propertiesEnd) {
        int newProgress = qRound(foundProgress->toDouble() * 100);
        if (newProgress != foundEntry->progress) {
            foundEntry->progress = newProgress;
            emit progressChanged(storageId, newProgress);
        }
    }

    updateLauncherProperty(storageId, properties, QStringLiteral("progress-visible"), &foundEntry->progressVisible, &Backend::progressVisibleChanged);
    updateLauncherProperty(storageId, properties, QStringLiteral("urgent"), &foundEntry->urgent, &Backend::urgentChanged);
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
    emit launcherRemoved(storageId);
}

void Backend::onApplicationJobAdded(const QString &source)
{
    m_dataEngine->connectSource(source, this);
}

void Backend::onApplicationJobRemoved(const QString &source)
{
    m_dataEngine->disconnectSource(source, this);

    const QString &storageId = m_dataSourceToStorageId.take(source);
    if (storageId.isEmpty()) {
        return;
    }

    // remove job, calculate new percentage, or remove launcher if gone altogether
    auto &jobs = m_storageIdToJobs[storageId];
    jobs.removeOne(source);
    if (jobs.isEmpty()) {
        m_storageIdToJobs.remove(storageId);
    }

    m_jobProgress.remove(source);

    auto foundEntry = m_launchers.find(storageId);
    if (foundEntry == m_launchers.end()) {
        qWarning() << "Cannot remove application job" << source << "as we don't know" << storageId;
        return;
    }

    updateApplicationJobPercent(storageId, &*foundEntry);

    if (!foundEntry->progressVisible && !foundEntry->progress) {
        // no progress anymore whatsoever, remove entire launcher
        m_launchers.remove(storageId);
        emit launcherRemoved(storageId);
    }
}

void Backend::dataUpdated(const QString &sourceName, const Plasma::DataEngine::Data &data)
{
    QString storageId;

    auto foundStorageId = m_dataSourceToStorageId.constFind(sourceName);
    if (foundStorageId == m_dataSourceToStorageId.constEnd()) { // we don't know this one, register
        QString appName = data.value(QStringLiteral("appName")).toString();
        if (appName.isEmpty()) {
            qWarning() << "Application jobs got update for" << sourceName << "without app name";
            return;
        }

        KService::Ptr service = KService::serviceByStorageId(appName);
        if (!service) {
            appName.prepend(QStringLiteral("org.kde."));
            // HACK try to find a service with org.kde. notation
            service = KService::serviceByStorageId(appName);
            if (!service) {
                qWarning() << "Could not find service for job" << sourceName << "with app name" << appName;
                return;
            }
        }

        storageId = service->storageId();
        m_dataSourceToStorageId.insert(sourceName, storageId);
    } else {
        storageId = *foundStorageId;
    }

    auto foundEntry = m_launchers.find(storageId);
    if (foundEntry == m_launchers.end()) { // we don't have it yet, create new Entry
        Entry entry;
        foundEntry = m_launchers.insert(storageId, entry);
    }

    int percent = data.value(QStringLiteral("percentage"), 0).toInt();

    // setup everything and calculate new percentage
    auto &jobs = m_storageIdToJobs[storageId];
    if (!jobs.contains(sourceName)) {
        jobs.append(sourceName);
    }

    m_jobProgress.insert(sourceName, percent); // insert() overrides if exist

    updateApplicationJobPercent(storageId, &*foundEntry);
}

void Backend::updateApplicationJobPercent(const QString &storageId, Entry *entry)
{
    // basically get all jobs for the given storageId and calculate an average progress

    const auto &jobs = m_storageIdToJobs.value(storageId);
    qreal jobCount = jobs.count();

    int totalProgress = 0;
    for (const QString &job : jobs) {
        totalProgress += m_jobProgress.value(job, 0);
    }

    int progress = 0;
    if (jobCount > 0) {
        progress = qRound(totalProgress / jobCount);
    }

    bool visible = (jobCount > 0);

    if (entry->count != jobCount) {
        entry->count = jobCount;
        emit countChanged(storageId, jobCount);
    }

    if (entry->countVisible != visible) {
        entry->countVisible = visible;
        emit countVisibleChanged(storageId, visible);
    }

    if (entry->progress != progress) {
        entry->progress = progress;
        emit progressChanged(storageId, progress);
    }

    if (entry->progressVisible != visible) {
        entry->progressVisible = visible;
        emit progressVisibleChanged(storageId, visible);
    }
}
