/*
    SPDX-FileCopyrightText: 2009-2010 Trever Fischer <tdfischer@fedoraproject.org>
    SPDX-FileCopyrightText: 2015 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef DEVICEAUTOMOUNTERKCM_H
#define DEVICEAUTOMOUNTERKCM_H

#include <KCModule>

#include "ui_DeviceAutomounterKCM.h"

class DeviceModel;
class AutomounterSettings;

class DeviceAutomounterKCM : public KCModule, public Ui::DeviceAutomounterKCM
{
    Q_OBJECT

public:
    explicit DeviceAutomounterKCM(QWidget *parent = nullptr, const QVariantList &args = QVariantList());
    ~DeviceAutomounterKCM() override;

public Q_SLOTS:
    void load() override;
    void save() override;

private Q_SLOTS:
    void updateForgetDeviceButton();
    void forgetSelectedDevices();

private:
    void saveLayout();
    void loadLayout();

    AutomounterSettings *m_settings;
    DeviceModel *m_devices;
};

#endif
