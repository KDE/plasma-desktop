/*
    Copyright 2008  Marco Martin  <notmart@gmail.com>

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
#include "kickoffabstractmodel.h"

// Qt
#include <QMimeData>

// KDE
#include <KUrl>
#include <QDebug>

// Local
#include "models.h"

using namespace Kickoff;

KickoffAbstractModel::KickoffAbstractModel(QObject *parent)
        : QAbstractItemModel(parent)
{}

KickoffAbstractModel::~KickoffAbstractModel()
{}

Qt::ItemFlags KickoffAbstractModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags defaultFlags = QAbstractItemModel::flags(index);

    if (index.isValid()) {
        return Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | defaultFlags;
    } else {
        return 0;
    }
}

QMimeData *KickoffAbstractModel::mimeData(const QModelIndexList &indexes) const
{
    KUrl::List urls;
    QByteArray itemData;

    foreach(const QModelIndex &index, indexes) {
        KUrl itemUrl = KUrl(data(index, UrlRole).toString());
        if (itemUrl.isValid()) {
            urls << itemUrl;
        }
    }

    QMimeData *mimeData = new QMimeData();

    if (!urls.isEmpty()) {
        urls.populateMimeData(mimeData);
    }

    return mimeData;
}

QStringList KickoffAbstractModel::mimeTypes() const
{
    QStringList types;
    types << QLatin1String("text/uri-list");
    return types;
}

Qt::DropActions KickoffAbstractModel::supportedDropActions() const
{
    return 0;
}

Qt::DropActions KickoffAbstractModel::supportedDragActions() const
{
    return Qt::CopyAction;
}

#include "kickoffabstractmodel.moc"

