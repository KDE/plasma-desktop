/*
    Copyright 2007 Robert Knight <robertknight@gmail.com>
    Copyright 2011 Martin Gräßlin <mgraesslin@kde.org>

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
#include "recentlyusedmodel.h"

// Qt

// KDE
#include <KDesktopFile>
#include <KDirWatch>
#include <KRecentDocument>
#include <KLocalizedString>
#include <KUrl>
#include <QDebug>

// Local
#include "recentapplications.h"
#include "recentadaptor.h"

using namespace Kickoff;

class RecentlyUsedModel::Private
{
public:
    Private(RecentlyUsedModel *parent, RecentType recenttype, int maxRecentApps)
            : q(parent),
              recenttype(recenttype),
              maxRecentApps(maxRecentApps >= 0 ? maxRecentApps : Kickoff::RecentApplications::self()->defaultMaximum()),
              displayOrder(NameAfterDescription),
              applicationTitle(i18n("Applications")),
              documentTitle(i18n("Documents")),
              recentApplicationCount(0)
    {
    }

    bool removeExistingItem(const QString& path) {
        if (!itemsByPath.contains(path)) {
            return false;
        }

        QStandardItem *existingItem = itemsByPath[path];
        qDebug() << "Removing existing item" << existingItem;
        QModelIndex index = q->indexFromItem(existingItem);
        if (!index.isValid()) {
            qDebug() << "Now Index for our existing item";
            return false;
        }
        q->takeRow(index.row());
        itemsByPath.remove(path);
        delete existingItem;
        return true;
    }

    void addRecentApplication(KService::Ptr service, bool append) {
        // remove existing item if any
        if (removeExistingItem(service->entryPath())) {
            --recentApplicationCount;
        }

        QStandardItem *appItem = StandardItemFactory::createItemForService(service, displayOrder);
        appItem->setData(applicationTitle, Kickoff::GroupNameRole);
        itemsByPath.insert(service->entryPath(), appItem);

        if (append) {
            q->insertRow(recentApplicationCount, appItem);
        } else {
            q->insertRow(0, appItem);
        }
        ++recentApplicationCount;

        while (recentApplicationCount > maxRecentApps) {
            --recentApplicationCount;
            QList<QStandardItem*> row = q->takeRow(recentApplicationCount);

            //don't leave pending stuff in itemsByPath
            if (!row.isEmpty()) {
                itemsByPath.remove(row.first()->data(UrlRole).toString());
                qDeleteAll(row.begin(), row.end());
            }
        }
    }

    void addRecentDocument(const QString& desktopPath, bool append) {
        // remove existing item if any
        KDesktopFile desktopFile(desktopPath);
        KUrl documentUrl = desktopFile.readUrl();

        removeExistingItem(documentUrl.url());

        QStandardItem *documentItem = StandardItemFactory::createItemForUrl(desktopPath, displayOrder);
        documentItem->setData(true, Kickoff::SubTitleMandatoryRole);
        documentItem->setData(documentTitle, Kickoff::GroupNameRole);
        itemsByPath.insert(desktopPath, documentItem);

        //qDebug() << "Document item" << documentItem << "text" << documentItem->text() << "url" << documentUrl.url();
        if (append) {
            q->appendRow(documentItem);
        } else {
            q->insertRow(0, documentItem);
        }
    }

    void loadRecentDocuments()
    {
        // create branch for documents and add existing items
        const QStringList documents = KRecentDocument::recentDocuments();
        foreach(const QString& document, documents) {
            addRecentDocument(document, true);
        }
    }

    void loadRecentApplications()
    {
        const QList<KService::Ptr> services = RecentApplications::self()->recentApplications();
        for(int i = 0; i < maxRecentApps && i < services.count(); ++i) {
            addRecentApplication(services[i], true);
        }
    }

    RecentlyUsedModel * const q;
    RecentType recenttype;
    int maxRecentApps;

    QHash<QString, QStandardItem*> itemsByPath;
    DisplayOrder displayOrder;
    QString applicationTitle;
    QString documentTitle;
    int recentApplicationCount;
};

RecentlyUsedModel::RecentlyUsedModel(QObject *parent, RecentType recenttype, int maxRecentApps)
        : KickoffModel(parent),
          d(new Private(this, recenttype, maxRecentApps))
{
    QDBusConnection dbus = QDBusConnection::sessionBus();
    (void)new RecentAdaptor(this);
    QDBusConnection::sessionBus().registerObject("/kickoff/RecentAppDoc", this);
    dbus.connect(QString(), "/kickoff/RecentAppDoc", "org.kde.plasma", "clearRecentDocumentsAndApplications", this, SLOT(clearRecentDocumentsAndApplications()));

    if (recenttype != DocumentsOnly) {
        d->loadRecentApplications();

        // listen for changes to the list of recent applications
        connect(RecentApplications::self(), SIGNAL(applicationAdded(KService::Ptr,int)),
                this, SLOT(recentApplicationAdded(KService::Ptr,int)));
        connect(RecentApplications::self(), SIGNAL(applicationRemoved(KService::Ptr)),
                this, SLOT(recentApplicationRemoved(KService::Ptr)));
        connect(RecentApplications::self(), SIGNAL(cleared()),
                this, SLOT(recentApplicationsCleared()));
    }

    if (recenttype != ApplicationsOnly) {
        d->loadRecentDocuments();

        // listen for changes to the list of recent documents
        KDirWatch *recentDocWatch = new KDirWatch(this);
        recentDocWatch->addDir(KRecentDocument::recentDocumentDirectory(), KDirWatch::WatchFiles);
        connect(recentDocWatch, SIGNAL(created(QString)), this, SLOT(recentDocumentAdded(QString)));
        connect(recentDocWatch, SIGNAL(deleted(QString)), this, SLOT(recentDocumentRemoved(QString)));
    }
}

RecentlyUsedModel::~RecentlyUsedModel()
{
    delete d;
}

QVariant RecentlyUsedModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal || section != 0) {
        return QVariant();
    }

    switch (role) {
    case Qt::DisplayRole:
        if (d->recenttype == DocumentsAndApplications) {
            return i18n("Recently Used");
        } else if (d->recenttype == DocumentsOnly) {
            return i18n("Recently Used Documents");
        } else if (d->recenttype == ApplicationsOnly) {
            return i18n("Recently Used Applications");
        }
    default:
        return QVariant();
    }
}

void RecentlyUsedModel::setNameDisplayOrder(DisplayOrder displayOrder) 
{
    if (d->displayOrder == displayOrder) {
        return;
    }

    d->displayOrder = displayOrder;

    d->itemsByPath.clear();
    clear();

    if (d->recenttype != DocumentsOnly) {
        d->loadRecentApplications();
    }

    if (d->recenttype != ApplicationsOnly) {
        d->loadRecentDocuments();
    }
}

DisplayOrder RecentlyUsedModel::nameDisplayOrder() const
{
   return d->displayOrder;
}

void RecentlyUsedModel::recentDocumentAdded(const QString& path)
{
    qDebug() << "Recent document added" << path;
    d->addRecentDocument(path, false);
}
void RecentlyUsedModel::recentDocumentRemoved(const QString& path)
{
    qDebug() << "Recent document removed" << path;
    d->removeExistingItem(path);
}

void RecentlyUsedModel::recentApplicationAdded(KService::Ptr service, int)
{
    if (service) {
        d->addRecentApplication(service, false);
    }
}

void RecentlyUsedModel::recentApplicationRemoved(KService::Ptr service)
{
    if (service) {
        d->removeExistingItem(service->entryPath());
    }
}

void RecentlyUsedModel::recentApplicationsCleared()
{
    while (d->recentApplicationCount != 0) {
        QList<QStandardItem *>items = takeRow(0);
        Q_ASSERT(items.count() == 1); //model is only 1 level deep, so this will only ever remove one
        d->itemsByPath.remove(items.first()->data(UrlRole).toString());
        delete items[0];

        --d->recentApplicationCount;
    }
}

void RecentlyUsedModel::clearRecentApplications()
{
    RecentApplications::self()->clear();
}
void RecentlyUsedModel::clearRecentDocuments()
{
    KRecentDocument::clear();
}

void RecentlyUsedModel::clearRecentDocumentsAndApplications()
{
    clearRecentDocuments();
    clearRecentApplications();
}


#include "recentlyusedmodel.moc"

