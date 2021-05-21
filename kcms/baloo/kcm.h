/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2007 Sebastian Trueg <trueg@kde.org>
    SPDX-FileCopyrightText: 2012-2014 Vishesh Handa <me@vhanda.in>
    SPDX-FileCopyrightText: 2020 Benjamin Port <benjamin.port@enioka.com>

    SPDX-License-Identifier: LGPL-2.0-only
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
