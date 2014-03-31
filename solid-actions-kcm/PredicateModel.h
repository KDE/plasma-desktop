/**************************************************************************
 * Copyright (C) 2009 Ben Cooksley <ben@eclipse.endoftheinternet.org>     *
 *                                                                        *
 * This program is free software; you can redistribute it and/or          *
 * modify it under the terms of the GNU General Public License            *
 * as published by the Free Software Foundation; either version 2         *
 * of the License, or (at your option) any later version.                 *
 *                                                                        *
 * This program is distributed in the hope that it will be useful,        *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 * GNU General Public License for more details.                           *
 *                                                                        *
 * You should have received a copy of the GNU General Public License      *
 * along with this program; if not, write to the Free Software            *
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA          *
 * 02110-1301, USA.                                                       *
***************************************************************************/

#ifndef PREDICATEMODEL_H
#define PREDICATEMODEL_H

#include <QtCore/QAbstractItemModel>

#include <Solid/Predicate>

class PredicateItem;

class PredicateModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit PredicateModel( PredicateItem * menuRoot, QObject *parent = 0 );
    ~PredicateModel();

    QVariant data( const QModelIndex &index, int role ) const;
    Qt::ItemFlags flags( const QModelIndex &index ) const;
    QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex() ) const;
    QModelIndex parent( const QModelIndex &index ) const;
    int rowCount( const QModelIndex &parent = QModelIndex() ) const;
    int columnCount( const QModelIndex &parent = QModelIndex() ) const;

    void setRootPredicate( PredicateItem * item );
    void itemUpdated( const QModelIndex& item );
    void childrenChanging( const QModelIndex& item, Solid::Predicate::Type oldType );

protected:
    PredicateItem * rootItem() const;

private:
    class Private;
    Private *const d;
};

#endif
