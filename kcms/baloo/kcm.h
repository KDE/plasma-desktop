/* This file is part of the KDE Project
   Copyright (c) 2007 Sebastian Trueg <trueg@kde.org>
   Copyright (c) 2012-2014 Vishesh Handa <me@vhanda.in>
   Copyright (c) 2020 Benjamin Port <benjamin.port@enioka.com>

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

#include <KQuickAddons/ManagedConfigModule>

#include "filteredfoldermodel.h"

class BalooSettings;
class BalooData;

namespace Baloo
{
class ServerConfigModule : public KQuickAddons::ManagedConfigModule
{
    Q_OBJECT
    Q_PROPERTY(FilteredFolderModel *filteredModel READ filteredModel CONSTANT)
    Q_PROPERTY(BalooSettings *balooSettings READ balooSettings CONSTANT)

public:
    ServerConfigModule(QObject *parent, const QVariantList &args);
    virtual ~ServerConfigModule() override;

    BalooSettings *balooSettings() const;
    FilteredFolderModel *filteredModel() const;

    Q_INVOKABLE void deleteIndex();
    Q_INVOKABLE int rawIndexFileSize();
    Q_INVOKABLE QString prettyIndexFileSize();

public Q_SLOTS:
    void load() override;
    void save() override;

private:
    BalooData *m_data;
    FilteredFolderModel *m_filteredFolderModel;
};
}

#endif
