/*
    Copyright 2009 Ivan Cukic <ivan.cukic+kde@gmail.com>

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

#ifndef KRUNNERMODEL_H
#define KRUNNERMODEL_H

#include "models.h"
#include "kickoffmodel.h"
#include "urlitemlauncher.h"

namespace Plasma {
    class QueryMatch;
}

namespace Kickoff {

    KService::Ptr serviceForUrl(const KUrl & url);

class KRunnerItemHandler : public UrlItemHandler {
public:
    virtual bool openUrl(const KUrl& url);
};

class KRunnerModel : public KickoffModel
{
    Q_OBJECT

public:
    KRunnerModel(QObject *parent = NULL);
    virtual ~KRunnerModel();
    void setNameDisplayOrder(DisplayOrder displayOrder);
    DisplayOrder nameDisplayOrder() const;

    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual QMimeData *mimeData(const QModelIndexList &indexes) const;
    // virtual QStringList mimeTypes() const;
    // virtual Qt::DropActions supportedDropActions() const;
    // virtual Qt::DropActions supportedDragActions() const;

private:
    void timerEvent(QTimerEvent * event);

public Q_SLOTS:
    void setQuery(const QString& query);

private Q_SLOTS:
    void matchesChanged(const QList< Plasma::QueryMatch > & matches);

Q_SIGNALS:
    void resultsAvailable();

private:
    class Private;
    Private * const d;
};

}

#endif // KRUNNERMODEL_H

