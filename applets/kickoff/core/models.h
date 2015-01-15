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

#ifndef MODELS_H
#define MODELS_H

// Qt

// KDE
#include <KService>
#include <KComponentData>

class QStandardItem;
class KUrl;
class QModelIndex;

namespace Solid
{
class Device;
}

namespace Kickoff
{
class StandardItemFactoryData
{
public:
    QHash<QString, Solid::Device> deviceByUrl;
};

StandardItemFactoryData* deviceFactoryData();

/**
 * List of applications contained in the DesktopFiles key in the
 * SystemApplications group, used to populate the System model
 * @return list of desktop entry names
 */
QStringList systemApplicationList();

/**
 * Additional data roles for data which the Kickoff models supply with their items
 * for use when rendering the items and launching them.
 */
enum DataRole {
    /** A sub title to be displayed below the text from the item's Qt::DisplayRole data */
    SubTitleRole = Qt::UserRole + 1,
    FirstDataRole = SubTitleRole,
    /** The URL to be opened when executing the item. */
    UrlRole = Qt::UserRole + 2,
    /** The Solid device identifier for items which represent devices. */
    DeviceUdiRole = Qt::UserRole + 3,
    /** The amount of space (in Kilobytes) used for items which represent storage. */
    DiskUsedSpaceRole = Qt::UserRole + 4,
    /** The amount of free space (in Kilobytes) for items which represent storage. */
    DiskFreeSpaceRole = Qt::UserRole + 5,
    SubTitleMandatoryRole = Qt::UserRole + 6,
    /** Is item a separator. **/
    SeparatorRole = Qt::UserRole + 7,
    /** relative path of the item */
    RelPathRole = Qt::UserRole + 8,
    IconNameRole = Qt::UserRole + 9,
    GroupNameRole = Qt::UserRole + 10,
    MimeDataRole = Qt::UserRole + 11,
    LastDataRole = Qt::UserRole + 100
};

/**
 * This enum describes the policy for displaying
 * Name of Application - Description
 * Description - Name of Application
 */
enum DisplayOrder {
    NameAfterDescription,
    NameBeforeDescription
};

/**
 * Factory for creating QStandardItems with appropriate text, icons, URL
 * and other Kickoff-specific information for a given URL or Service.
 */
class StandardItemFactory
{
public:
    static QStandardItem *createItemForUrl(const QString& url, DisplayOrder displayOrder);
    static QStandardItem *createItemForService(KService::Ptr service,
                                               DisplayOrder displayOrder);
    static QStandardItem *createItem(const QIcon & icon, const QString & title,
        const QString & description, const QString & url, const QString & mimeData = QString());

private:
    static void setSpecialUrlProperties(const KUrl& url, QStandardItem *item);
};

/**
 * Abstract base class for delegates which provide information about a model
 * item's state in a particular view.
 */
class ItemStateProvider
{
public:
    virtual ~ItemStateProvider() {}

    /**
     * Returns true if a @p index should be drawn in the view or
     * false if it should be hidden.
     */
    virtual bool isVisible(const QModelIndex& index) const = 0;
};

// returns true if 'first' represents a more recent version of
// an application than 'second'
//
// eg. isLaterVersion(myapp_kde4,myapp_kde3) returns true
bool isLaterVersion(KService::Ptr first , KService::Ptr second);

#if 0
/** Swaps the data for two indexes in a QAbstractItemModel */
void swapModelIndexes(QModelIndex& first, QModelIndex& second);
#endif

// returns the Kickoff component data, this is mainly used
// to access the Kickoff shared config data
KComponentData componentData();
}

#endif //MODELS_H
