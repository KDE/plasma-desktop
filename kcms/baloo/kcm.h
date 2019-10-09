/* This file is part of the KDE Project
   Copyright (c) 2007 Sebastian Trueg <trueg@kde.org>
   Copyright (c) 2012-2014 Vishesh Handa <me@vhanda.in>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef _BALOO_FILE_KCM_H_
#define _BALOO_FILE_KCM_H_

#include <KQuickAddons/ConfigModule>

#include "filteredfoldermodel.h"

namespace Baloo
{

class ServerConfigModule : public KQuickAddons::ConfigModule
{
    Q_OBJECT
    Q_PROPERTY(FilteredFolderModel *filteredModel READ filteredModel CONSTANT)
    Q_PROPERTY(bool indexing READ indexing WRITE setIndexing NOTIFY indexingChanged)
    Q_PROPERTY(bool fileContents READ fileContents WRITE setFileContents NOTIFY fileContentsChanged)

public:
    ServerConfigModule(QObject* parent, const QVariantList& args);
    virtual ~ServerConfigModule() override;

    bool indexing() const;
    void setIndexing(bool indexing);
    Q_SIGNAL void indexingChanged(bool indexing);

    bool fileContents() const;
    void setFileContents(bool fileContents);
    Q_SIGNAL void fileContentsChanged(bool fileContents);

    FilteredFolderModel *filteredModel() const;

public Q_SLOTS:
    void load() override;
    void save() override;
    void defaults() override;

private:
    /**
     * @brief Check if all mount points are in the excluded from indexing list.
     *
     * @return True if all mount points are excluded. False otherwise.
     */
    bool allMountPointsExcluded();
    FilteredFolderModel *m_filteredFolderModel;
    bool m_previouslyEnabled;
    bool m_indexing;
    bool m_fileContents;
};
}

#endif
