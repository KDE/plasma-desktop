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

#ifndef FAVORITESMODEL_H
#define FAVORITESMODEL_H

#include "kickoffmodel.h"
#include "models.h"

namespace Kickoff
{

/**
 * A model which provides an ordered list of 'favorite' items chosen by the user.
 * The items may represent documents, folders, applications, devices or anything else
 * identified by a URL.
 *
 * The information persists between sessions.
 */
class FavoritesModel : public KickoffModel
{
    Q_OBJECT

public:
    FavoritesModel(QObject *parent = NULL);
    virtual ~FavoritesModel();

    /** Add a new item for @p url to the user's favorites list and sync list. */
    Q_INVOKABLE static void add(const QString& url);

    /** Add a new item for @p url to the user's favorites list. */
    static void load(const QString& url);

    /** Remove the item associated with @p url from the user's favorites list. */
    Q_INVOKABLE static void remove(const QString& url);
    /** Returns true if @p url is in the list of the user's favorite URLs. */
    Q_INVOKABLE static void move(int startRow, int destRow);
    static int numberOfFavorites();
    static void sortFavorites(Qt::SortOrder order);
    Q_INVOKABLE static bool isFavorite(const QString& url);

    using QAbstractItemModel::dropMimeData;
    Q_INVOKABLE virtual bool dropMimeData(const QString& text, const QVariantList& urls,
                                          int row, int column);
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    void setNameDisplayOrder(DisplayOrder displayOrder);
    DisplayOrder nameDisplayOrder() const;

public Q_SLOTS:
    void sortFavoritesAscending();
    void sortFavoritesDescending();

private:
    class Private;
    Private * const d;
};

}

#endif // FAVORITESMODEL_H
