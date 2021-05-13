/***************************************************************************
 *   Copyright (C) 2016, 2019 Kai Uwe Broulik <kde@privat.broulik.de>      *
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

#ifndef SMARTLAUNCHER_BACKEND_H
#define SMARTLAUNCHER_BACKEND_H

#include <QDBusContext>
#include <QHash>
#include <QObject>
#include <QVariantMap>

#include <notificationmanager/jobsmodel.h>

class QDBusServiceWatcher;
class QString;

namespace NotificationManager
{
class Settings;
}

namespace SmartLauncher
{
struct Entry {
    int count = 0;
    bool countVisible = false;
    int progress = 0;
    bool progressVisible = false;
    bool urgent = false;
};

class Backend : public QObject, protected QDBusContext
{
    Q_OBJECT

public:
    explicit Backend(QObject *parent = nullptr);
    ~Backend() override;

    bool hasLauncher(const QString &storageId) const;

    int count(const QString &uri) const;
    bool countVisible(const QString &uri) const;
    int progress(const QString &uri) const;
    bool progressVisible(const QString &uri) const;
    bool urgent(const QString &uri) const;

    QHash<QString, QString> unityMappingRules() const;

Q_SIGNALS:
    void countChanged(const QString &uri, int count);
    void countVisibleChanged(const QString &uri, bool countVisible);
    void progressChanged(const QString &uri, int progress);
    void progressVisibleChanged(const QString &uri, bool progressVisible);
    void urgentChanged(const QString &uri, bool urgent);

    void reloadRequested(const QString &uri);
    void launcherRemoved(const QString &uri);

private Q_SLOTS:
    void update(const QString &uri, const QMap<QString, QVariant> &properties);

private:
    void reload();
    void setupUnity();
    void setupApplicationJobs();

    void onServiceUnregistered(const QString &service);

    template<typename T>
    void updateLauncherProperty(const QString &storageId, // our KService storage id
                                const QVariantMap &properties, // the map of properties we're given by DBus
                                const QString &property, // the property we're looking for
                                T *entryMember, // the member variable we're going to write our result in
                                // the getter for this property which might return something different from the raw value
                                T (Backend::*getter)(const QString &) const,
                                // the change signal that will be emitted if the property has changed
                                void (Backend::*changeSignal)(const QString &, T))
    {
        auto foundProperty = properties.constFind(property);
        if (foundProperty != properties.constEnd()) {
            const T oldSanitizedValue = ((this)->*getter)(storageId);

            T newValue = foundProperty->value<T>();
            *entryMember = newValue;

            const T newSanitizedValue = ((this)->*getter)(storageId);

            if (newSanitizedValue != oldSanitizedValue) {
                Q_EMIT((this)->*changeSignal)(storageId, newSanitizedValue);
            }
        }
    }

    bool doNotDisturbMode() const;

    // Unity Launchers
    QDBusServiceWatcher *m_watcher;
    QHash<QString, QString> m_dbusServiceToLauncherUrl;
    QHash<QString, QString> m_launcherUrlToStorageId;
    // these rules can be configured in the taskmanagerrulesrc in the "Unity Launcher Mapping" section
    // key is the actual desktop file name of the application (some-broken-app-beta.desktop)
    // vaue is how it actually announces itself on the Unity API (some-broken-app.desktop)
    QHash<QString, QString> m_unityMappingRules;

    // Application Jobs
    NotificationManager::JobsModel::Ptr m_jobsModel;

    NotificationManager::Settings *m_settings = nullptr;

    QHash<QString, Entry> m_launchers;

    QStringList m_badgeBlacklist;

    bool m_available = false;
};

} // namespace SmartLauncher

#endif // SMARTLAUNCHER_BACKEND_H
