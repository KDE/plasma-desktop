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

#ifndef RECENTLYUSEDMODEL_H
#define RECENTLYUSEDMODEL_H

#include "kickoffmodel.h"
#include "models.h"

// Qt

// KDE
#include <KService>

namespace Kickoff
{

/**
 * Model for the Recently Used view which provides a tree of recently used
 * applications and documents.
 */
class RecentlyUsedModel : public KickoffModel
{
    Q_OBJECT

public:

    /** Defines what the model should display. */
    enum RecentType {
        DocumentsAndApplications, ///< display recently used documents and applications.
        DocumentsOnly, ///< documents only. recently used applications will not be displayed.
        ApplicationsOnly ///< applications only, recently used documents will not be displayed.
    };

    /** Construct a new RecentlyUsedModel with the specified parent. */
    explicit RecentlyUsedModel(QObject *parent = 0, RecentType recenttype = DocumentsAndApplications, int maxRecentApps = -1);
    virtual ~RecentlyUsedModel();

    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    void setNameDisplayOrder(DisplayOrder displayOrder);
    DisplayOrder nameDisplayOrder() const;

public Q_SLOTS:
    void clearRecentApplications();
    void clearRecentDocuments();
    void clearRecentDocumentsAndApplications();

private Q_SLOTS:
    void recentDocumentAdded(const QString& path);
    void recentDocumentRemoved(const QString& path);
    void recentApplicationAdded(KService::Ptr, int startCount);
    void recentApplicationRemoved(KService::Ptr);
    void recentApplicationsCleared();

private:
    class Private;
    Private * const d;
};

}

#endif // RECENTLYUSEDMODEL_H
