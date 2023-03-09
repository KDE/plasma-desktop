/*
    SPDX-FileCopyrightText: 2012-2016 Ivan Cukic <ivan.cukic@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#pragma once

#include <KActivities/Consumer>
#include <KActivities/Info>
#include <KCModule>
#include <KPluginFactory>

#include <utils/d_ptr.h>

/**
 * MainConfigurationWidget
 */
class MainConfigurationWidget : public KCModule
{
    Q_OBJECT
public:
    MainConfigurationWidget(QObject *parent, const KPluginMetaData &data, const QVariantList &args);
    ~MainConfigurationWidget() override;

public Q_SLOTS:
    void defaults() override;
    void load() override;
    void save() override;

private:
    D_PTR;
};
