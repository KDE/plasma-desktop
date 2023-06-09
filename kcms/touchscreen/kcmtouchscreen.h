/*
    SPDX-FileCopyrightText: 2021 Aleix Pol Gonzalez <aleixpol@kde.org>

    Work sponsored by Technische Universität Dresden:
    SPDX-FileCopyrightText: 2022 Klarälvdalens Datakonsult AB a KDAB Group company <info@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <KQuickManagedConfigModule>

#include <KSharedConfig>
#include <QKeySequence>

#include "devicesmodel.h"

class Touchscreen : public KQuickManagedConfigModule
{
    Q_OBJECT
    Q_PROPERTY(DevicesModel *touchscreensModel READ touchscreensModel CONSTANT)

public:
    explicit Touchscreen(QObject *parent, const KPluginMetaData &metaData);
    ~Touchscreen() override;

    void load() override;
    void save() override;
    void defaults() override;
    bool isSaveNeeded() const override;
    bool isDefaults() const override;

    DevicesModel *touchscreensModel() const;

private:
    void refreshNeedsSave();

    DevicesModel *const m_touchscreensModel;
};
