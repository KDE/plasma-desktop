/*
    SPDX-FileCopyrightText: 2021 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <KQuickManagedConfigModule>

#include <KSharedConfig>
#include <QKeySequence>

#include "devicesmodel.h"

class TabletSettings;
class TabletData;

class Tablet : public KQuickManagedConfigModule
{
    Q_OBJECT
    Q_PROPERTY(DevicesModel *touchpadModel READ touchpadModel CONSTANT)

public:
    explicit Tablet(QObject *parent, const KPluginMetaData &metaData, const QVariantList &list);
    ~Tablet() override;

    void load() override;
    void save() override;
    void defaults() override;
    // bool isSaveNeeded() const override;
    bool isDefaults() const override;

    DevicesModel *touchpadModel() const;

Q_SIGNALS:
    // void settingsRestored();

private:
    // void refreshNeedsSave();

    DevicesModel *const m_model;
};
