/*
    SPDX-FileCopyrightText: 2018 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "inputbackend.h"
#include "kwin_wl_device.h"

#include <QList>

class QDBusInterface;

class KWinWaylandBackend : public InputBackend
{
    Q_OBJECT

public:
    explicit KWinWaylandBackend(QObject *parent = nullptr);
    ~KWinWaylandBackend();

    bool applyConfig() override;
    bool getConfig() override;
    bool getDefaultConfig() override;
    bool isChangedConfig() const override;
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

    QDBusInterface *m_deviceManager;
    QList<KWinWaylandDevice *> m_devices;
    QVariantMap m_buttonMapping;
    QVariantMap m_loadedButtonMapping;

    QString m_errorString = QString();
};
