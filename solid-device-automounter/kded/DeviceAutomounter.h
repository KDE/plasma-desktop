/*
    SPDX-FileCopyrightText: 2009 Trever Fischer <wm161@wm161.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef DEVICEAUTOMOUNTER_H
#define DEVICEAUTOMOUNTER_H

#include "AutomounterSettings.h"
#include <KDEDModule>
#include <Solid/Device>

class DeviceAutomounter : public KDEDModule
{
    Q_OBJECT
public:
    explicit DeviceAutomounter(QObject *parent = nullptr, const QVariantList &args = QVariantList());
    ~DeviceAutomounter() override;

private Q_SLOTS:
    void init();
    void deviceAdded(const QString &udi);
    void deviceMountChanged(bool accessible, const QString &udi);

private:
    void automountDevice(Solid::Device &dev, AutomounterSettings::AutomountType type);

    AutomounterSettings *m_settings;
};

#endif
