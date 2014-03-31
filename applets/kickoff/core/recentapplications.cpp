/*
    Copyright 2007 Robert Knight <robertknight@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

// Own
#include "recentapplications.h"

// Qt
#include <QtCore/QHash>
#include <QtCore/QLinkedList>

// KDE
#include <KConfigGroup>
#include <KGlobal>

#include <QDebug>

// Local
#include "models.h"

using namespace Kickoff;

class RecentApplications::Private
{
public:
    class ServiceInfo;

    Private() : defaultMaxServices(DEFAULT_MAX_SERVICES) {
        KConfigGroup recentGroup = componentData().config()->group("RecentlyUsed");
        QList<QString> recentApplications = recentGroup.readEntry("Applications", QList<QString>());
        defaultMaxServices = maxServices = qMax(0, recentGroup.readEntry("MaxApplications", defaultMaxServices));

        // TESTING
        //      the actual last date/time is not currently recorded, instead we just use
        //      the current date/time and adjust it by one second after each item is added
        //      to preserve the order of the applications in the list loaded from the KConfig
        //      source
        QDateTime dateTime = QDateTime::currentDateTime();
        foreach(const QString& application, recentApplications) {
            ServiceInfo info;
            info.storageId = application;
            info.startCount = 1;
            info.lastStartedTime = dateTime;
            addEntry(info.storageId, info);
            dateTime = dateTime.addSecs(1);
        }
    };
    ~Private() {
        KConfigGroup recentGroup = componentData().config()->group("RecentlyUsed");

        QList<ServiceInfo> services = serviceInfo.values();
        qSort(services.begin(), services.end());

        // TESTING
        //      only the desktop file used is currently recorded, information such
        //      as start count and date/time of last used is lost
        QList<QString> recentApplications;
        foreach(const ServiceInfo& info, services) {
            recentApplications << info.storageId;
        }

        recentGroup.writeEntry("Applications", recentApplications);
        recentGroup.config()->sync();
    }
    void addEntry(const QString& id, ServiceInfo& info) {
        // if this service is already in the list then remove the existing
        // queue entry (so that there are no duplicates in the queue)
        if (serviceInfo.contains(id)) {
            qDebug() << "Duplicate entry added.  Removing existing entry from queue.";
            serviceQueue.erase(serviceInfo[id].queueIter);
        }

        serviceQueue.append(id);
        info.queueIter = --serviceQueue.end();
        serviceInfo.insert(id, info);
    }

    void removeExpiredEntries() {
        // if more than the maximum number of services have been added
        // remove the least recently used service
        while (serviceQueue.count() > maxServices) {
            QString removeId = serviceQueue.takeFirst();
            qDebug() << "More than the maximal " << maxServices << " services added.  Removing" << removeId << "from queue.";
            serviceInfo.remove(removeId);
            emit instance.applicationRemoved(KService::serviceByStorageId(removeId));
        }
    }

    class ServiceInfo
    {
    public:
        ServiceInfo() : startCount(0) {}

        QString storageId;
        int startCount;
        QDateTime lastStartedTime;
        QLinkedList<QString>::iterator queueIter;

        bool operator<(const ServiceInfo& rhs) const {
            return this->lastStartedTime < rhs.lastStartedTime;
        }
    };

    static const int DEFAULT_MAX_SERVICES = 5;
    int defaultMaxServices, maxServices;
    // queue to keep track of the order in which services have been used
    // (most recently used at the back)
    QLinkedList<QString> serviceQueue;
    QHash<QString, ServiceInfo> serviceInfo;
    RecentApplications instance;
};
K_GLOBAL_STATIC(RecentApplications::Private, privateSelf)

RecentApplications *RecentApplications::self()
{
    return &privateSelf->instance;
}

RecentApplications::RecentApplications()
{
}

QList<KService::Ptr> RecentApplications::recentApplications() const
{
    QList<Private::ServiceInfo> services = privateSelf->serviceInfo.values();
    qSort(services.begin(), services.end(), qGreater<Private::ServiceInfo>());

    QList<KService::Ptr> servicePtrs;
    foreach(const Private::ServiceInfo& info, services) {
        KService::Ptr s = KService::serviceByStorageId(info.storageId);

        if (s) {
            servicePtrs << s;
        }
    }
    return servicePtrs;
}
int RecentApplications::startCount(KService::Ptr service) const
{
    return privateSelf->serviceInfo[service->storageId()].startCount;
}
QDateTime RecentApplications::lastStartedTime(KService::Ptr service) const
{
    return privateSelf->serviceInfo[service->storageId()].lastStartedTime;
}
void RecentApplications::setMaximum(int maximum)
{
    Q_ASSERT(maximum >= 0);
    privateSelf->maxServices = maximum;
    privateSelf->removeExpiredEntries();
}
int RecentApplications::maximum() const
{
    return privateSelf->maxServices;
}
int RecentApplications::defaultMaximum() const
{
    return privateSelf->defaultMaxServices;
}
void RecentApplications::add(KService::Ptr service)
{
    Private::ServiceInfo info = privateSelf->serviceInfo.value(service->storageId());
    info.storageId = service->storageId();
    info.startCount++;
    info.lastStartedTime = QDateTime::currentDateTime();

    privateSelf->addEntry(info.storageId, info);

    qDebug() << "Recent app added" << info.storageId << info.startCount;
    emit applicationAdded(service, info.startCount);

    privateSelf->removeExpiredEntries();
}
void RecentApplications::clear()
{
    privateSelf->serviceInfo.clear();
    emit cleared();
}

#include "recentapplications.moc"
