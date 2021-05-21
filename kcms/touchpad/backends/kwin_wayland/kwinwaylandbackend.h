/*
    SPDX-FileCopyrightText: 2017 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KWINWAYLANDBACKEND_H
#define KWINWAYLANDBACKEND_H

#include "touchpadbackend.h"

#include <QVector>

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

    virtual int touchpadCount() const override
    {
        return m_devices.count();
    }
    virtual QVector<QObject *> getDevices() const override
    {
        return m_devices;
    }

private Q_SLOTS:
    void onDeviceAdded(QString);
    void onDeviceRemoved(QString);

private:
    void findTouchpads();

    QDBusInterface *m_deviceManager;
    QVector<QObject *> m_devices;

    QString m_errorString = QString();
};

#endif // KWINWAYLANDBACKEND_H
