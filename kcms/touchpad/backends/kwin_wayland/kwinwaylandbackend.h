/*
    SPDX-FileCopyrightText: 2017 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "kwinwaylandtouchpad.h"

#include <QList>

namespace KWinDevices
{
class DevicesModel;
}

class KWinWaylandBackend : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QList<KWinWaylandTouchpad *> inputDevices READ inputDevices NOTIFY inputDevicesChanged FINAL)

public:
    explicit KWinWaylandBackend(QObject *parent = nullptr);
    ~KWinWaylandBackend();

    static KWinWaylandBackend *implementation();

    bool save();
    bool load();
    bool defaults();
    bool isSaveNeeded() const;

    QString errorString() const
    {
        return m_errorString;
    }

    int deviceCount() const
    {
        return m_devices.count();
    }
    QList<KWinWaylandTouchpad *> inputDevices() const
    {
        return m_devices;
    }

Q_SIGNALS:
    void needsSaveChanged();

    void touchpadStateChanged();
    void touchpadReset();
    void keyboardActivityStarted();
    void keyboardActivityFinished();

    void inputDevicesChanged();
    void deviceAdded(bool success);
    void deviceRemoved(int index);

private:
    void findTouchpads();

    void onDeviceRowsInserted(const QModelIndex &parent, int first, int last);
    void onDeviceRowsAboutToBeRemoved(const QModelIndex &parent, int first, int last);

    KWinDevices::DevicesModel *m_devicesModel;
    QList<KWinWaylandTouchpad *> m_devices;

    QString m_errorString = QString();
};
