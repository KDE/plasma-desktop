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
#include "kickoffmodel.h"

// Qt
#include <QMimeData>

// KDE
#include <KUrl>
#include <QDebug>

// Local
#include "models.h"

using namespace Kickoff;

KickoffModel::KickoffModel(QObject *parent)
        : QStandardItemModel(parent)
{
    QHash<int, QByteArray> roles;
    roles[Qt::DisplayRole] = "display";
    roles[Qt::DecorationRole] = "decoration";
    roles[Kickoff::SubTitleRole] = "subtitle";
    roles[Kickoff::UrlRole] = "url";
    roles[GroupNameRole] = "group";
    roles[Kickoff::MimeDataRole] = "mimedata";
    setRoleNames(roles);
}

KickoffModel::~KickoffModel()
{}

Qt::ItemFlags KickoffModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags defaultFlags = QStandardItemModel::flags(index);

    if (index.isValid()) {
        return Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | defaultFlags;
    } else {
        return 0;
    }
}

QMimeData *KickoffModel::mimeData(const QModelIndexList &indexes) const
{
    KUrl::List urls;

    foreach (const QModelIndex &index, indexes) {
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

QStringList KickoffModel::mimeTypes() const
{
    QStringList types;
    types << QLatin1String("text/uri-list");
    return types;
}

Qt::DropActions KickoffModel::supportedDropActions() const
{
    return Qt::MoveAction;
}

Qt::DropActions KickoffModel::supportedDragActions() const
{
    return Qt::CopyAction;
}

#include "kickoffmodel.moc"

