/*
    SPDX-FileCopyrightText: 2009-2010 Trever Fischer <tdfischer@fedoraproject.org>
    SPDX-FileCopyrightText: 2015 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <KCModule>

#include "ui_DeviceAutomounterKCM.h"

class DeviceModel;
class AutomounterSettings;

class DeviceAutomounterKCM : public KCModule, public Ui::DeviceAutomounterKCM
{
    Q_OBJECT

public:
    explicit DeviceAutomounterKCM(QObject *parent, const KPluginMetaData &data);
    ~DeviceAutomounterKCM() override;

public Q_SLOTS:
    void load() override;
    void save() override;
    void defaults() override;

private Q_SLOTS:
    void updateForgetDeviceButton();
    void forgetSelectedDevices();
    void updateState();

private:
    void saveLayout();
    void loadLayout();

    AutomounterSettings *const m_settings;
    DeviceModel *const m_devices;
    bool m_unmanagedChanges = false;
};
