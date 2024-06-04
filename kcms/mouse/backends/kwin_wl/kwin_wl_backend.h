/*
    SPDX-FileCopyrightText: 2018 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "inputbackend.h"
#include "kwin_wl_device.h"

#include <KConfigGroup>

#include <memory>
#include <vector>

class QDBusInterface;

class KWinWaylandBackend : public InputBackend
{
    Q_OBJECT

public:
    explicit KWinWaylandBackend(QObject *parent = nullptr);
    ~KWinWaylandBackend();

    bool save() override;
    bool load() override;
    bool defaults() override;
    bool isSaveNeeded() const override;
    QString errorString() const override;
    int deviceCount() const override;
    QList<InputDevice *> inputDevices() const override;

    QVariantMap buttonMapping() const override;
    void setButtonMapping(const QVariantMap &mapping) override;

private Q_SLOTS:
    void onDeviceAdded(QString);
    void onDeviceRemoved(QString);

private:
    void findDevices();
    // all_of without short-circuiting is like for_each but with an AND-reduced accumulator.
    bool forAllDevices(bool (KWinWaylandDevice::*f)()) const;

    static KConfigGroup mouseButtonRebindsConfigGroup();

    std::unique_ptr<QDBusInterface> m_deviceManager;
    std::vector<std::unique_ptr<KWinWaylandDevice>> m_devices;
    QVariantMap m_buttonMapping;
    QVariantMap m_loadedButtonMapping;

    QString m_errorString;
};
