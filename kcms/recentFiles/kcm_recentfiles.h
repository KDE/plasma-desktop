/*
    SPDX-FileCopyrightText: 2012-2016 Ivan Cukic <ivan.cukic@kde.org>
    SPDX-FileCopyrightText: 2022 Méven Car <meven.car@kdenet.net>
    SPDX-FileCopyrightText: 2026 James Graham <james.h.graham@protonmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#pragma once

#include <KQuickManagedConfigModule>

#include "kactivitymanagerd_plugins_settings.h"

class KActivitiesData;

/**
 * RecentFilesKcm
 */
class RecentFilesKcm : public KQuickManagedConfigModule
{
    Q_OBJECT

    Q_PROPERTY(KActivityManagerdPluginsSettings *settings READ settings CONSTANT)

public:
    enum WhatToRemember {
        AllApplications = 0,
        SpecificApplications = 1,
        NoApplications = 2,
    };

    explicit RecentFilesKcm(QObject *parent, const KPluginMetaData &data);

    KActivityManagerdPluginsSettings *settings() const;

    Q_INVOKABLE void forget(int count, const QString &what);

private:
    KActivitiesData *m_data;
};
