/*
    SPDX-FileCopyrightText: 2021 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <KQuickAddons/ManagedConfigModule>

#include <KSharedConfig>

#include "devicesmodel.h"

class TabletSettings;
class TabletData;

class Tablet : public KQuickAddons::ManagedConfigModule
{
    Q_OBJECT
    Q_PROPERTY(DevicesModel *devicesModel READ devicesModel CONSTANT)

public:
    explicit Tablet(QObject *parent, const KPluginMetaData &metaData, const QVariantList &list);
    ~Tablet() override;

    void load() override;
    void save() override;
    void defaults() override;
    bool isSaveNeeded() const override;
    bool isDefaults() const override;

    DevicesModel *devicesModel() const;

private:
    void refreshNeedsSave();

    DevicesModel *m_devicesModel;
};
