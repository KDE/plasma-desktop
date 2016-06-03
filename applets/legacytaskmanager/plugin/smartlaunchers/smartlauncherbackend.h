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

#ifndef SMARTLAUNCHER_BACKEND_H
#define SMARTLAUNCHER_BACKEND_H

#include <QObject>
#include <QDBusContext>
#include <QHash>
#include <QVariantMap>

#include <Plasma/DataEngine>

class QDBusServiceWatcher;
class QString;

namespace Plasma {
class DataEngineConsumer;
}

namespace SmartLauncher {

struct Entry
{
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
    virtual ~Backend();

    bool available() const;
    bool hasLauncher(const QString &storageId) const;

    int count(const QString &uri) const;
    bool countVisible(const QString &uri) const;
    int progress(const QString &uri) const;
    bool progressVisible(const QString &uri) const;
    bool urgent(const QString &uri) const;

    QHash<QString, QString> unityMappingRules() const;

public slots:
    void dataUpdated(const QString &sourceName, const Plasma::DataEngine::Data &data);

signals:
    void countChanged(const QString &uri, int count);
    void countVisibleChanged(const QString &uri, bool countVisible);
    void progressChanged(const QString &uri, int progress);
    void progressVisibleChanged(const QString &uri, bool progressVisible);
    void urgentChanged(const QString &uri, bool urgent);

    void launcherRemoved(const QString &uri);

private slots:
    void update(const QString &uri, const QMap<QString, QVariant> &properties);

private:
    bool setupUnity();
    bool setupApplicationJobs();

    void onServiceUnregistered(const QString &service);

    template <typename T>
    void updateLauncherProperty(const QString &storageId, // our KService storage id
                                const QVariantMap &properties, // the map of properties we're given by DBus
                                const QString &property, // the property we're looking for
                                T *entryMember, // the member variable we're going to write our result in
                                // the change signal that will be emitted if the property has changed
                                void (Backend::*changeSignal)(const QString &, T))
    {
        auto foundProperty = properties.constFind(property);
        if (foundProperty != properties.constEnd()) {
            T newValue = foundProperty->value<T>();

            if (newValue != *entryMember) {
                *entryMember = newValue;
                emit ((this)->*changeSignal)(storageId, newValue);
            }
        }
    }

    void onApplicationJobAdded(const QString &source);
    void onApplicationJobRemoved(const QString &source);
    void updateApplicationJobPercent(const QString &storageId, Entry *entry);

    // Unity Launchers
    QDBusServiceWatcher *m_watcher;
    QHash<QString, QString> m_dbusServiceToLauncherUrl;
    QHash<QString, QString> m_launcherUrlToStorageId;
    // these rules can be configured in the legacytaskmanagerrulesrc in the "Unity Launcher Mapping" section
    // key is the actual desktop file name of the application (some-broken-app-beta.desktop)
    // vaue is how it actually announces itself on the Unity API (some-broken-app.desktop)
    QHash<QString, QString> m_unityMappingRules;

    // Application Jobs
    Plasma::DataEngineConsumer *m_dataEngineConsumer;
    Plasma::DataEngine *m_dataEngine;
    QHash<QString, QString> m_dataSourceToStorageId; // <Job 1, foo.desktop>
    QHash<QString, QStringList> m_storageIdToJobs; // <foo.desktop, <Job 1, Job 2, ..>>
    QHash<QString, int> m_jobProgress; // <Job 1, 42>

    QHash<QString, Entry> m_launchers;

    bool m_available = false;

};

} // namespace SmartLauncher

#endif // SMARTLAUNCHER_BACKEND_H
