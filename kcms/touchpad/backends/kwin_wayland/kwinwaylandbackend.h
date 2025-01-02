/*
    SPDX-FileCopyrightText: 2017 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "touchpadbackend.h"

#include <QList>

class QDBusInterface;

class KWinWaylandBackend : public TouchpadBackend
{
    Q_OBJECT

public:
    explicit KWinWaylandBackend(QObject *parent = nullptr);
    ~KWinWaylandBackend();

    bool applyConfig() override;
    bool getConfig() override;
    bool getDefaultConfig() override;
    bool isChangedConfig() const override;
    QString errorString() const override
    {
        return m_errorString;
    }

    int deviceCount() const override
    {
        return m_devices.count();
    }
    QList<LibinputCommon *> inputDevices() const override
    {
        return m_devices;
    }

private Q_SLOTS:
    void onDeviceAdded(QString);
    void onDeviceRemoved(QString);

private:
    void findTouchpads();

    QDBusInterface *m_deviceManager;
    QList<LibinputCommon *> m_devices;

    QString m_errorString = QString();
};
