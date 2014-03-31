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

#ifndef KICKOFFMODEL_H
#define KICKOFFMODEL_H

// Qt
#include <QStandardItemModel>

// KDE

namespace Kickoff
{

/**
 * Base model for Kickoff models based on QStandardItemModel, enables drag and drop support
 */
class KickoffModel : public QStandardItemModel
{
    Q_OBJECT

public:
    /** Construct a new KickoffModel with the specified parent. */
    KickoffModel(QObject *parent = 0);
    virtual ~KickoffModel();

    //Reimplementations
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual QMimeData *mimeData(const QModelIndexList &indexes) const;
    virtual QStringList mimeTypes() const;
    virtual Qt::DropActions supportedDropActions() const;
    virtual Qt::DropActions supportedDragActions() const;

private:
};

}

#endif // KICKOFFMODEL_H
