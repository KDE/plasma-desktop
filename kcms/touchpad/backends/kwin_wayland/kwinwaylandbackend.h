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

    Q_PROPERTY(int touchpadCount READ touchpadCount CONSTANT)

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

    int touchpadCount() const override
    {
        return m_devices.count();
    }
    QList<QObject *> getDevices() const override
    {
        return m_devices;
    }

private Q_SLOTS:
    void onDeviceAdded(QString);
    void onDeviceRemoved(QString);

private:
    void findTouchpads();

    QDBusInterface *m_deviceManager;
    QList<QObject *> m_devices;

    QString m_errorString = QString();
};
