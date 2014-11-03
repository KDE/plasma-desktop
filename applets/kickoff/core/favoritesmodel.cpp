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

//Own
#include "favoritesmodel.h"
#include "krunnermodel.h"

// Qt
#include <QHash>
#include <QList>
#include <QMimeData>
#include <QFileInfo>
#include <QDebug>

// KDE
#include <KConfigGroup>
#include <KMimeTypeTrader>
#include <KService>
#include <KDesktopFile>
#include <KLocalizedString>
#include <KUrl>



using namespace Kickoff;

class FavoritesModel::Private
{
public:
    Private(FavoritesModel *parent)
            : q(parent),
              displayOrder(NameAfterDescription)
    {
        init();
    }

    void init()
    {
    }

    void addFavoriteItem(const QString& url)
    {
        QStandardItem *item = StandardItemFactory::createItemForUrl(url, displayOrder);
        item->setData(i18n("Favorites"), Kickoff::GroupNameRole);
        q->appendRow(item);
    }

    void moveFavoriteItem(int startRow, int destRow)
    {
        if (destRow == startRow) {
            return;
        }

        QStandardItem *item = q->takeItem(startRow);
        q->removeRow(startRow);
        q->insertRow(destRow, item);
    }

    void removeFavoriteItem(const QString& url)
    {
        Q_UNUSED(url)
        QModelIndexList matches = q->match(q->index(0, 0), UrlRole,
                                           url, -1,
                                           Qt::MatchFlags(Qt::MatchStartsWith | Qt::MatchWrap | Qt::MatchRecursive));

        qDebug() << "Removing item matches" << matches;

        foreach (const QModelIndex& index, matches) {
            QStandardItem *item = q->itemFromIndex(index);
            if (item->parent()) {
                item->parent()->removeRow(item->row());
            } else {
                qDeleteAll(q->takeRow(item->row()));
            }
        }
    }

    static void loadFavorites()
    {
        globalFavoriteList.clear();
        globalFavoriteSet.clear();

        KConfigGroup favoritesGroup = componentData().config()->group("Favorites");
        QList<QString> favoriteList = favoritesGroup.readEntry("FavoriteURLs", QList<QString>());
        if (favoriteList.isEmpty()) {
            favoriteList = defaultFavorites();
        }

        foreach (const QString &favorite, favoriteList) {
            FavoritesModel::load(favorite);
        }
    }

    static QList<QString> defaultFavorites()
    {
        QList<QString> applications;

        KConfigGroup config(KSharedConfig::openConfig(), "General");
        QString browser = config.readPathEntry("BrowserApplication", QString());

        if (browser.isEmpty()) {
            const KService::Ptr htmlApp = KMimeTypeTrader::self()->preferredService(QLatin1String("text/html"));

            if (htmlApp) {
                browser = htmlApp->storageId().replace(QLatin1String(".desktop"), QString());
            }
        } else if (browser.startsWith('!')) {
            browser = browser.mid(1);
        }

        applications << browser << "kontact" << "systemsettings" << "dolphin" << "ktp-contactlist" << "kate";

        QList<QString> desktopFiles;

        foreach (const QString& application, applications) {
            KService::Ptr service = KService::serviceByStorageId(application + ".desktop");
            if (service) {
                desktopFiles << service->entryPath();
            }
        }

        return desktopFiles;
    }

    static void saveFavorites()
    {
        KConfigGroup favoritesGroup = componentData().config()->group("Favorites");
        favoritesGroup.writeEntry("FavoriteURLs", globalFavoriteList);
        favoritesGroup.config()->sync();
    }

    static QList<QString> globalFavoriteList;
    static QSet<QString> globalFavoriteSet;
    static QSet<FavoritesModel*> models;

    FavoritesModel * const q;
    DisplayOrder displayOrder;
};

QList<QString> FavoritesModel::Private::globalFavoriteList;
QSet<QString> FavoritesModel::Private::globalFavoriteSet;
QSet<FavoritesModel*> FavoritesModel::Private::models;

FavoritesModel::FavoritesModel(QObject *parent)
        : KickoffModel(parent)
        , d(new Private(this))
{
    Private::models << this;
    if (Private::models.count() == 1 && Private::globalFavoriteList.isEmpty()) {
        Private::loadFavorites();
    } else {
        foreach (const QString &url, Private::globalFavoriteList) {
            d->addFavoriteItem(url);
        }
    }

}

FavoritesModel::~FavoritesModel()
{
    Private::models.remove(this);

    if (Private::models.isEmpty()) {
        Private::saveFavorites();
    }

    delete d;
}

void FavoritesModel::add(const QString& url)
{
    FavoritesModel::load(url);

    // save after each add in case we crash
    Private::saveFavorites();
}

void FavoritesModel::load(const QString &url)
{
    QString handledUrl;
    KService::Ptr service = Kickoff::serviceForUrl(url);
    if (service) {
        handledUrl = service->entryPath();
    } else {
        handledUrl = url;
    }

    Private::globalFavoriteList << handledUrl;
    Private::globalFavoriteSet << handledUrl;

    foreach (FavoritesModel* model, Private::models) {
        model->d->addFavoriteItem(handledUrl);
    }
}

void FavoritesModel::move(int startRow, int destRow)
{
    // just move the item
//     Private::globalFavoriteList.move(startRow, destRow);

    foreach (FavoritesModel* model, Private::models) {
        qDebug() << "Move : " << startRow << destRow << model->rowCount();
        if (destRow >= model->rowCount()) {
            destRow = model->rowCount() - 1;
        }
        if (startRow >= model->rowCount()) {
            startRow = model->rowCount() - 1;
        }
        qDebug() << "Move : " << startRow << destRow << model->rowCount();
        Private::globalFavoriteList.move(startRow, destRow);
        model->d->moveFavoriteItem(startRow, destRow);
    }

    // save after each add in case we crash
    Private::saveFavorites();
}

void FavoritesModel::remove(const QString& url)
{
    Private::globalFavoriteList.removeAll(url);
    Private::globalFavoriteSet.remove(url);

    foreach (FavoritesModel* model, Private::models) {
        model->d->removeFavoriteItem(url);
    }

    // save after each remove in case of crash or other mishaps
    Private::saveFavorites();
}

bool FavoritesModel::isFavorite(const QString& url)
{
    QString handledUrl;
    KService::Ptr service = Kickoff::serviceForUrl(url);
    if (service) {
        handledUrl = service->entryPath();
    } else {
        handledUrl = url;
    }

    return Private::globalFavoriteSet.contains(handledUrl);
}

int FavoritesModel::numberOfFavorites()
{
    foreach (FavoritesModel* model, Private::models) {
        return model->d->q->rowCount() - 1;
    }

    return 0;
}

void FavoritesModel::sortFavorites(Qt::SortOrder order)
{
    if(Private::models.isEmpty()) {
       return;
    }

    foreach (FavoritesModel *model, Private::models) {
        model->d->q->sort(0, order);
    }

    Private::globalFavoriteList.clear();

    FavoritesModel *model = *Private::models.begin();
    QStandardItem *childData;
    for (int i = 0; i <= numberOfFavorites(); i++) {
        childData = model->d->q->item(i, 0);
        Private::globalFavoriteList.append(childData->data(Kickoff::UrlRole).toString());
    }
    Private::saveFavorites();
}

void FavoritesModel::sortFavoritesAscending()
{
    sortFavorites(Qt::AscendingOrder);
}

void FavoritesModel::sortFavoritesDescending()
{
    sortFavorites(Qt::DescendingOrder);
}

bool FavoritesModel::dropMimeData(const QString& text, const QVariantList& urls,
                                  int row, int column)
{
    qDebug() << "Dropping into row: " << row << urls;
    // FIXME: pass DropAction by qml
    Qt::DropAction action = Qt::MoveAction;
    if (action == Qt::IgnoreAction) {
        return true;
    }

    if (column > 0 || row > numberOfFavorites()) {
        qDebug() << "Exit " << numberOfFavorites();
        return false;
    }

    if (action == Qt::MoveAction) {
        QModelIndex modelIndex;
        QStandardItem *startItem;
        int startRow = -1;
        int destRow = row;
        qDebug() << "Moving ..." <<  startRow << destRow;

        // look for the favorite that was dragged
        bool conv = false;
        startRow = text.toInt(&conv, 10);

        if (!conv) {
            bool dropped = false;
            foreach (const QVariant &varUrl, urls) {
                if (!varUrl.value<QUrl>().isValid()) {
                    continue;
                }
                const QString path = varUrl.value<QUrl>().toLocalFile();
                if (!KDesktopFile::isDesktopFile(path)) {
                    continue;
                }
                KDesktopFile dFile(path);
                if (dFile.hasApplicationType() && !dFile.noDisplay()) {
                    FavoritesModel::add(path);
                    dropped = true;
                }
            }
            qDebug() << " now move the item to it's new location " << startRow << destRow;
            return dropped;
        }

        if (destRow < 0) {
            qDebug() << "Returning here" << startRow << destRow;
            return false;
        }

        // now move the item to it's new location
        FavoritesModel::move(startRow, destRow);

        return true;
    }

    return true;
}

QVariant FavoritesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal || section != 0) {
        return QVariant();
    }

    switch (role) {
    case Qt::DisplayRole:
        return i18nc("@title:column", "Favorites");
        break;
    default:
        return QVariant();
    }
}

void FavoritesModel::setNameDisplayOrder(DisplayOrder displayOrder)
{
    if (d->displayOrder == displayOrder) {
        return;
    }

    d->displayOrder = displayOrder;

    foreach (FavoritesModel* model, Private::models) {
        model->clear();
        model->d->init();
    }

    Private::loadFavorites();
}

DisplayOrder FavoritesModel::nameDisplayOrder() const
{
   return d->displayOrder;
}

#include "favoritesmodel.moc"
