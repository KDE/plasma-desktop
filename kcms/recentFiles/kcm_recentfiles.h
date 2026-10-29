/*
    SPDX-FileCopyrightText: 2012-2016 Ivan Cukic <ivan.cukic@kde.org>
    SPDX-FileCopyrightText: 2022 Méven Car <meven.car@kdenet.net>
    SPDX-FileCopyrightText: 2026 James Graham <james.h.graham@protonmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#pragma once

#include <KQuickConfigModule>

#include "excludedapplicationsmodel.h"
#include "kactivitymanagerd_plugins_settings.h"

/**
 * RecentFilesKcm
 */
class RecentFilesKcm : public KQuickConfigModule
{
    Q_OBJECT

    Q_PROPERTY(int keepHistoryFor READ keepHistoryFor WRITE setKeepHistoryFor NOTIFY keepHistoryForChanged)

    Q_PROPERTY(int whatToRemember READ whatToRemember WRITE setWhatToRemember NOTIFY whatToRememberChanged)

    Q_PROPERTY(bool blockByDefault READ blockByDefault WRITE setBlockByDefault NOTIFY blockByDefaultChanged)

    Q_PROPERTY(ExcludedApplicationsModel *excludedApplicationsModel READ excludedApplicationsModel CONSTANT)

public:
    enum WhatToRemember {
        AllApplications = 0,
        SpecificApplications = 1,
        NoApplications = 2,
    };

    explicit RecentFilesKcm(QObject *parent, const KPluginMetaData &data);

    int whatToRemember() const;
    void setWhatToRemember(int whatToRemember);
    int keepHistoryFor() const;
    void setKeepHistoryFor(int keepHistoryFor);
    bool blockByDefault() const;
    void setBlockByDefault(bool blockByDefault);

    ExcludedApplicationsModel *excludedApplicationsModel() const;

    void defaults() override;
    void load() override;
    void save() override;

    Q_INVOKABLE void forget(int count, const QString &what);

Q_SIGNALS:
    void whatToRememberChanged();
    void keepHistoryForChanged();
    void blockByDefaultChanged();

private:
    KActivityManagerdPluginsSettings *m_settings;
    ExcludedApplicationsModel *m_excludedApplicationsModel;
};
