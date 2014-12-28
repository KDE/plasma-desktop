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
#include "models.h"
#include "leavemodel.h"

// Qt
#include <QFileInfo>
#include <QStandardItem>
#include <QDir>

// KDE
#include <QDebug>
#include <KConfigGroup>
#include <KDesktopFile>
#include <QIcon>
#include <KMimeType>
#include <KLocalizedString>
#include <KGlobal>
#include <KUrl>
#include <Solid/Device>
#include <Solid/StorageAccess>
#include <Solid/StorageDrive>

using namespace Kickoff;

namespace Kickoff
{

Q_GLOBAL_STATIC_WITH_ARGS(KUrl, homeUrl, (QDir::homePath()))
Q_GLOBAL_STATIC_WITH_ARGS(KUrl, remoteUrl, ("remote:/"))
K_GLOBAL_STATIC(StandardItemFactoryData, factoryData)

StandardItemFactoryData* deviceFactoryData()
{
    return factoryData;
}
} // namespace Kickoff

QStandardItem *StandardItemFactory::createItemForUrl(const QString& urlString, DisplayOrder displayOrder)
{
    KUrl url(urlString);

    QStandardItem *item = 0;

    // Match files ending with ".desktop" and being local or having a relative
    // path. For instance applications that still installs .desktop files at
    // /usr/share/applnk, like KVirc 3
    if (urlString.endsWith(QLatin1String(".desktop")) && (url.isLocalFile() || url.isRelative())) {
        // .desktop files may be services (type field == 'Application' or 'Service')
        // or they may be other types such as links.
        //
        // first look in the KDE service database to see if this file is a service,
        // otherwise represent it as a generic .desktop file
        KService::Ptr service = KService::serviceByDesktopPath(url.toLocalFile());
        if (service) {
            return createItemForService(service, displayOrder);
        }

        item = new QStandardItem;
        KDesktopFile desktopFile(url.toLocalFile());
        item->setText(QFileInfo(urlString.mid(0, urlString.lastIndexOf('.'))).completeBaseName());
        item->setIcon(QIcon::fromTheme(desktopFile.readIcon()));

        //FIXME: desktopUrl is a hack around borkage in KRecentDocuments which
        //       stores a path in the URL field!
        KUrl desktopUrl(desktopFile.desktopGroup().readPathEntry("URL", QString()));
        if (!desktopUrl.url().isEmpty()) {
            item->setData(desktopUrl.url(), Kickoff::UrlRole);
        } else {
            // desktopUrl.url() is empty if the file doesn't exist so set the
            // url role to that which was passed so that the item can still be
            // manually removed
            item->setData(urlString, Kickoff::UrlRole);
        }

        QString subTitle = desktopUrl.isLocalFile() ? desktopUrl.toLocalFile() : desktopUrl.prettyUrl();
        item->setData(subTitle, Kickoff::SubTitleRole);

        setSpecialUrlProperties(desktopUrl, item);
    } else if (url.scheme() == "leave") {
        item = LeaveModel::createStandardItem(urlString);
    } else {
        item = new QStandardItem;
        const QString subTitle = url.isLocalFile() ? url.toLocalFile() : url.prettyUrl();
        QString basename = QFileInfo(url.prettyUrl()).completeBaseName();
        if (basename.isNull()) {
            basename = subTitle;
        }

        item->setText(basename);
        //FIXME
//         item->setIcon(QIcon::fromTheme(KMimeType::iconNameForUrl(url)));
        item->setData(url.url(), Kickoff::UrlRole);
        item->setData(subTitle, Kickoff::SubTitleRole);

        setSpecialUrlProperties(url, item);
    }

    return item;
}

void StandardItemFactory::setSpecialUrlProperties(const KUrl& url, QStandardItem *item)
{
    // specially handled URLs
    if (homeUrl() && url == *homeUrl()) {
        item->setText(i18n("Home Folder"));
        item->setIcon(QIcon::fromTheme("user-home"));
    } else if (remoteUrl() && url == *remoteUrl()) {
        item->setText(i18n("Network Folders"));
    }
}

QStandardItem *StandardItemFactory::createItem(const QIcon & icon, const QString & title,
        const QString & description, const QString & url, const QString & mimeData)
{
    QStandardItem *appItem = new QStandardItem;

    appItem->setText(title);
    appItem->setIcon(icon);
    appItem->setData(description, Kickoff::SubTitleRole);
    appItem->setData(url, Kickoff::UrlRole);
    appItem->setData(mimeData, Kickoff::MimeDataRole);

    return appItem;
}

QStandardItem *StandardItemFactory::createItemForService(KService::Ptr service, DisplayOrder displayOrder)
{
    QStandardItem *appItem = new QStandardItem;

    QString genericName = service->genericName();
    QString appName = service->name();
    bool nameFirst = displayOrder == NameBeforeDescription;
    appItem->setText(nameFirst || genericName.isEmpty() ? appName : genericName);
    appItem->setIcon(QIcon::fromTheme(service->icon()));
    appItem->setData(service->entryPath(), Kickoff::UrlRole);

    if (nameFirst) {
        if (!genericName.isEmpty()) {
            appItem->setData(genericName, Kickoff::SubTitleRole);
        }
     } else if (!genericName.isEmpty()) {
         // we only set the subtitle to appname if the generic name is empty because if
         // the generic name IS empty, then the app name is used as the title role
         // and we don't want it repeated twice.
         appItem->setData(appName, Kickoff::SubTitleRole);
    }

    return appItem;
}

bool Kickoff::isLaterVersion(KService::Ptr first , KService::Ptr second)
{
    // a very crude heuristic using the .desktop path names
    // which only understands kde3 vs kde4
    bool firstIsKde4 = first->entryPath().contains("kde4");
    bool secondIsKde4 = second->entryPath().contains("kde4");

    return firstIsKde4 && !secondIsKde4;
}

QStringList Kickoff::systemApplicationList()
{
    KConfigGroup appsGroup = componentData().config()->group("SystemApplications");
    QStringList apps;
    apps << "systemsettings";
    apps = appsGroup.readEntry("DesktopFiles", apps);
    return apps;
}

#if 0
void Kickoff::swapModelIndexes(QModelIndex& first, QModelIndex& second)
{
    Q_ASSERT(first.isValid());
    Q_ASSERT(second.isValid());

    QAbstractItemModel *firstModel = const_cast<QAbstractItemModel*>(first.model());
    QAbstractItemModel *secondModel = const_cast<QAbstractItemModel*>(second.model());

    Q_ASSERT(firstModel && secondModel);

    QMap<int, QVariant> firstIndexData = firstModel->itemData(first);
    QMap<int, QVariant> secondIndexData = secondModel->itemData(second);

    firstModel->setItemData(first, secondIndexData);
    secondModel->setItemData(second, firstIndexData);
}
#endif

K_GLOBAL_STATIC_WITH_ARGS(KComponentData, kickoffComponent, ("kickoff", QByteArray(), KComponentData::SkipMainComponentRegistration))
KComponentData Kickoff::componentData()
{
    return *kickoffComponent;
}



