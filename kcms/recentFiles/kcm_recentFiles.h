/*
    SPDX-FileCopyrightText: 2012-2016 Ivan Cukic <ivan.cukic@kde.org>
    SPDX-FileCopyrightText: 2022 Méven Car <meven.car@kdenet.net>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef RECENT_FILES_H
#define RECENT_FILES_H

#include <KCModule>
#include <KPluginFactory>

#include <utils/d_ptr.h>

class KCoreConfigSkeleton;

/**
 * RecentFilesKcm
 */
class RecentFilesKcm : public KCModule
{
    Q_OBJECT
public:
    explicit RecentFilesKcm(QWidget *parent, QVariantList args);
    ~RecentFilesKcm() override;

    KCoreConfigSkeleton *pluginConfig();

public Q_SLOTS:
    void defaults() override;
    void load() override;
    void save() override;

private Q_SLOTS:
    void forget(int count, const QString &what);
    void forgetLastHour();
    void forgetTwoHours();
    void forgetDay();
    void forgetAll();

    void spinKeepHistoryValueChanged(int value);

Q_SIGNALS:
    void blackListModelChanged(bool changed);
    void blackListModelDefaulted(bool isDefault);

private:
    enum WhatToRemember {
        AllApplications = 0,
        SpecificApplications = 1,
        NoApplications = 2,
    };

    D_PTR;
};

#endif // RECENT_FILES_H
